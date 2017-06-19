#include"pancy_posttreatment.h"

void render_posttreatment_RTGR::update_windowsize(int wind_width_need, int wind_height_need)
{
	half_render_viewport.TopLeftX = 0.0f;
	half_render_viewport.TopLeftY = 0.0f;
	half_render_viewport.Width = static_cast<float>(d3d_pancy_basic_singleton::GetInstance()->get_wind_width()) / 2.0f;
	half_render_viewport.Height = static_cast<float>(d3d_pancy_basic_singleton::GetInstance()->get_wind_height()) / 2.0f;
	half_render_viewport.MinDepth = 0.0f;
	half_render_viewport.MaxDepth = 1.0f;
	release_texture();
	build_texture();
}
render_posttreatment_RTGR::render_posttreatment_RTGR()
{
	color_tex = NULL;
	reflect_target = NULL;
	reflect_tex = NULL;
	final_reflect_target = NULL;
	final_reflect_tex = NULL;
	blur_reflect_tex = NULL;
	blur_reflect_target = NULL;
	mask_target = NULL;
	mask_tex = NULL;
	half_render_viewport.TopLeftX = 0.0f;
	half_render_viewport.TopLeftY = 0.0f;
	half_render_viewport.Width = static_cast<float>(d3d_pancy_basic_singleton::GetInstance()->get_wind_width());
	half_render_viewport.Height = static_cast<float>(d3d_pancy_basic_singleton::GetInstance()->get_wind_height());
	half_render_viewport.MinDepth = 0.0f;
	half_render_viewport.MaxDepth = 1.0f;


}
engine_basic::engine_fail_reason render_posttreatment_RTGR::create()
{
	fullscreen_buffer = new mesh_aosquare(false);
	auto check_error = fullscreen_buffer->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = build_texture();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason render_posttreatment_RTGR::build_texture()
{
	HRESULT hr;
	//创建输入资源
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = d3d_pancy_basic_singleton::GetInstance()->get_wind_width();
	texDesc.Height = d3d_pancy_basic_singleton::GetInstance()->get_wind_height();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* inputcolortex = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &inputcolortex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create SSR_color texture error in reflect posttreat part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(inputcolortex, 0, &color_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create SSR_color SRV error in reflect posttreat part");
		return error_message;
	}
	inputcolortex->Release();

	ID3D11Texture2D* inputmasktex = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &inputmasktex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create SSR_mask texture error in reflect posttreat part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(inputmasktex, 0, &input_mask_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create SSR_mask SRV error in reflect posttreat part");
		return error_message;
	}
	inputmasktex->Release();
	//作为shader resource view的普通纹理(无多重采样)
	texDesc.Width = static_cast<UINT>(half_render_viewport.Width);
	texDesc.Height = static_cast<UINT>(half_render_viewport.Height);
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	//用作pass1反射记录的普通纹理
	ID3D11Texture2D* reflecttex = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &reflecttex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create SSR reflect save texture1 error in globel reflect part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(reflecttex, 0, &reflect_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create SSR reflect save SRV error in globel reflect part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(reflecttex, 0, &reflect_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create SSR reflect save RTV error in globel reflect part");
		return error_message;
	}
	reflecttex->Release();
	//ssr pass1区分是否计算成功的掩码纹理
	ID3D11Texture2D* reflectmasktex = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &reflectmasktex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create SSR reflect mask texture error in globel reflect part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(reflectmasktex, 0, &mask_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create SSR reflect mask SRV error in globel reflect part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(reflectmasktex, 0, &mask_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create SSR reflect mask RTV error in globel reflect part");
		return error_message;
	}
	reflectmasktex->Release();
	//最终合并渲染的反射纹理
	ID3D11Texture2D* final_reflect_data = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &final_reflect_data);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create final reflect save texture error in globel reflect part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(final_reflect_data, 0, &final_reflect_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create final reflect save SRV error in globel reflect part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(final_reflect_data, 0, &final_reflect_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create final reflect save RTV error in globel reflect part");
		return error_message;
	}
	final_reflect_data->Release();
	//存储高斯模糊结果的中间纹理
	texDesc.Width = d3d_pancy_basic_singleton::GetInstance()->get_wind_width();
	texDesc.Height = d3d_pancy_basic_singleton::GetInstance()->get_wind_height();
	ID3D11Texture2D* blur_reflect_data = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &blur_reflect_data);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create mid reflect blur texture error in globel reflect part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(blur_reflect_data, 0, &blur_reflect_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create mid reflect blur SRV error in globel reflect part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(blur_reflect_data, 0, &blur_reflect_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create mid reflect blur RTV error in globel reflect part");
		return error_message;
	}
	blur_reflect_data->Release();
	ID3D11Texture2D* blur_reflect_data2 = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &blur_reflect_data2);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create mid reflect blur texture error in globel reflect part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(blur_reflect_data2, 0, &blur_reflect_tex2);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create mid reflect blur SRV error in globel reflect part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(blur_reflect_data2, 0, &blur_reflect_target2);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create mid reflect blur RTV error in globel reflect part");
		return error_message;
	}
	blur_reflect_data2->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void render_posttreatment_RTGR::release_texture()
{
	safe_release(mask_tex);
	safe_release(mask_target);
	safe_release(reflect_target);
	safe_release(reflect_tex);

	safe_release(final_reflect_target);
	safe_release(final_reflect_tex);
	safe_release(blur_reflect_target);
	safe_release(blur_reflect_tex);
	safe_release(blur_reflect_target2);
	safe_release(blur_reflect_tex2);
}
void render_posttreatment_RTGR::release()
{
	fullscreen_buffer->release();
	safe_release(input_mask_tex);
	safe_release(color_tex);
	release_texture();
}
void render_posttreatment_RTGR::build_reflect_map(XMFLOAT3 center_position, ID3D11RenderTargetView *rendertarget_input, ID3D11RenderTargetView *mask_target_input, ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex, ID3D11ShaderResourceView *reflect_cube_SRV, ID3D11ShaderResourceView *reflect_cubestencil_SRV)
{
	XMFLOAT3 up[6] =
	{
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f,-1.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f)
	};
	XMFLOAT3 look[6] =
	{
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f,-1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 0.0f,-1.0f)
	};
	for (int i = 0; i < 6; ++i)
	{
		XMFLOAT3 look_vec, up_vec;
		look_vec = look[i];
		up_vec = up[i];
		//pancy_camera::get_instance()->get_view_position(&pos_vec);
		//pancy_camera::get_instance()->get_view_position(&pos_vec);
		pancy_camera::get_instance()->count_view_matrix(look_vec, up_vec, center_position, &static_cube_view_matrix[i]);
	}
	//将多重采样纹理转换至非多重纹理
	ID3D11Resource *rendertargetTex = 0;
	ID3D11Resource *rendertargetTex_singlesample = 0;
	rendertarget_input->GetResource(&rendertargetTex);
	color_tex->GetResource(&rendertargetTex_singlesample);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ResolveSubresource(rendertargetTex_singlesample, D3D11CalcSubresource(0, 0, 1), rendertargetTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	color_tex->Release();
	color_tex = NULL;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(rendertargetTex_singlesample, 0, &color_tex);
	rendertargetTex->Release();
	rendertargetTex_singlesample->Release();

	rendertargetTex = 0;
	rendertargetTex_singlesample = 0;
	mask_target_input->GetResource(&rendertargetTex);
	input_mask_tex->GetResource(&rendertargetTex_singlesample);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ResolveSubresource(rendertargetTex_singlesample, D3D11CalcSubresource(0, 0, 1), rendertargetTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	input_mask_tex->Release();
	input_mask_tex = NULL;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(rendertargetTex_singlesample, 0, &input_mask_tex);
	rendertargetTex->Release();
	rendertargetTex_singlesample->Release();
	//绑定渲染目标纹理，不设置深度模缓冲区因为这里不需要
	ID3D11RenderTargetView* renderTargets[2] = { reflect_target,mask_target };
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(2, renderTargets, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(reflect_target, clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &half_render_viewport);
	//renderstate_lib->clear_posttreatmentcrendertarget();
	//设置渲染状态
	static const XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX P = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	XMFLOAT4X4 PT;
	XMStoreFloat4x4(&PT, P*T);
	engine_basic::engine_fail_reason check_error;
	auto shader_reflectpass = shader_control::GetInstance()->get_shader_reflect_draw(check_error);
	XMFLOAT4X4 invview_mat, view_mat;
	XMFLOAT3 view_pos;
	pancy_camera::get_instance()->count_invview_matrix(&invview_mat);
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX rec_need = XMLoadFloat4x4(&invview_mat) * XMLoadFloat4x4(&view_mat);

	pancy_camera::get_instance()->get_view_position(&view_pos);
	shader_reflectpass->set_invview_matrix(&invview_mat);
	shader_reflectpass->set_view_matrix(&view_mat);
	shader_reflectpass->set_view_pos(view_pos);
	shader_reflectpass->set_ViewToTexSpace(&PT);
	XMFLOAT4 FrustumFarCorner[4];
	engine_basic::perspective_message::get_instance()->get_FrustumFarCorner(FrustumFarCorner);
	shader_reflectpass->set_FrustumCorners(FrustumFarCorner);
	shader_reflectpass->set_NormalDepthtex(normaldepth_tex);
	shader_reflectpass->set_Depthtex(depth_tex);
	shader_reflectpass->set_diffusetex(color_tex);
	shader_reflectpass->set_color_mask_tex(input_mask_tex);

	shader_reflectpass->set_camera_positions(center_position);
	shader_reflectpass->set_cubeview_matrix(static_cube_view_matrix, 6);
	shader_reflectpass->set_enviroment_tex(reflect_cube_SRV);
	shader_reflectpass->set_enviroment_stencil(reflect_cubestencil_SRV);
	//渲染屏幕空间像素图
	ID3DX11EffectTechnique* tech, *tech_cube;
	//选定绘制路径
	shader_reflectpass->get_technique(&tech, "draw_ssrmap");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	fullscreen_buffer->get_teque(tech);
	fullscreen_buffer->show_mesh_pass(0);
	//contex_pancy->DrawIndexed(6, 0, 0);
	shader_reflectpass->set_diffusetex(color_tex);
	shader_reflectpass->set_color_mask_tex(mask_tex);
	shader_reflectpass->set_color_ssr_tex(reflect_tex);
	ID3D11RenderTargetView* NULL_target[2] = { NULL,NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(2, NULL_target, 0);

	//basic_blur(mask_tex,reflect_tex, blur_reflect_target, true);
	//basic_blur(mask_tex,blur_reflect_tex, reflect_target, false);
	//basic_blur(mask_tex, reflect_tex, blur_reflect_target, true);
	//basic_blur(mask_tex, blur_reflect_tex, reflect_target, false);
	//basic_blur(mask_tex, blur_reflect_target, true);
	//basic_blur(blur_reflect_tex, mask_target, false);

	ID3D11RenderTargetView* renderTarget_final[1] = { final_reflect_target };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTarget_final, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(final_reflect_target, clearColor);
	//d3d_pancy_basic_singleton::GetInstance()->restore_render_target();

	//tech->GetPassByIndex(1)->Apply(0, contex_pancy);
	//contex_pancy->DrawIndexed(6, 0, 0);
	fullscreen_buffer->show_mesh_pass(1);

	shader_reflectpass->set_NormalDepthtex(NULL);
	shader_reflectpass->set_diffusetex(NULL);
	shader_reflectpass->set_enviroment_tex(NULL);
	shader_reflectpass->set_color_mask_tex(NULL);
	shader_reflectpass->set_color_ssr_tex(NULL);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
}
void render_posttreatment_RTGR::draw_reflect(XMFLOAT3 center_position, ID3D11RenderTargetView *rendertarget_input, ID3D11RenderTargetView *mask_target_input, ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex, ID3D11ShaderResourceView *reflect_cube_SRV, ID3D11ShaderResourceView *reflect_cubestencil_SRV)
{
	build_reflect_map(center_position, rendertarget_input, mask_target_input, normaldepth_tex, depth_tex, reflect_cube_SRV, reflect_cubestencil_SRV);
	d3d_pancy_basic_singleton::GetInstance()->reset_viewport();
	blur_map(normaldepth_tex, depth_tex);
	draw_to_posttarget();
}

void render_posttreatment_RTGR::blur_map(ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex)
{
	basic_blur(normaldepth_tex, depth_tex, mask_tex, final_reflect_tex, blur_reflect_target, true);
	basic_blur(normaldepth_tex, depth_tex, mask_tex, blur_reflect_tex, blur_reflect_target2, false);
	//basic_blur(final_reflect_tex, blur_reflect_target, true);
	//basic_blur(blur_reflect_tex, final_reflect_target, false);
}
void render_posttreatment_RTGR::basic_blur(ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex, ID3D11ShaderResourceView *mask, ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz)
{
	//设置渲染目标
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	ID3D11RenderTargetView* renderTargets[1] = { output };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTargets, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(output, black);
	engine_basic::engine_fail_reason check_error;
	auto shader_blur = shader_control::GetInstance()->get_shader_reflect_blur(check_error);
	XMFLOAT4 map_range = XMFLOAT4(1.0f / half_render_viewport.Width, 1.0f / half_render_viewport.Height, 1.0f / d3d_pancy_basic_singleton::GetInstance()->get_wind_width(), 1.0f / d3d_pancy_basic_singleton::GetInstance()->get_wind_height());
	shader_blur->set_image_size(map_range);
	shader_blur->set_tex_resource(input);
	shader_blur->set_tex_normal_resource(normaldepth_tex);
	shader_blur->set_tex_depth_resource(depth_tex);
	shader_blur->set_tex_mask_resource(mask);
	ID3DX11EffectTechnique* tech;
	if (if_horz == true)
	{
		shader_blur->get_technique(&tech, "HorzBlur_color");
	}
	else
	{
		shader_blur->get_technique(&tech, "VertBlur_color");
	}
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	fullscreen_buffer->get_teque(tech);
	fullscreen_buffer->show_mesh();

	shader_blur->set_tex_resource(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
}
void render_posttreatment_RTGR::draw_to_posttarget()
{
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
	engine_basic::engine_fail_reason check_error;
	auto shader_final_pass = shader_control::GetInstance()->get_shader_reflect_final(check_error);
	shader_final_pass->set_tex_color_resource(color_tex);
	shader_final_pass->set_tex_reflect_resource(final_reflect_tex);
	XMFLOAT4 map_range = XMFLOAT4(1.0f / half_render_viewport.Width, 1.0f / half_render_viewport.Height, 1.0f / render_viewport.Width, 1.0f / render_viewport.Height);
	shader_final_pass->set_image_size(map_range);
	ID3DX11EffectTechnique* tech;
	shader_final_pass->get_technique(&tech, "blend_reflect");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	fullscreen_buffer->get_teque(tech);
	fullscreen_buffer->show_mesh();
	shader_final_pass->set_tex_color_resource(NULL);
	shader_final_pass->set_tex_reflect_resource(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
}
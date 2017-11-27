#include"pancy_posttreatment.h"
/*
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
	blur_reflect_texarray = NULL;
	for (int i = 0; i < 5; ++i)
	{
		blur_reflect_target2[i] = NULL;
	}
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
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* reflectpass_out = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &reflectpass_out);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect out texture error in reflect posttreat part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(reflectpass_out, 0, &reflectpass_outSRV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect out SRV error in reflect posttreat part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(reflectpass_out, 0, &reflectpass_outRTV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect out RTV error in reflect posttreat part");
		return error_message;
	}
	reflectpass_out->Release();

	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
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
	texDesc.MipLevels = 6;
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
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(final_reflect_data, &rtvDesc, &final_reflect_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create final reflect save RTV error in globel reflect part");
		return error_message;
	}
	final_reflect_data->Release();

	texDesc.MipLevels = 1;
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

	texDesc.ArraySize = 5;
	ID3D11Texture2D* blur_reflect_final = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &blur_reflect_final);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create final reflect blur texture error in globel reflect part");
		return error_message;
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = 5;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(blur_reflect_final, &viewDesc, &blur_reflect_texarray);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create mid reflect blur SRV error in globel reflect part");
		return error_message;
	}
	for (int i = 0; i < 5; ++i) 
	{
		D3D11_RENDER_TARGET_VIEW_DESC dsvDesc =
		{
			texDesc.Format,
			D3D11_RTV_DIMENSION_TEXTURE2DARRAY
		};
		dsvDesc.Texture2DArray.ArraySize = 1;
		dsvDesc.Texture2DArray.FirstArraySlice = i;
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(blur_reflect_final, &dsvDesc, &blur_reflect_target2[i]);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason check_error(hr, "create final reflect blur rendertarget error");
			return check_error;
		}
	}
	blur_reflect_final->Release();
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
	safe_release(blur_reflect_texarray);
	for (int i = 0; i < 5; ++i)
	{
		safe_release(blur_reflect_target2[i]);
	}
	safe_release(reflectpass_outRTV);
	safe_release(reflectpass_outSRV);
	safe_release(input_mask_tex);
	safe_release(color_tex);
	//safe_release(blur_reflect_target2);
	//safe_release(blur_reflect_tex2);
}
void render_posttreatment_RTGR::release()
{

	fullscreen_buffer->release();
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
	ID3D11RenderTargetView* renderTargets[2] = { final_reflect_target,mask_target };
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(2, renderTargets, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(final_reflect_target, clearColor);
	float mask_clearColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(mask_target, mask_clearColor);
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
	//shader_reflectpass->set_color_ssr_tex(reflect_tex);
	XMFLOAT4X4 Proj_mat_reflect;
	XMStoreFloat4x4(&Proj_mat_reflect, DirectX::XMMatrixPerspectiveFovLH(0.5f*XM_PI, 1.0f, engine_basic::perspective_message::get_instance()->get_perspective_near_plane(), engine_basic::perspective_message::get_instance()->get_perspective_far_plane()));
	XMFLOAT3 rec_proj_vec;
	rec_proj_vec.x = 1.0f / Proj_mat_reflect._43;
	rec_proj_vec.y = -Proj_mat_reflect._33 / Proj_mat_reflect._43;
	rec_proj_vec.z = 0.0f;
	shader_reflectpass->set_projmessage(rec_proj_vec);
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
	//d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(final_reflect_target, clearColor);
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
	//blur_map(normaldepth_tex, depth_tex);
	//draw_to_posttarget();
}

void render_posttreatment_RTGR::blur_map(ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex)
{
	int basic_width = d3d_pancy_basic_singleton::GetInstance()->get_wind_width();
	int basic_height = d3d_pancy_basic_singleton::GetInstance()->get_wind_height();
	//第一遍blur，去掉噪点
	D3D11_VIEWPORT reflect_blur_VP;
	reflect_blur_VP.TopLeftX = 0.0f;
	reflect_blur_VP.TopLeftY = 0.0f;
	reflect_blur_VP.Width = basic_width;
	reflect_blur_VP.Height = basic_height;
	reflect_blur_VP.MinDepth = 0.0f;
	reflect_blur_VP.MaxDepth = 1.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &reflect_blur_VP);
	basic_blur(normaldepth_tex, depth_tex, mask_tex, final_reflect_tex, blur_reflect_target, true);
	basic_blur(normaldepth_tex, depth_tex, mask_tex, blur_reflect_tex, blur_reflect_target2[0], false);
	d3d_pancy_basic_singleton::GetInstance()->reset_viewport();
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
void render_posttreatment_RTGR::basic_blur_mipmap(ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex, ID3D11ShaderResourceView *mask, ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz, int mip_level)
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
	shader_blur->set_tex_resource_array(input);
	shader_blur->set_tex_normal_resource(normaldepth_tex);
	shader_blur->set_tex_depth_resource(depth_tex);
	shader_blur->set_tex_mask_resource(mask);
	shader_blur->set_sample_level(XMUINT4(mip_level, 0, 0, 0));
	ID3DX11EffectTechnique* tech;
	if (if_horz == true)
	{
		shader_blur->get_technique(&tech, "HorzBlur_mipmap");
	}
	else
	{
		shader_blur->get_technique(&tech, "VertBlur_color_mipmap");
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
void render_posttreatment_RTGR::draw_to_posttarget(ID3D11ShaderResourceView *ao_in, ID3D11ShaderResourceView *metallic_in, ID3D11ShaderResourceView *specrough_in, ID3D11ShaderResourceView *brdf_in)
{
	//d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(reflectpass_outRTV,NULL);
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(reflectpass_outRTV, black);
	engine_basic::engine_fail_reason check_error;
	auto shader_final_pass = shader_control::GetInstance()->get_shader_reflect_final(check_error);
	shader_final_pass->set_tex_color_resource(color_tex);
	shader_final_pass->set_tex_reflect_resource(final_reflect_tex);
	shader_final_pass->set_tex_ao_resource(ao_in);
	shader_final_pass->set_tex_metallic_resource(metallic_in);
	shader_final_pass->set_tex_specroughness_resource(specrough_in);
	shader_final_pass->set_tex_brdflist_resource(brdf_in);
	shader_final_pass->set_tex_tex_albedonov_resource(input_mask_tex);
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
*/
//拷贝到CPU上进行平均亮度计算的map方法
/*
ID3D11Resource *check_rec;
UAV_HDR_mid->GetResource(&check_rec);
ID3D11Buffer* pDebugBuffer = NULL;
D3D11_BOX box;
box.left = 0;
box.right = sizeof(float) * map_num;
box.top = 0;
box.bottom = 1;
box.front = 0;
box.back = 1;
contex_pancy->CopySubresourceRegion(CPU_read_buffer, 0, 0, 0, 0, check_rec, 0, &box);
D3D11_MAPPED_SUBRESOURCE vertex_resource;
hr = contex_pancy->Map(CPU_read_buffer, 0, D3D11_MAP_READ, 0, &vertex_resource);
if (FAILED(hr))
{
MessageBox(0, L"get vertex buffer map error", L"tip", MB_OK);
return hr;
}
float vertex[1000];
memcpy(static_cast<void*>(vertex), vertex_resource.pData, map_num * sizeof(float));
average_light = 0.0f;
for (int i = 0; i < map_num; ++i)
{
average_light += vertex[i];
}
//average_light /= width * height;
average_light = exp(average_light);
check_rec->Release();
contex_pancy->Unmap(CPU_read_buffer, 0);
*/
/*
render_posttreatment_HDR::render_posttreatment_HDR(int width_need, int height_need)
{
	width = width_need;
	height = height_need;
	CPU_read_buffer = NULL;
	average_light_last = 0.0f;
	HDR_fullscreen = new mesh_square(false);
}
void render_posttreatment_HDR::update_windowsize(int wind_width_need, int wind_height_need)
{
	width = wind_width_need;
	height = wind_height_need;
	release_basic();
	init_texture();
	init_buffer();
}
engine_basic::engine_fail_reason render_posttreatment_HDR::count_average_light(ID3D11ShaderResourceView *SRV_HDR_use_in)
{
	HRESULT hr;
	//pass1计算平均亮度
	engine_basic::engine_fail_reason check_error;
	auto shader_test = shader_control::GetInstance()->get_shader_hdr_averagelight(check_error);

	SRV_HDR_use = SRV_HDR_use_in;
	check_error = shader_test->set_compute_tex(SRV_HDR_use);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = shader_test->set_compute_buffer(UAV_HDR_mid, UAV_HDR_final);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	width_rec = width / 4;
	height_rec = height / 4;
	//计算线程数量
	if (width % 4 != 0)
	{
		width_rec += 1;
	}
	if (height % 4 != 0)
	{
		height_rec += 1;
	}
	//计算线程组的数量
	if (width_rec % 16 != 0)
	{
		width_rec = (width_rec / 16 + 1) * 16;
	}
	if (height_rec % 16 != 0)
	{
		height_rec = (height_rec / 16 + 1) * 16;
	}

	//计算线程数量
	buffer_num = width_rec * height_rec;
	//计算线程组数量
	if (buffer_num % 256 != 0)
	{
		buffer_num = buffer_num / 256 + 1;
	}
	else
	{
		buffer_num = buffer_num / 256;
	}
	check_error = shader_test->set_piccturerange(width, height, width_rec * height_rec, height_rec);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	map_num = buffer_num / 256;
	if (buffer_num % 256 != 0)
	{
		map_num = buffer_num / 256 + 1;
	}
	shader_test->dispatch(width_rec / 16, height_rec / 16, buffer_num, map_num);
	shader_test->set_compute_buffer(NULL, NULL);
	shader_test->set_compute_tex(NULL);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason render_posttreatment_HDR::build_preblur_map()
{
	
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &render_viewport);
	//pass2高光模糊的预处理
	engine_basic::engine_fail_reason check_error;
	auto shader_test2 = shader_control::GetInstance()->get_shader_hdr_preblur(check_error);
	shader_test2->set_buffer_input(SRV_HDR_map, SRV_HDR_use);
	shader_test2->set_lum_message(0.0f, 1.0f, 1.5f, 0.38f);
	shader_test2->set_piccturerange(width, height, map_num, width * height);
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(RTV_HDR_blur1, NULL);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(RTV_HDR_blur1, black);
	ID3DX11EffectTechnique* tech;
	shader_test2->get_technique(&tech, "draw_preblur");
	HDR_fullscreen->get_teque(tech);
	HDR_fullscreen->show_mesh();

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	shader_test2->set_buffer_input(NULL, NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(0, NULL_target, 0);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason render_posttreatment_HDR::blur_map()
{
	//pass3高光模糊
	basic_blur(SRV_HDR_blur1, RTV_HDR_blur2, true);
	basic_blur(SRV_HDR_blur2, RTV_HDR_blur1, false);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void render_posttreatment_HDR::basic_blur(ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz)
{
	//设置渲染目标
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };

	d3d_pancy_basic_singleton::GetInstance()->set_render_target(output, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(output, black);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &render_viewport);
	engine_basic::engine_fail_reason check_error;
	auto shader_blur = shader_control::GetInstance()->get_shader_hdr_blur(check_error);
	shader_blur->set_image_size(1.0f / width, 1.0f / height);
	shader_blur->set_tex_resource(input);

	ID3DX11EffectTechnique* tech;
	if (if_horz == true)
	{
		shader_blur->get_technique(&tech, "HorzBlur");
	}
	else
	{
		shader_blur->get_technique(&tech, "VertBlur");
	}
	HDR_fullscreen->get_teque(tech);
	HDR_fullscreen->show_mesh();
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	shader_blur->set_tex_resource(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
}
engine_basic::engine_fail_reason render_posttreatment_HDR::HDR_map()
{
	//pass4最终合成
	//root_state_need->restore_rendertarget();
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
	engine_basic::engine_fail_reason check_error;
	auto shader_test3 = shader_control::GetInstance()->get_shader_hdr_final(check_error);

	shader_test3->set_tex_resource(SRV_HDR_use, SRV_HDR_blur1, SRV_HDR_map);
	shader_test3->set_lum_message(average_light, 1.0f, 2.0f, 0.68f);
	shader_test3->set_piccturerange(width, height, map_num, width * height);

	ID3DX11EffectTechnique* tech;
	shader_test3->get_technique(&tech, "draw_HDRfinal");

	HDR_fullscreen->get_teque(tech);
	HDR_fullscreen->show_mesh();

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	shader_test3->set_tex_resource(NULL, NULL, NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
	
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason render_posttreatment_HDR::CreateCPUaccessBuf(int size_need)
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = size_need*sizeof(float);        //顶点缓存的大小
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(float);
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.BindFlags = 0;
	//bufferDesc.MiscFlags = 0;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&bufferDesc, NULL, &CPU_read_buffer);

	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create CPU access read buffer error in HDR pass");
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason render_posttreatment_HDR::create()
{
	engine_basic::engine_fail_reason check_error;
	check_error = HDR_fullscreen->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_buffer();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason render_posttreatment_HDR::init_texture()
{
	//创建输入资源
	HRESULT hr;
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	//创建高光存储资源
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	ID3D11Texture2D* HDR_preblurtex = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &HDR_preblurtex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_prepass texture error");
		return check_error;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(HDR_preblurtex, 0, &SRV_HDR_save);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_prepass texture error");
		return check_error;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(HDR_preblurtex, 0, &RTV_HDR_save);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_prepass texture error");
		return check_error;
	}
	HDR_preblurtex->Release();
	//创建高斯模糊处理资源
	texDesc.Width = width;
	texDesc.Height = height;
	ID3D11Texture2D* HDR_blurtex1 = 0, *HDR_blurtex2 = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &HDR_blurtex1);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_blurpass texture error");
		return check_error;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &HDR_blurtex2);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_blurpass texture error");
		return check_error;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(HDR_blurtex1, 0, &SRV_HDR_blur1);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_blurpass SRV error");
		return check_error;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(HDR_blurtex2, 0, &SRV_HDR_blur2);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_blurpass SRV error");
		return check_error;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(HDR_blurtex1, 0, &RTV_HDR_blur1);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_blurpass RTV error");
		return check_error;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(HDR_blurtex2, 0, &RTV_HDR_blur2);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_blurpass RTV error");
		return check_error;
	}
	HDR_blurtex1->Release();
	HDR_blurtex2->Release();
	//注册视口信息
	render_viewport.TopLeftX = 0.0f;
	render_viewport.TopLeftY = 0.0f;
	render_viewport.Width = static_cast<float>(width);
	render_viewport.Height = static_cast<float>(height);
	render_viewport.MinDepth = 0.0f;
	render_viewport.MaxDepth = 1.0f;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason render_posttreatment_HDR::init_buffer()
{
	HRESULT hr;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~创建缓冲区~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11Buffer *buffer_HDR_mid;
	ID3D11Buffer *buffer_HDR_final;
	int width_rec = width / 4;
	int height_rec = height / 4;
	//设定线程数量
	if (width % 4 != 0)
	{
		width_rec += 1;
	}
	if (height % 4 != 0)
	{
		height_rec += 1;
	}
	//设定线程组的数量
	if (width_rec % 16 != 0)
	{
		width_rec = (width_rec / 16 + 1) * 16;
	}
	if (height_rec % 16 != 0)
	{
		height_rec = (height_rec / 16 + 1) * 16;
	}
	//创建HDR的第一个buffer，用于1/16的向下采样
	D3D11_BUFFER_DESC HDR_buffer_desc;
	HDR_buffer_desc.Usage = D3D11_USAGE_DEFAULT;            //通用类型
	HDR_buffer_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//缓存类型为uav+srv
	HDR_buffer_desc.ByteWidth = width_rec * height_rec*sizeof(float);        //顶点缓存的大小
	HDR_buffer_desc.CPUAccessFlags = 0;
	HDR_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	HDR_buffer_desc.StructureByteStride = sizeof(float);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&HDR_buffer_desc, NULL, &buffer_HDR_mid);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_averagetex buffer error");
		return check_error;
	}
	//创建第二个buffer，用于1/256的向下采样
	int buffer_num = width_rec * height_rec;
	if (buffer_num % 256 != 0)
	{
		buffer_num = buffer_num / 256 + 1;
	}
	else
	{
		buffer_num = buffer_num / 256;
	}
	HDR_buffer_desc.ByteWidth = buffer_num * sizeof(float);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&HDR_buffer_desc, NULL, &buffer_HDR_final);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_averagetex buffer error");
		return check_error;
	}
	//创建第一个UAV，代表第一个buffer
	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.NumElements = width_rec * height_rec;

	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateUnorderedAccessView(buffer_HDR_mid, &DescUAV, &UAV_HDR_mid);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_averagetex UAV error");
		return check_error;
	}

	//创建第二个UAV，代表第二个buffer
	DescUAV.Buffer.NumElements = buffer_num;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateUnorderedAccessView(buffer_HDR_final, &DescUAV, &UAV_HDR_final);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_averagetex UAV error");
		return check_error;
	}


	D3D11_SHADER_RESOURCE_VIEW_DESC DescSRV;
	ZeroMemory(&DescSRV, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	DescSRV.Format = DXGI_FORMAT_UNKNOWN;
	DescSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	DescSRV.Buffer.FirstElement = 0;
	DescSRV.Buffer.NumElements = width_rec * height_rec;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(buffer_HDR_mid, &DescSRV, &SRV_HDR_map);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_averagetex buffer SRV error");
		return check_error;
	}
	buffer_HDR_mid->Release();
	buffer_HDR_final->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void render_posttreatment_HDR::release_basic()
{
	UAV_HDR_mid->Release();
	UAV_HDR_final->Release();
	SRV_HDR_map->Release();
	if (CPU_read_buffer != NULL)
	{
		CPU_read_buffer->Release();
	}
	SRV_HDR_save->Release();
	RTV_HDR_save->Release();
	SRV_HDR_blur1->Release();
	RTV_HDR_blur1->Release();
	SRV_HDR_blur2->Release();
	RTV_HDR_blur2->Release();
}
void render_posttreatment_HDR::release()
{
	HDR_fullscreen->release();
	release_basic();
}
engine_basic::engine_fail_reason render_posttreatment_HDR::display(ID3D11ShaderResourceView *SRV_HDR_use_in)
{
	count_average_light(SRV_HDR_use_in);
	build_preblur_map();
	blur_map();
	HDR_map();
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetState(NULL);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetDepthStencilState(NULL, NULL);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
*/



postRTGR_out_message::postRTGR_out_message(int width_in, int height_in)
{
	buffer_data.render_viewport.TopLeftX = 0.0f;
	buffer_data.render_viewport.TopLeftY = 0.0f;
	buffer_data.render_viewport.Width = width_in;
	buffer_data.render_viewport.Height = height_in;
	buffer_data.render_viewport.MinDepth = 0.0f;
	buffer_data.render_viewport.MaxDepth = 1.0f;

	buffer_data.half_viewport.TopLeftX = 0.0f;
	buffer_data.half_viewport.TopLeftY = 0.0f;
	buffer_data.half_viewport.Width = width_in;
	buffer_data.half_viewport.Height = height_in;
	buffer_data.half_viewport.MinDepth = 0.0f;
	buffer_data.half_viewport.MaxDepth = 1.0f;
}
engine_basic::engine_fail_reason postRTGR_out_message::create()
{
	engine_basic::engine_fail_reason check_error;
	check_error = init_texture();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason postRTGR_out_message::init_texture()
{
	engine_basic::engine_fail_reason check_error;

	check_error = init_texture_diffrent_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.rtgr_input_tex, &buffer_data.rtgr_input_target, "RTGR_reflect_rendertex");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture_diffrent_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.rtgr_InputMask_tex, &buffer_data.rtgr_InputMask_target, "RTGR_reflectmask_intex");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.reflect_tex, &buffer_data.reflect_target, "RTGR_reflect_targettex");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.mask_tex, &buffer_data.mask_target, "RTGR_reflect_mask_midtex");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.blur_reflect1_tex, &buffer_data.blur_reflect1_target, "RTGR_blur_reflect_target1tex");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.blur_reflect2_tex, &buffer_data.blur_reflect2_target, "RTGR_blur_reflect_target2tex");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.final_reflect_tex, &buffer_data.final_reflect_target, "RTGR_final_reflect_targettex");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.reflect_out_tex, &buffer_data.reflect_out_target, "RTGR_output_reflect_targettex");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason postRTGR_out_message::init_texture_same_resource(DXGI_FORMAT tex_format, ID3D11ShaderResourceView **SRV_in, ID3D11RenderTargetView **RTV_in, string texture_name)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建共用资源纹理访问器~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = static_cast<int>(buffer_data.render_viewport.Width);
	texDesc.Height = static_cast<int>(buffer_data.render_viewport.Height);
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = tex_format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* tex_data_buffer = 0;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &tex_data_buffer);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create " + texture_name + " texdata error when create gbuffer");
		return error_message;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(tex_data_buffer, 0, RTV_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create " + texture_name + " RTV error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(tex_data_buffer, 0, SRV_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create " + texture_name + " SRV error when create gbuffer");
		return error_message;
	}
	tex_data_buffer->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason postRTGR_out_message::init_texture_diffrent_resource(DXGI_FORMAT tex_format, ID3D11ShaderResourceView **SRV_in, ID3D11RenderTargetView **RTV_in, string texture_name)
{
	D3D11_TEXTURE2D_DESC texDesc;
	//4xMSAA抗锯齿纹理作为渲染目标~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.Width = static_cast<int>(buffer_data.render_viewport.Width);
	texDesc.Height = static_cast<int>(buffer_data.render_viewport.Height);
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = tex_format;
	texDesc.SampleDesc.Count = 4;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* tex_data_msaa = 0;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &tex_data_msaa);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create 4xMSAA" + texture_name + " texdata error when create gbuffer");
		return error_message;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(tex_data_msaa, 0, RTV_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create 4xMSAA" + texture_name + " RTV error when create gbuffer");
		return error_message;
	}
	tex_data_msaa->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~非抗锯齿纹理作为访问资源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	ID3D11Texture2D* tex_data_single = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &tex_data_single);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA normalspec_buf texture error when create gbuffer");
		return error_message;
	}
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(tex_data_single, 0, SRV_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA normalspec_buf shaderresourceview error when create gbuffer");
		return error_message;
	}
	tex_data_single->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void postRTGR_out_message::release()
{
	//buffer_data.HDR_input_target->Release();
	//buffer_data.HDR_input_tex->Release();
	buffer_data.rtgr_input_target->Release();
	buffer_data.rtgr_input_tex->Release();
	buffer_data.blur_reflect1_target->Release();
	buffer_data.blur_reflect1_tex->Release();
	buffer_data.blur_reflect2_target->Release();
	buffer_data.blur_reflect2_tex->Release();
	buffer_data.final_reflect_target->Release();
	buffer_data.final_reflect_tex->Release();
	buffer_data.mask_target->Release();
	buffer_data.mask_tex->Release();
	buffer_data.rtgr_InputMask_target->Release();
	buffer_data.rtgr_InputMask_tex->Release();
	buffer_data.reflect_out_target->Release();
	buffer_data.reflect_out_tex->Release();
	buffer_data.reflect_tex->Release();
	buffer_data.reflect_target->Release();
}

render_posttreatment_RTGR::render_posttreatment_RTGR()
{
	fullscreen_buffer = NULL;
}
engine_basic::engine_fail_reason render_posttreatment_RTGR::create()
{
	fullscreen_buffer = new mesh_aosquare(false);
	auto check_error = fullscreen_buffer->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void render_posttreatment_RTGR::release()
{
	fullscreen_buffer->release();
}
void render_posttreatment_RTGR::draw_reflect(
	gbuffer_render_target *gbuffer_texture_in,
	postRTGR_render_target *render_texture_in,
	XMFLOAT3 center_position,
	ID3D11ShaderResourceView *reflect_cube_SRV,
	ID3D11ShaderResourceView *reflect_cubestencil_SRV
	)
{
	build_reflect_map(gbuffer_texture_in, render_texture_in, center_position, reflect_cube_SRV, reflect_cubestencil_SRV);
	blur_map(gbuffer_texture_in, render_texture_in);
	draw_to_posttarget(render_texture_in);
}
void render_posttreatment_RTGR::build_reflect_map(
	gbuffer_render_target *gbuffer_texture_in,
	postRTGR_render_target *rtgr_texture_in,
	XMFLOAT3 center_position,
	ID3D11ShaderResourceView *reflect_cube_SRV,
	ID3D11ShaderResourceView *reflect_cubestencil_SRV
	)
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
		pancy_camera::get_instance()->count_view_matrix(look_vec, up_vec, center_position, &static_cube_view_matrix[i]);
	}
	//将多重采样纹理转换至非多重纹理
	ID3D11Resource *rendertargetTex = 0;
	ID3D11Resource *rendertargetTex_singlesample = 0;
	rtgr_texture_in->rtgr_input_target->GetResource(&rendertargetTex);
	rtgr_texture_in->rtgr_input_tex->GetResource(&rendertargetTex_singlesample);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ResolveSubresource(rendertargetTex_singlesample, D3D11CalcSubresource(0, 0, 1), rendertargetTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	rtgr_texture_in->rtgr_input_tex->Release();
	rtgr_texture_in->rtgr_input_tex = NULL;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(rendertargetTex_singlesample, 0, &rtgr_texture_in->rtgr_input_tex);
	rendertargetTex->Release();
	rendertargetTex_singlesample->Release();

	rendertargetTex = 0;
	rendertargetTex_singlesample = 0;
	rtgr_texture_in->rtgr_InputMask_target->GetResource(&rendertargetTex);
	rtgr_texture_in->rtgr_InputMask_tex->GetResource(&rendertargetTex_singlesample);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ResolveSubresource(rendertargetTex_singlesample, D3D11CalcSubresource(0, 0, 1), rendertargetTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	rtgr_texture_in->rtgr_InputMask_tex->Release();
	rtgr_texture_in->rtgr_InputMask_tex = NULL;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(rendertargetTex_singlesample, 0, &rtgr_texture_in->rtgr_InputMask_tex);
	rendertargetTex->Release();
	rendertargetTex_singlesample->Release();

	//绑定渲染目标纹理，不设置深度模缓冲区因为这里不需要
	ID3D11RenderTargetView* renderTargets[2] = { rtgr_texture_in->final_reflect_target,rtgr_texture_in->mask_target };
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(2, renderTargets, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(rtgr_texture_in->final_reflect_target, clearColor);
	float mask_clearColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(rtgr_texture_in->mask_target, mask_clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &rtgr_texture_in->half_viewport);
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
	shader_reflectpass->set_NormalDepthtex(gbuffer_texture_in->normalspec_tex);
	shader_reflectpass->set_Depthtex(gbuffer_texture_in->depthmap_single_tex);
	shader_reflectpass->set_diffusetex(rtgr_texture_in->rtgr_input_tex);
	shader_reflectpass->set_color_mask_tex(rtgr_texture_in->rtgr_InputMask_tex);

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
	shader_reflectpass->set_diffusetex(rtgr_texture_in->rtgr_input_tex);
	shader_reflectpass->set_color_mask_tex(rtgr_texture_in->mask_tex);
	XMFLOAT4X4 Proj_mat_reflect;
	XMStoreFloat4x4(&Proj_mat_reflect, DirectX::XMMatrixPerspectiveFovLH(0.5f*XM_PI, 1.0f, engine_basic::perspective_message::get_instance()->get_perspective_near_plane(), engine_basic::perspective_message::get_instance()->get_perspective_far_plane()));
	XMFLOAT3 rec_proj_vec;
	rec_proj_vec.x = 1.0f / Proj_mat_reflect._43;
	rec_proj_vec.y = -Proj_mat_reflect._33 / Proj_mat_reflect._43;
	rec_proj_vec.z = 0.0f;
	shader_reflectpass->set_projmessage(rec_proj_vec);
	ID3D11RenderTargetView* NULL_target[2] = { NULL,NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(2, NULL_target, 0);

	ID3D11RenderTargetView* renderTarget_final[1] = { rtgr_texture_in->final_reflect_target };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTarget_final, 0);

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
void render_posttreatment_RTGR::blur_map(gbuffer_render_target *gbuffer_texture_in, postRTGR_render_target *render_texture_in)
{
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &render_texture_in->render_viewport);
	basic_blur(gbuffer_texture_in, render_texture_in, render_texture_in->final_reflect_tex, render_texture_in->blur_reflect1_target, true);
	basic_blur(gbuffer_texture_in, render_texture_in, render_texture_in->blur_reflect1_tex, render_texture_in->blur_reflect2_target, false);
	d3d_pancy_basic_singleton::GetInstance()->reset_viewport();
}
void render_posttreatment_RTGR::basic_blur(
	gbuffer_render_target *gbuffer_texture_in,
	postRTGR_render_target *render_texture_in,
	ID3D11ShaderResourceView *input,
	ID3D11RenderTargetView *output,
	bool if_horz
	)
{
	//设置渲染目标
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	ID3D11RenderTargetView* renderTargets[1] = { output };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTargets, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(output, black);
	engine_basic::engine_fail_reason check_error;
	auto shader_blur = shader_control::GetInstance()->get_shader_reflect_blur(check_error);
	XMFLOAT4 map_range = XMFLOAT4(1.0f / render_texture_in->half_viewport.Width, 1.0f / render_texture_in->half_viewport.Height, 1.0f / render_texture_in->render_viewport.Width, 1.0f / render_texture_in->render_viewport.Height);
	shader_blur->set_image_size(map_range);
	shader_blur->set_tex_resource(input);
	shader_blur->set_tex_normal_resource(gbuffer_texture_in->normalspec_tex);
	shader_blur->set_tex_depth_resource(gbuffer_texture_in->depthmap_single_tex);
	shader_blur->set_tex_mask_resource(render_texture_in->mask_tex);
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
void render_posttreatment_RTGR::draw_to_posttarget(postRTGR_render_target *render_texture_in)
{
	//d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(render_texture_in->reflect_out_target, NULL);
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(render_texture_in->reflect_out_target, black);
	engine_basic::engine_fail_reason check_error;
	auto shader_final_pass = shader_control::GetInstance()->get_shader_reflect_final(check_error);
	shader_final_pass->set_tex_color_resource(render_texture_in->rtgr_input_tex);
	shader_final_pass->set_tex_reflect_resource(render_texture_in->final_reflect_tex);
	XMFLOAT4 map_range = XMFLOAT4(1.0f / render_texture_in->half_viewport.Width, 1.0f / render_texture_in->half_viewport.Height, 1.0f / render_texture_in->render_viewport.Width, 1.0f / render_texture_in->render_viewport.Height);
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

postHDR_out_message::postHDR_out_message(int width_in, int height_in)
{
	buffer_data.render_viewport.TopLeftX = 0.0f;
	buffer_data.render_viewport.TopLeftY = 0.0f;
	buffer_data.render_viewport.Width = width_in;
	buffer_data.render_viewport.Height = height_in;
	buffer_data.render_viewport.MinDepth = 0.0f;
	buffer_data.render_viewport.MaxDepth = 1.0f;
}
engine_basic::engine_fail_reason postHDR_out_message::init_texture_same_resource(DXGI_FORMAT tex_format, ID3D11ShaderResourceView **SRV_in, ID3D11RenderTargetView **RTV_in, string texture_name)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建共用资源纹理访问器~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = static_cast<int>(buffer_data.render_viewport.Width);
	texDesc.Height = static_cast<int>(buffer_data.render_viewport.Height);
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = tex_format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* tex_data_buffer = 0;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &tex_data_buffer);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create " + texture_name + " texdata error when create gbuffer");
		return error_message;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(tex_data_buffer, 0, RTV_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create " + texture_name + " RTV error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(tex_data_buffer, 0, SRV_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create " + texture_name + " SRV error when create gbuffer");
		return error_message;
	}
	tex_data_buffer->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason postHDR_out_message::init_texture()
{
	engine_basic::engine_fail_reason check_error;
	check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.SRV_HDR_save, &buffer_data.RTV_HDR_save, "HDR_preblur_rendertex");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.SRV_HDR_blur1, &buffer_data.RTV_HDR_blur1, "HDR_blur_rendertex1");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.SRV_HDR_blur2, &buffer_data.RTV_HDR_blur2, "HDR_blur_rendertex2");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason postHDR_out_message::init_buffer()
{
	HRESULT hr;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~创建缓冲区~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11Buffer *buffer_HDR_mid;
	ID3D11Buffer *buffer_HDR_final;
	int width = static_cast<int>(buffer_data.render_viewport.Width);
	int height = static_cast<int>(buffer_data.render_viewport.Height);
	int width_rec = width / 4;
	int height_rec = height / 4;
	//设定线程数量
	if (width % 4 != 0)
	{
		width_rec += 1;
	}
	if (height % 4 != 0)
	{
		height_rec += 1;
	}
	//设定线程组的数量
	if (width_rec % 16 != 0)
	{
		width_rec = (width_rec / 16 + 1) * 16;
	}
	if (height_rec % 16 != 0)
	{
		height_rec = (height_rec / 16 + 1) * 16;
	}
	//创建HDR的第一个buffer，用于1/16的向下采样
	D3D11_BUFFER_DESC HDR_buffer_desc;
	HDR_buffer_desc.Usage = D3D11_USAGE_DEFAULT;            //通用类型
	HDR_buffer_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//缓存类型为uav+srv
	HDR_buffer_desc.ByteWidth = width_rec * height_rec*sizeof(float);        //顶点缓存的大小
	HDR_buffer_desc.CPUAccessFlags = 0;
	HDR_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	HDR_buffer_desc.StructureByteStride = sizeof(float);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&HDR_buffer_desc, NULL, &buffer_HDR_mid);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_averagetex buffer error");
		return check_error;
	}
	//创建第二个buffer，用于1/256的向下采样
	int buffer_num = width_rec * height_rec;
	if (buffer_num % 256 != 0)
	{
		buffer_num = buffer_num / 256 + 1;
	}
	else
	{
		buffer_num = buffer_num / 256;
	}
	HDR_buffer_desc.ByteWidth = buffer_num * sizeof(float);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&HDR_buffer_desc, NULL, &buffer_HDR_final);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_averagetex buffer error");
		return check_error;
	}
	//创建第一个UAV，代表第一个buffer
	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.NumElements = width_rec * height_rec;

	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateUnorderedAccessView(buffer_HDR_mid, &DescUAV, &buffer_data.UAV_HDR_mid);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_averagetex UAV error");
		return check_error;
	}

	//创建第二个UAV，代表第二个buffer
	DescUAV.Buffer.NumElements = buffer_num;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateUnorderedAccessView(buffer_HDR_final, &DescUAV, &buffer_data.UAV_HDR_final);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_averagetex UAV error");
		return check_error;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC DescSRV;
	ZeroMemory(&DescSRV, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	DescSRV.Format = DXGI_FORMAT_UNKNOWN;
	DescSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	DescSRV.Buffer.FirstElement = 0;
	DescSRV.Buffer.NumElements = width_rec * height_rec;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(buffer_HDR_mid, &DescSRV, &buffer_data.SRV_HDR_map);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create HDR_averagetex buffer SRV error");
		return check_error;
	}
	buffer_HDR_mid->Release();
	buffer_HDR_final->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason postHDR_out_message::create()
{
	engine_basic::engine_fail_reason check_error;
	check_error = init_texture();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_buffer();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void postHDR_out_message::release()
{
	buffer_data.RTV_HDR_blur1->Release();
	buffer_data.RTV_HDR_blur2->Release();
	buffer_data.RTV_HDR_save->Release();
	buffer_data.SRV_HDR_blur1->Release();
	buffer_data.SRV_HDR_blur2->Release();
	buffer_data.SRV_HDR_map->Release();
	buffer_data.SRV_HDR_save->Release();
	buffer_data.UAV_HDR_final->Release();
	buffer_data.UAV_HDR_mid->Release();
}

render_posttreatment_HDR::render_posttreatment_HDR()
{
	average_light_last = 0.0f;
	HDR_fullscreen = new mesh_square(false);
}
engine_basic::engine_fail_reason render_posttreatment_HDR::create()
{
	engine_basic::engine_fail_reason check_error;
	check_error = HDR_fullscreen->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason render_posttreatment_HDR::display(ID3D11ShaderResourceView *input, HDR_render_target render_target_need, ID3D11RenderTargetView *output)
{
	count_average_light(input, render_target_need);
	build_preblur_map(input, render_target_need);
	blur_map(render_target_need);
	HDR_map(input, render_target_need, output);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason render_posttreatment_HDR::count_average_light(ID3D11ShaderResourceView *SRV_HDR_use_in, HDR_render_target render_target_need)
{
	HRESULT hr;
	//pass1计算平均亮度
	engine_basic::engine_fail_reason check_error;
	auto shader_test = shader_control::GetInstance()->get_shader_hdr_averagelight(check_error);

	check_error = shader_test->set_compute_tex(SRV_HDR_use_in);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = shader_test->set_compute_buffer(render_target_need.UAV_HDR_mid, render_target_need.UAV_HDR_final);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	int width = static_cast<int>(render_target_need.render_viewport.Width);
	int height = static_cast<int>(render_target_need.render_viewport.Height);
	width_rec = width / 4;
	height_rec = height / 4;
	//计算线程数量
	if (width % 4 != 0)
	{
		width_rec += 1;
	}
	if (height % 4 != 0)
	{
		height_rec += 1;
	}
	//计算线程组的数量
	if (width_rec % 16 != 0)
	{
		width_rec = (width_rec / 16 + 1) * 16;
	}
	if (height_rec % 16 != 0)
	{
		height_rec = (height_rec / 16 + 1) * 16;
	}

	//计算线程数量
	buffer_num = width_rec * height_rec;
	//计算线程组数量
	if (buffer_num % 256 != 0)
	{
		buffer_num = buffer_num / 256 + 1;
	}
	else
	{
		buffer_num = buffer_num / 256;
	}
	check_error = shader_test->set_piccturerange(width, height, width_rec * height_rec, height_rec);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	map_num = buffer_num / 256;
	if (buffer_num % 256 != 0)
	{
		map_num = buffer_num / 256 + 1;
	}
	shader_test->dispatch(width_rec / 16, height_rec / 16, buffer_num, map_num);
	shader_test->set_compute_buffer(NULL, NULL);
	shader_test->set_compute_tex(NULL);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason render_posttreatment_HDR::build_preblur_map(ID3D11ShaderResourceView *SRV_HDR_use_in, HDR_render_target render_target_need)
{
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &render_target_need.render_viewport);
	//pass2高光模糊的预处理
	int width = static_cast<int>(render_target_need.render_viewport.Width);
	int height = static_cast<int>(render_target_need.render_viewport.Height);
	engine_basic::engine_fail_reason check_error;
	auto shader_test2 = shader_control::GetInstance()->get_shader_hdr_preblur(check_error);
	shader_test2->set_buffer_input(render_target_need.SRV_HDR_map, SRV_HDR_use_in);
	shader_test2->set_lum_message(0.0f, 1.0f, 1.5f, 0.38f);
	shader_test2->set_piccturerange(width, height, map_num, width * height);
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(render_target_need.RTV_HDR_blur1, NULL);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(render_target_need.RTV_HDR_blur1, black);
	ID3DX11EffectTechnique* tech;
	shader_test2->get_technique(&tech, "draw_preblur");
	HDR_fullscreen->get_teque(tech);
	HDR_fullscreen->show_mesh();

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	shader_test2->set_buffer_input(NULL, NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(0, NULL_target, 0);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void render_posttreatment_HDR::basic_blur(HDR_render_target render_target_need, ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz)
{
	int width = static_cast<int>(render_target_need.render_viewport.Width);
	int height = static_cast<int>(render_target_need.render_viewport.Height);
	//设置渲染目标
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };

	d3d_pancy_basic_singleton::GetInstance()->set_render_target(output, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(output, black);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &render_target_need.render_viewport);
	engine_basic::engine_fail_reason check_error;
	auto shader_blur = shader_control::GetInstance()->get_shader_hdr_blur(check_error);
	shader_blur->set_image_size(1.0f / width, 1.0f / height);
	shader_blur->set_tex_resource(input);

	ID3DX11EffectTechnique* tech;
	if (if_horz == true)
	{
		shader_blur->get_technique(&tech, "HorzBlur");
	}
	else
	{
		shader_blur->get_technique(&tech, "VertBlur");
	}
	HDR_fullscreen->get_teque(tech);
	HDR_fullscreen->show_mesh();
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	shader_blur->set_tex_resource(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
}
engine_basic::engine_fail_reason render_posttreatment_HDR::blur_map(HDR_render_target render_target_need)
{
	//pass3高光模糊
	basic_blur(render_target_need,render_target_need.SRV_HDR_blur1, render_target_need.RTV_HDR_blur2, true);
	basic_blur(render_target_need,render_target_need.SRV_HDR_blur2, render_target_need.RTV_HDR_blur1, false);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason render_posttreatment_HDR::HDR_map(ID3D11ShaderResourceView *input, HDR_render_target render_target_need, ID3D11RenderTargetView *output)
{
	int width = static_cast<int>(render_target_need.render_viewport.Width);
	int height = static_cast<int>(render_target_need.render_viewport.Height);
	//pass4最终合成
	//root_state_need->restore_rendertarget();
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	ID3D11RenderTargetView* renderTargets[1] = { output };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTargets, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(output, black);

	engine_basic::engine_fail_reason check_error;
	auto shader_test3 = shader_control::GetInstance()->get_shader_hdr_final(check_error);

	shader_test3->set_tex_resource(input, render_target_need.SRV_HDR_blur1, render_target_need.SRV_HDR_map);
	shader_test3->set_lum_message(average_light, 1.0f, 2.0f, 0.68f);
	shader_test3->set_piccturerange(width, height, map_num, width * height);

	ID3DX11EffectTechnique* tech;
	shader_test3->get_technique(&tech, "draw_HDRfinal");

	HDR_fullscreen->get_teque(tech);
	HDR_fullscreen->show_mesh();

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	shader_test3->set_tex_resource(NULL, NULL, NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void render_posttreatment_HDR::release()
{
	HDR_fullscreen->release();
}
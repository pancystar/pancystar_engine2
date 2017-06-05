#include"pancy_pretreatment.h"
Pretreatment_gbuffer::Pretreatment_gbuffer(int width_need, int height_need)
{
	map_width = width_need;
	map_height = height_need;

}
engine_basic::engine_fail_reason Pretreatment_gbuffer::create()
{
	//创建缓冲区和纹理
	fullscreen_buffer = new mesh_square(false);
	engine_basic::engine_fail_reason check_error = fullscreen_buffer->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	fullscreen_Lbuffer = new mesh_aosquare(false);
	check_error = fullscreen_Lbuffer->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}




	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void Pretreatment_gbuffer::update_windowsize(int wind_width_need, int wind_height_need)
{
	map_width = wind_width_need;
	map_height = wind_height_need;
	release_texture();
	init_texture();
}
engine_basic::engine_fail_reason Pretreatment_gbuffer::init_texture()
{
	HRESULT hr;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~4xMSAA抗锯齿深度缓冲区&纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = map_width;
	texDesc.Height = map_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc.Count = 4;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	//建立纹理资源
	ID3D11Texture2D* depthMap = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &depthMap);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create 4xMSAA depthtexture error when create gbuffer");
		return error_message;
	}
	//建立深度缓冲区访问器
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateDepthStencilView(depthMap, &dsvDesc, &depthmap_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create 4xMSAA depthstencilview error when create gbuffer");
		return error_message;
	}
	//建立纹理访问器
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(depthMap, &srvDesc, &depthmap_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create 4xMSAA depth shaderresourceview error when create gbuffer");
		return error_message;
	}
	safe_release(depthMap);
	//~~~~~~~~~~~~~~~法线&镜面反射纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//4xMSAA抗锯齿纹理格式并创建纹理资源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.Width = map_width;
	texDesc.Height = map_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 4;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* normalspec_buf = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &normalspec_buf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create 4xMSAA normalspec_buf texture error when create gbuffer");
		return error_message;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(normalspec_buf, 0, &normalspec_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create 4xMSAA normalspec_buf rendertargetview error when create gbuffer");
		return error_message;
	}
	safe_release(normalspec_buf);
	//非抗锯齿法线&镜面纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	ID3D11Texture2D* normalspec_singlebuf = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &normalspec_singlebuf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA normalspec_buf texture error when create gbuffer");
		return error_message;
	}
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(normalspec_singlebuf, 0, &normalspec_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA normalspec_buf shaderresourceview error when create gbuffer");
		return error_message;
	}
	safe_release(normalspec_singlebuf);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~光照信息存储纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	ID3D11Texture2D *diffuse_buf = 0, *specular_buf = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &diffuse_buf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA diffuselight texture error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &specular_buf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA specularlight texture error when create gbuffer");
		return error_message;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(diffuse_buf, 0, &gbuffer_diffuse_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA diffuselight ShaderResourceView error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(diffuse_buf, 0, &gbuffer_diffuse_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA diffuselight RenderTargetView error when create gbuffer");
		return error_message;
	}

	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(specular_buf, 0, &gbuffer_specular_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA specularlight ShaderResourceView error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(specular_buf, 0, &gbuffer_specular_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA specularlight RenderTargetView error when create gbuffer");
		return error_message;
	}
	safe_release(diffuse_buf);
	safe_release(specular_buf);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~4xMSAA重采样深度纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.Width = map_width;
	texDesc.Height = map_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* depth_singlebuffer = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &depth_singlebuffer);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA depth texture error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(depth_singlebuffer, 0, &depthmap_single_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA depth ShaderResourceView error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(depth_singlebuffer, 0, &depthmap_single_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA depth RenderTargetView error when create gbuffer");
		return error_message;
	}
	safe_release(depth_singlebuffer);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void Pretreatment_gbuffer::set_normalspecdepth_target()
{
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(normalspec_target, depthmap_target);
	float clearColor[] = { 0.0f, 0.0f, -1.0f, 1e5f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(normalspec_target, clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearDepthStencilView(depthmap_target, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
void Pretreatment_gbuffer::set_multirender_target()
{
	ID3D11RenderTargetView* renderTargets[2] = { gbuffer_diffuse_target,gbuffer_specular_target };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(2, renderTargets, NULL);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(gbuffer_diffuse_target, clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(gbuffer_specular_target, clearColor);
}
void Pretreatment_gbuffer::set_resolvdepth_target()
{
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(depthmap_single_target, NULL);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(depthmap_single_target, clearColor);
}
void Pretreatment_gbuffer::release_texture()
{
	safe_release(depthmap_tex);
	safe_release(depthmap_target);
	safe_release(normalspec_target);
	safe_release(normalspec_tex);
	safe_release(gbuffer_diffuse_target);
	safe_release(gbuffer_diffuse_tex);
	safe_release(gbuffer_specular_target);
	safe_release(gbuffer_specular_tex);
	safe_release(depthmap_single_target);
	safe_release(depthmap_single_tex);
}
void Pretreatment_gbuffer::render_gbuffer()
{
	//关闭alpha混合
	set_normalspecdepth_target();
	//绘制gbuffer
	pancy_geometry_control_singleton::get_instance()->render_gbuffer();
	//还原渲染状态
	ID3D11Resource * normalDepthTex = 0;
	ID3D11Resource * normalDepthTex_singlesample = 0;
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, NULL_target, NULL);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~存储法线镜面反射光纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	normalspec_target->GetResource(&normalDepthTex);
	normalspec_tex->GetResource(&normalDepthTex_singlesample);
	//将多重采样纹理转换至非多重纹理
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ResolveSubresource(normalDepthTex_singlesample, D3D11CalcSubresource(0, 0, 1), normalDepthTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	normalspec_tex->Release();
	normalspec_tex = NULL;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(normalDepthTex_singlesample, 0, &normalspec_tex);
	normalDepthTex->Release();
	normalDepthTex_singlesample->Release();
	//msaa-shader重采样
	set_resolvdepth_target();
	engine_basic::engine_fail_reason check_error;
	auto shader_resolve = shader_control::GetInstance()->get_shader_resolvedepth(check_error);
	shader_resolve->set_texture_MSAA(depthmap_tex);
	XMFLOAT3 rec_proj_vec;
	rec_proj_vec.x = 1.0f / engine_basic::perspective_message::get_instance()->get_proj_matrix()._43;
	rec_proj_vec.y = -engine_basic::perspective_message::get_instance()->get_proj_matrix()._33 / engine_basic::perspective_message::get_instance()->get_proj_matrix()._43;
	rec_proj_vec.z = 0.0f;
	shader_resolve->set_projmessage(rec_proj_vec);
	shader_resolve->set_window_size(static_cast<float>(map_width), static_cast<float>(map_height));
	ID3DX11EffectTechnique *tech_need;
	shader_resolve->get_technique(&tech_need, "resolove_msaa");
	resolve_depth_render(tech_need);
	shader_resolve->set_texture_MSAA(NULL);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, NULL_target, NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech_need->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
}
void Pretreatment_gbuffer::render_lbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 invview_matrix, bool if_shadow)
{

	set_multirender_target();
	engine_basic::engine_fail_reason check_error;
	auto lbuffer_shader = shader_control::GetInstance()->get_shader_lightbuffer(check_error);
	lbuffer_shader->set_DepthMap_tex(depthmap_single_tex);
	lbuffer_shader->set_Normalspec_tex(normalspec_tex);
	XMFLOAT4 farcorner[4];
	engine_basic::perspective_message::get_instance()->get_FrustumFarCorner(farcorner);
	lbuffer_shader->set_FrustumCorners(farcorner);


	XMFLOAT4X4 view_mat,invview_mat;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	pancy_camera::get_instance()->count_invview_matrix(&invview_mat);
	lbuffer_shader->set_view_matrix(&view_mat);
	lbuffer_shader->set_invview_matrix(&invview_mat);

	ID3DX11EffectTechnique *tech_need;
	if (if_shadow == true)
	{
		lbuffer_shader->get_technique(&tech_need, "draw_common");
	}
	else
	{
		lbuffer_shader->get_technique(&tech_need, "draw_withoutshadow");
	}
	light_buffer_render(tech_need);

	//还原渲染状态
	ID3D11RenderTargetView* NULL_target[2] = { NULL,NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(2, NULL_target, 0);
	lbuffer_shader->set_DepthMap_tex(NULL);
	lbuffer_shader->set_Normalspec_tex(NULL);
	lbuffer_shader->set_shadow_tex(NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech_need->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
}

void Pretreatment_gbuffer::display()
{
	render_gbuffer();
}
void Pretreatment_gbuffer::display_lbuffer(bool if_shadow)
{
	XMFLOAT4X4 view_matrix, invview_matrix;
	pancy_camera::get_instance()->count_view_matrix(&view_matrix);
	pancy_camera::get_instance()->count_invview_matrix(&invview_matrix);
	render_lbuffer(view_matrix, invview_matrix, if_shadow);

	engine_basic::engine_fail_reason check_error;
	auto shader_deffered = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	shader_deffered->set_diffuse_light_tex(gbuffer_diffuse_tex);
	shader_deffered->set_specular_light_tex(gbuffer_specular_tex);
}
void Pretreatment_gbuffer::release()
{
	fullscreen_Lbuffer->release();
	fullscreen_buffer->release();
	release_texture();
}
void Pretreatment_gbuffer::resolve_depth_render(ID3DX11EffectTechnique* tech)
{
	fullscreen_buffer->get_teque(tech);
	fullscreen_buffer->show_mesh();
}
void Pretreatment_gbuffer::light_buffer_render(ID3DX11EffectTechnique* tech)
{
	fullscreen_Lbuffer->get_teque(tech);
	fullscreen_Lbuffer->show_mesh();
}
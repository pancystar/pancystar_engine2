#include"pancy_pretreatment.h"
gbuffer_out_message::gbuffer_out_message(int width_in, int height_in, bool if_MSAA)
{
	//获得视口信息及抗锯齿信息
	buffer_data.IF_MSAA = if_MSAA;
	buffer_data.render_viewport.TopLeftX = 0.0f;
	buffer_data.render_viewport.TopLeftY = 0.0f;
	buffer_data.render_viewport.Width = width_in;
	buffer_data.render_viewport.Height = height_in;
	buffer_data.render_viewport.MinDepth = 0.0f;
	buffer_data.render_viewport.MaxDepth = 1.0f;
	//清空纹理指针
	buffer_data.depthmap_target = NULL;
	buffer_data.depthmap_tex = NULL;
	buffer_data.AtmosphereMask_target = NULL;
	buffer_data.AtmosphereMask_tex = NULL;
	buffer_data.normalspec_target = NULL;
	buffer_data.normalspec_tex = NULL;
	buffer_data.specroughness_target = NULL;
	buffer_data.specroughness_tex = NULL;
	buffer_data.depthmap_single_target = NULL;
	buffer_data.depthmap_single_tex = NULL;
	buffer_data.gbuffer_atmosphere_target = NULL;
	buffer_data.gbuffer_atmosphere_tex = NULL;
	buffer_data.gbuffer_diffuse_target = NULL;
	buffer_data.gbuffer_diffuse_tex = NULL;
	buffer_data.gbuffer_specular_target = NULL;
	buffer_data.gbuffer_specular_tex = NULL;
}
engine_basic::engine_fail_reason gbuffer_out_message::create()
{
	auto check_error = init_texture();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason gbuffer_out_message::init_texture_same_resource(DXGI_FORMAT tex_format, ID3D11ShaderResourceView **SRV_in, ID3D11RenderTargetView **RTV_in, string texture_name)
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
engine_basic::engine_fail_reason gbuffer_out_message::init_texture_diffrent_resource(DXGI_FORMAT tex_format, ID3D11ShaderResourceView **SRV_in, ID3D11RenderTargetView **RTV_in, string texture_name)
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

engine_basic::engine_fail_reason gbuffer_out_message::init_texture()
{
	engine_basic::engine_fail_reason check_error;
	if (buffer_data.IF_MSAA)
	{
		HRESULT hr;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~4xMSAA抗锯齿深度缓冲区&纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = static_cast<int>(buffer_data.render_viewport.Width);
		texDesc.Height = static_cast<int>(buffer_data.render_viewport.Height);
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
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateDepthStencilView(depthMap, &dsvDesc, &buffer_data.depthmap_target);
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
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(depthMap, &srvDesc, &buffer_data.depthmap_tex);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "create 4xMSAA depth shaderresourceview error when create gbuffer");
			return error_message;
		}
		depthMap->Release();
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建非共享资源纹理访问器~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		check_error = init_texture_diffrent_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.normalspec_tex, &buffer_data.normalspec_target, "normal_specular tex");
		if (!check_error.check_if_failed())
		{
			return check_error;
		}
		check_error = init_texture_diffrent_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.specroughness_tex, &buffer_data.specroughness_target, "speculat_roughness tex");
		if (!check_error.check_if_failed())
		{
			return check_error;
		}
		check_error = init_texture_diffrent_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.AtmosphereMask_tex, &buffer_data.AtmosphereMask_target, "Atmosphere_Mask tex");
		if (!check_error.check_if_failed())
		{
			return check_error;
		}
		//非MSAA深度纹理
		check_error = init_texture_same_resource(DXGI_FORMAT_R32_FLOAT, &buffer_data.depthmap_single_tex, &buffer_data.depthmap_single_target, "depth single tex");
		if (!check_error.check_if_failed())
		{
			return check_error;
		}
	}
	else
	{
		//深度纹理
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = static_cast<int>(buffer_data.render_viewport.Width);
		texDesc.Height = static_cast<int>(buffer_data.render_viewport.Height);
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		//建立纹理资源
		ID3D11Texture2D* depthMap = 0;
		HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &depthMap);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "create NoMsaa depthtexture error when create gbuffer");
			return error_message;
		}
		//建立深度缓冲区访问器
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = 0;
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateDepthStencilView(depthMap, &dsvDesc, &buffer_data.depthmap_target);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "create NoMsaa depthtexture DSV error when create gbuffer");
			return error_message;
		}
		//建立纹理访问器
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(depthMap, &srvDesc, &buffer_data.depthmap_tex);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "create NoMsaa depthtexture SRV error when create gbuffer");
			return error_message;
		}
		depthMap->Release();
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建共享资源纹理访问器~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.normalspec_tex, &buffer_data.normalspec_target, "normal_specular tex");
		if (!check_error.check_if_failed())
		{
			return check_error;
		}
		check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.specroughness_tex, &buffer_data.specroughness_target, "speculat_roughness tex");
		if (!check_error.check_if_failed())
		{
			return check_error;
		}
		check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.AtmosphereMask_tex, &buffer_data.AtmosphereMask_target, "Atmosphere_Mask tex");
		if (!check_error.check_if_failed())
		{
			return check_error;
		}
	}
	//lightbuffer渲染对象
	check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.gbuffer_diffuse_tex, &buffer_data.gbuffer_diffuse_target, "diffuse_color tex");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.gbuffer_specular_tex, &buffer_data.gbuffer_specular_target, "speculat_color tex");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture_same_resource(DXGI_FORMAT_R16G16B16A16_FLOAT, &buffer_data.gbuffer_atmosphere_tex, &buffer_data.gbuffer_atmosphere_target, "Atmosphere_color tex");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void gbuffer_out_message::release()
{
	buffer_data.depthmap_target->Release();
	buffer_data.depthmap_tex->Release();
	buffer_data.AtmosphereMask_target->Release();
	buffer_data.AtmosphereMask_tex->Release();
	buffer_data.normalspec_target->Release();
	buffer_data.normalspec_tex->Release();
	buffer_data.specroughness_target->Release();
	buffer_data.specroughness_tex->Release();
	if (buffer_data.IF_MSAA)
	{
		buffer_data.depthmap_single_target->Release();
		buffer_data.depthmap_single_tex->Release();
	}
	buffer_data.gbuffer_atmosphere_target->Release();
	buffer_data.gbuffer_atmosphere_tex->Release();
	buffer_data.gbuffer_diffuse_target->Release();
	buffer_data.gbuffer_diffuse_tex->Release();
	buffer_data.gbuffer_specular_target->Release();
	buffer_data.gbuffer_specular_tex->Release();
}

Pretreatment_gbuffer::Pretreatment_gbuffer()
{
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
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

void Pretreatment_gbuffer::set_normalspecdepth_target(gbuffer_render_target render_target_out, bool if_clear)
{
	ID3D11RenderTargetView* renderTargets[3] = { render_target_out.normalspec_target,render_target_out.specroughness_target,render_target_out.AtmosphereMask_target };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(3, renderTargets, render_target_out.depthmap_target);
	if (if_clear)
	{
		//d3d_pancy_basic_singleton::GetInstance()->set_render_target(normalspec_target, depthmap_target);
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(render_target_out.normalspec_target, clearColor);
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(render_target_out.specroughness_target, clearColor);
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(render_target_out.AtmosphereMask_target, clearColor);
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearDepthStencilView(render_target_out.depthmap_target, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}

void Pretreatment_gbuffer::set_multirender_target(gbuffer_render_target render_target_out)
{
	ID3D11RenderTargetView* renderTargets[3] = { render_target_out.gbuffer_diffuse_target,render_target_out.gbuffer_specular_target,render_target_out.gbuffer_atmosphere_target };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(3, renderTargets, NULL);
	//ID3D11RenderTargetView* renderTargets[3] = { render_target_out.gbuffer_diffuse_target,render_target_out.gbuffer_specular_target };
	//d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(2, renderTargets, NULL);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(render_target_out.gbuffer_diffuse_target, clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(render_target_out.gbuffer_specular_target, clearColor);
}
void Pretreatment_gbuffer::set_resolvdepth_target(gbuffer_render_target render_target_out)
{
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(render_target_out.depthmap_single_target, NULL);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(render_target_out.depthmap_single_target, clearColor);
}

void Pretreatment_gbuffer::release()
{
	fullscreen_Lbuffer->release();
	fullscreen_buffer->release();
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

void Pretreatment_gbuffer::render_gbuffer(
	pancy_geometry_control *geometry_list,
	gbuffer_render_target *render_target_out,
	XMFLOAT4X4 view_matrix,
	engine_basic::extra_perspective_message *perspective_message,
	bool if_static,
	bool if_post
	)
{
	//engine_basic::engine_fail_reason check_error;
	//设置渲染目标及视口
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &render_target_out->render_viewport);
	//开始渲染
	if (if_post)
	{
		set_normalspecdepth_target(*render_target_out,false);
		geometry_list->render_gbuffer_post(view_matrix, perspective_message->get_proj_matrix(), if_static);
	}
	else
	{
		set_normalspecdepth_target(*render_target_out, true);
		geometry_list->render_gbuffer(view_matrix, perspective_message->get_proj_matrix(), if_static);
	}
	if (render_target_out->IF_MSAA)
	{
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~存储法线镜面反射光纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		ID3D11Resource * normalDepthTex = 0;
		ID3D11Resource * normalDepthTex_singlesample = 0;
		render_target_out->normalspec_target->GetResource(&normalDepthTex);
		render_target_out->normalspec_tex->GetResource(&normalDepthTex_singlesample);
		//将多重采样纹理转换至非多重纹理
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ResolveSubresource(normalDepthTex_singlesample, D3D11CalcSubresource(0, 0, 1), normalDepthTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
		render_target_out->normalspec_tex->Release();
		render_target_out->normalspec_tex = NULL;
		auto check_error1 = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(normalDepthTex_singlesample, 0, &render_target_out->normalspec_tex);
		normalDepthTex->Release();
		normalDepthTex_singlesample->Release();

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~存储镜面反射粗糙度纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		ID3D11Resource * specroughnessTex = 0;
		ID3D11Resource * specroughnessTex_singlesample = 0;
		render_target_out->specroughness_target->GetResource(&specroughnessTex);
		render_target_out->specroughness_tex->GetResource(&specroughnessTex_singlesample);
		//将多重采样纹理转换至非多重纹理
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ResolveSubresource(specroughnessTex_singlesample, D3D11CalcSubresource(0, 0, 1), specroughnessTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
		render_target_out->specroughness_tex->Release();
		render_target_out->specroughness_tex = NULL;
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(specroughnessTex_singlesample, 0, &render_target_out->specroughness_tex);
		specroughnessTex->Release();
		specroughnessTex_singlesample->Release();
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~存储大气掩码纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		ID3D11Resource * atmospheremaslTex = 0;
		ID3D11Resource * atmospheremaslTex_singlesample = 0;
		render_target_out->AtmosphereMask_target->GetResource(&atmospheremaslTex);
		render_target_out->AtmosphereMask_tex->GetResource(&atmospheremaslTex_singlesample);
		//将多重采样纹理转换至非多重纹理
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ResolveSubresource(atmospheremaslTex_singlesample, D3D11CalcSubresource(0, 0, 1), atmospheremaslTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
		render_target_out->AtmosphereMask_tex->Release();
		render_target_out->AtmosphereMask_tex = NULL;
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(atmospheremaslTex_singlesample, 0, &render_target_out->AtmosphereMask_tex);
		atmospheremaslTex->Release();
		atmospheremaslTex_singlesample->Release();

		engine_basic::engine_fail_reason check_error;
		//auto shader_atmosphere = shader_control::GetInstance()->get_shader_atmosphere_render(check_error);
		//shader_atmosphere->set_tex_mask(AtmosphereMask_tex);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~msaa-shader重采样~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		set_resolvdepth_target(*render_target_out);
		//	engine_basic::engine_fail_reason check_error;
		auto shader_resolve = shader_control::GetInstance()->get_shader_resolvedepth(check_error);
		shader_resolve->set_texture_MSAA(render_target_out->depthmap_tex);
		XMFLOAT3 rec_proj_vec;
		rec_proj_vec.x = 1.0f / engine_basic::perspective_message::get_instance()->get_proj_matrix()._43;
		rec_proj_vec.y = -engine_basic::perspective_message::get_instance()->get_proj_matrix()._33 / engine_basic::perspective_message::get_instance()->get_proj_matrix()._43;
		rec_proj_vec.z = 0.0f;
		shader_resolve->set_projmessage(rec_proj_vec);
		shader_resolve->set_window_size(static_cast<float>(render_target_out->render_viewport.Width), static_cast<float>(render_target_out->render_viewport.Height));
		ID3DX11EffectTechnique *tech_need;
		shader_resolve->get_technique(&tech_need, "resolove_msaa");
		resolve_depth_render(tech_need);
		shader_resolve->set_texture_MSAA(NULL);
		ID3D11RenderTargetView* NULL_target[1] = { NULL };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, NULL_target, NULL);
		D3DX11_TECHNIQUE_DESC techDesc;
		tech_need->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			tech_need->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
		}
	}
	else
	{
		render_target_out->depthmap_single_tex = render_target_out->depthmap_tex;
	}
}
void Pretreatment_gbuffer::render_lbuffer(
	gbuffer_render_target *render_target_out,
	XMFLOAT3 view_position,
	XMFLOAT4X4 view_matrix,
	XMFLOAT4X4 invview_matrix,
	engine_basic::extra_perspective_message *perspective_message,
	bool if_shadow
	)
{
	//渲染正方向光照纹理
	set_multirender_target(*render_target_out);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &render_target_out->render_viewport);
	engine_basic::engine_fail_reason check_error;
	auto lbuffer_shader = shader_control::GetInstance()->get_shader_lightbuffer(check_error);
	//gbuffer计算完成的三个变量
	lbuffer_shader->set_DepthMap_tex(render_target_out->depthmap_single_tex);
	lbuffer_shader->set_Normalspec_tex(render_target_out->normalspec_tex);
	lbuffer_shader->set_SpecRoughness_tex(render_target_out->specroughness_tex);
	//取景变换，投影变换以及视点位置
	XMFLOAT4 farcorner[4];
	perspective_message->get_FrustumFarCorner(farcorner);
	lbuffer_shader->set_FrustumCorners(farcorner);
	lbuffer_shader->set_view_matrix(&view_matrix);
	lbuffer_shader->set_invview_matrix(&invview_matrix);
	lbuffer_shader->set_tex_mask(render_target_out->AtmosphereMask_tex);
	lbuffer_shader->set_camera(view_position);
	lbuffer_shader->set_exposure(10.0f);
	//lbuffer_shader->set_sun_size(XMFLOAT2(1,1));
	//MSAA深度还原参数
	XMFLOAT3 rec_proj_vec;
	rec_proj_vec.x = 1.0f / perspective_message->get_proj_matrix()._43;
	rec_proj_vec.y = -perspective_message->get_proj_matrix()._33 / perspective_message->get_proj_matrix()._43;
	rec_proj_vec.z = 0.0f;
	lbuffer_shader->set_projmessage(rec_proj_vec);
	//开始渲染
	ID3DX11EffectTechnique *tech_need;
	if (if_shadow == true)
	{
		if (render_target_out->IF_MSAA)
		{
			lbuffer_shader->get_technique(&tech_need, "draw_common_pbr");
		}
		else
		{
			lbuffer_shader->get_technique(&tech_need, "draw_pbr_withoutMSAA");
		}

	}
	else
	{
		if (render_target_out->IF_MSAA)
		{
			lbuffer_shader->get_technique(&tech_need, "draw_pbr_withoutshadow");
		}
		else
		{
			lbuffer_shader->get_technique(&tech_need, "draw_pbr_withoutshadowMSAA");
		}
	}
	light_buffer_render(tech_need);
	auto shader_sky = shader_control::GetInstance()->get_shader_sky_draw(check_error);
	shader_sky->set_tex_atmosphere(render_target_out->gbuffer_atmosphere_tex);
	shader_sky->set_tex_atmosphere_occlusion(render_target_out->AtmosphereMask_tex);
	auto shader_deffered = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	shader_deffered->set_tex_atmosphere_occlusion(render_target_out->AtmosphereMask_tex);
	//还原渲染状态
	ID3D11RenderTargetView* NULL_target[3] = { NULL,NULL,NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(3, NULL_target, 0);
	lbuffer_shader->set_DepthMap_tex(NULL);
	lbuffer_shader->set_Normalspec_tex(NULL);
	lbuffer_shader->set_shadow_tex(NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech_need->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
	/*
	//设置光照贴图
	auto shader_deffered = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	shader_deffered->set_diffuse_light_tex(render_target_out->gbuffer_diffuse_tex);
	shader_deffered->set_specular_light_tex(render_target_out->gbuffer_specular_tex);
	shader_deffered->set_normal_tex(render_target_out->normalspec_tex);
	shader_deffered->set_tex_specroughness_resource(render_target_out->specroughness_tex);
	*/
}


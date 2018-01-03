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
	/*
	environment_map_place = XMFLOAT3(0.0f, 5.0f, 0.0f);
	map_width = width_need;
	map_height = height_need;
	quality_reflect = quality_reflect_need;
	depthmap_tex = NULL;
	depthmap_target = NULL;
	normalspec_target = NULL;
	normalspec_tex = NULL;
	gbuffer_diffuse_target = NULL;
	gbuffer_diffuse_tex = NULL;
	gbuffer_specular_target = NULL;
	gbuffer_specular_tex = NULL;
	depthmap_single_target = NULL;
	depthmap_single_tex = NULL;
	reflect_cubenormal_SRV = NULL;
	reflect_cubenormal_RTV = NULL;
	reflect_DSV = NULL;
	reflect_depthcube_SRV = NULL;
	reflect_diffuse_target = NULL;
	reflect_diffuse_tex = NULL;
	reflect_specular_target = NULL;
	reflect_specular_tex = NULL;
	reflect_cubestencil_SRV = NULL;
	for (int i = 0; i < 6; ++i)
	{
		reflect_cubestencil_RTV[i] = NULL;
	}
	posttreatment_RTV = NULL;
	reflectmask_RTV = NULL;
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
		up_cube_reflect[i] = up[i];
		look_cube_reflect[i] = look[i];
	}
	now_reflect_render_face = 0;
	last_reflect_render_face = -1;
	*/
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
	/*
	check_error = init_texture();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_reflect_texture();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	*/
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
/*
engine_basic::engine_fail_reason Pretreatment_gbuffer::init_reflect_texture()
{
	HRESULT hr;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建反射贴图的深度纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = static_cast<int>(1024.0f * quality_reflect);
	texDesc.Height = static_cast<int>(1024.0f * quality_reflect);
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
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &depthMap);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect depthtexture error when create gbuffer");
		return error_message;
	}
	//建立深度缓冲区访问器
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateDepthStencilView(depthMap, &dsvDesc, &reflect_DSV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect depthstencilview error when create gbuffer");
		return error_message;
	}
	//建立纹理访问器
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(depthMap, &srvDesc, &reflect_depthcube_SRV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect depth shaderresourceview error when create gbuffer");
		return error_message;
	}
	safe_release(depthMap);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建反射贴图的法线纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.Width = static_cast<int>(1024.0f * quality_reflect);
	texDesc.Height = static_cast<int>(1024.0f * quality_reflect);
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* normalspec_buf = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &normalspec_buf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect normalspec_buf texture error when create gbuffer");
		return error_message;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(normalspec_buf, 0, &reflect_cubenormal_RTV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect normalspec_buf rendertargetview error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(normalspec_buf, 0, &reflect_cubenormal_SRV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect normalspec_buf shaderresourceview error when create gbuffer");
		return error_message;
	}
	safe_release(normalspec_buf);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建反射贴图的镜面反射粗糙度纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.Width = static_cast<int>(1024.0f * quality_reflect);
	texDesc.Height = static_cast<int>(1024.0f * quality_reflect);
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* specroughness_buf = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &specroughness_buf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect specroughness_buf texture error when create gbuffer");
		return error_message;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(specroughness_buf, 0, &reflect_cubeSpecRough_RTV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect specroughness_buf rendertargetview error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(specroughness_buf, 0, &reflect_cubeSpecRough_SRV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect specroughness_buf shaderresourceview error when create gbuffer");
		return error_message;
	}
	safe_release(specroughness_buf);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建反射贴图的大气散射纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.Width = static_cast<int>(1024.0f * quality_reflect);
	texDesc.Height = static_cast<int>(1024.0f * quality_reflect);
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* AtmosphereMask_buf = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &AtmosphereMask_buf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect AtmosphereMask texture error when create gbuffer");
		return error_message;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(AtmosphereMask_buf, 0, &reflect_AtmosphereMask_RTV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect AtmosphereMask_buf rendertargetview error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(AtmosphereMask_buf, 0, &reflect_AtmosphereMask_SRV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect AtmosphereMask_buf shaderresourceview error when create gbuffer");
		return error_message;
	}
	safe_release(AtmosphereMask_buf);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~光照信息存储纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11Texture2D *diffuse_buf = 0, *specular_buf = 0, *atmosphere_buffer = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &diffuse_buf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect diffuselight texture error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &specular_buf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect specularlight texture error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &atmosphere_buffer);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect atmosphere texture error when create gbuffer");
		return error_message;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(diffuse_buf, 0, &reflect_diffuse_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect diffuselight ShaderResourceView error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(diffuse_buf, 0, &reflect_diffuse_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect diffuselight RenderTargetView error when create gbuffer");
		return error_message;
	}

	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(specular_buf, 0, &reflect_specular_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect specularlight ShaderResourceView error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(specular_buf, 0, &reflect_specular_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect specularlight RenderTargetView error when create gbuffer");
		return error_message;
	}

	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(atmosphere_buffer, 0, &reflect_atmosphere_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect atmospherelight ShaderResourceView error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(atmosphere_buffer, 0, &reflect_atmosphere_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect atmospherelight RenderTargetView error when create gbuffer");
		return error_message;
	}
	safe_release(diffuse_buf);
	safe_release(specular_buf);
	safe_release(atmosphere_buffer);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建cube面信息记录纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	D3D11_TEXTURE2D_DESC cubeMapDesc;
	//渲染目标
	cubeMapDesc.Width = static_cast<UINT>(1024.0f * quality_reflect);
	cubeMapDesc.Height = static_cast<UINT>(1024.0f * quality_reflect);
	cubeMapDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	cubeMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	cubeMapDesc.ArraySize = 6;
	cubeMapDesc.Usage = D3D11_USAGE_DEFAULT;
	cubeMapDesc.CPUAccessFlags = 0;
	cubeMapDesc.MipLevels = 1;
	cubeMapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	cubeMapDesc.SampleDesc.Count = 1;
	cubeMapDesc.SampleDesc.Quality = 0;
	//使用以上描述创建纹理
	ID3D11Texture2D *cubeMap_stencil(NULL);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&cubeMapDesc, 0, &cubeMap_stencil);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil texture error when create ssrinput tex");
		return error_message;
	}
	//创建六个rendertarget
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc_reflect;
	rtvDesc_reflect.Format = cubeMapDesc.Format;
	rtvDesc_reflect.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc_reflect.Texture2DArray.ArraySize = 1;
	rtvDesc_reflect.Texture2DArray.MipSlice = 0;
	for (UINT i = 0; i < 6; ++i)
	{
		rtvDesc_reflect.Texture2DArray.FirstArraySlice = i;
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(cubeMap_stencil, &rtvDesc_reflect, &reflect_cubestencil_RTV[i]);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil texture RTV error when create ssrinput tex");
			return error_message;
		}
	}
	//创建一个SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc_reflect;
	srvDesc_reflect.Format = cubeMapDesc.Format;
	srvDesc_reflect.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc_reflect.TextureCube.MipLevels = 1;
	srvDesc_reflect.TextureCube.MostDetailedMip = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(cubeMap_stencil, &srvDesc_reflect, &reflect_cubestencil_SRV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil texture SRV error when create ssrinput tex");
		return error_message;
	}
	cubeMap_stencil->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建cube面信息记录纹理后台缓冲~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//使用以上描述创建纹理
	ID3D11Texture2D *cubeMap_stencil_backbuffer(NULL);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&cubeMapDesc, 0, &cubeMap_stencil_backbuffer);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil backbuffer error when create ssrinput tex");
		return error_message;
	}
	//创建六个rendertarget
	for (UINT i = 0; i < 6; ++i)
	{
		rtvDesc_reflect.Texture2DArray.FirstArraySlice = i;
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(cubeMap_stencil_backbuffer, &rtvDesc_reflect, &reflect_cubestencil_RTV_backbuffer[i]);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil backbuffer RTV error when create ssrinput tex");
			return error_message;
		}
	}
	//创建一个SRV
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(cubeMap_stencil_backbuffer, &srvDesc_reflect, &reflect_cubestencil_SRV_backbuffer);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil backbuffer SRV error when create ssrinput tex");
		return error_message;
	}
	cubeMap_stencil_backbuffer->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
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
	//~~~~~~~~~~~~~~~大气光照掩码纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
	ID3D11Texture2D* atmospheremask_buf = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &atmospheremask_buf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create 4xMSAA atmosphere_buf texture error when create gbuffer");
		return error_message;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(atmospheremask_buf, 0, &AtmosphereMask_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create 4xMSAA atmosphere_buf rendertargetview error when create gbuffer");
		return error_message;
	}
	safe_release(atmospheremask_buf);
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	ID3D11Texture2D* atmosphere_singlebuf = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &atmosphere_singlebuf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA atmospheremask texture error when create gbuffer");
		return error_message;
	}
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(atmosphere_singlebuf, 0, &AtmosphereMask_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA atmospheremask shaderresourceview error when create gbuffer");
		return error_message;
	}
	safe_release(atmosphere_singlebuf);


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~镜面光&粗糙度纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
	ID3D11Texture2D* specrough_buf = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &specrough_buf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create 4xMSAA specrough_buf texture error when create gbuffer");
		return error_message;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(specrough_buf, 0, &specroughness_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create 4xMSAA specrough_buf rendertargetview error when create gbuffer");
		return error_message;
	}
	safe_release(specrough_buf);
	//非抗锯齿镜面光&粗糙度纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	ID3D11Texture2D* specrough_buf_singlebuf = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &specrough_buf_singlebuf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA specrough_buf texture error when create gbuffer");
		return error_message;
	}
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(specrough_buf_singlebuf, 0, &specroughness_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA specrough_buf shaderresourceview error when create gbuffer");
		return error_message;
	}
	safe_release(specrough_buf_singlebuf);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~光照信息存储纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	ID3D11Texture2D *diffuse_buf = 0, *specular_buf = 0, *atmosphere_buf;
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
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &atmosphere_buf);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA atmosphere texture error when create gbuffer");
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

	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(atmosphere_buf, 0, &gbuffer_atmosphere_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA atmospherelight ShaderResourceView error when create gbuffer");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(atmosphere_buf, 0, &gbuffer_atmosphere_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create noMSAA atmospherelight RenderTargetView error when create gbuffer");
		return error_message;
	}
	safe_release(diffuse_buf);
	safe_release(specular_buf);
	safe_release(atmosphere_buf);
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



	//建立后处理渲染目标
	D3D11_TEXTURE2D_DESC desc_back;
	desc_back.Width = map_width;
	desc_back.Height = map_height;
	desc_back.MipLevels = 1;
	desc_back.ArraySize = 1;
	desc_back.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc_back.SampleDesc.Count = 4;
	desc_back.SampleDesc.Quality = 0;
	desc_back.Usage = D3D11_USAGE_DEFAULT;
	desc_back.BindFlags = D3D11_BIND_RENDER_TARGET;
	desc_back.CPUAccessFlags = 0;
	desc_back.MiscFlags = 0;
	ID3D11Texture2D *posttreat_buffer;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&desc_back, 0, &posttreat_buffer);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect posttreat texturedata error when create posttreat tex");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(posttreat_buffer, 0, &posttreatment_RTV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_reason(hr, "create reflect posttreat texture RTV error when create posttreat tex");
		return failed_reason;
	}
	posttreat_buffer->Release();
	//建立反射标记渲染目标
	D3D11_TEXTURE2D_DESC texDesc_reflect;
	texDesc_reflect.Width = map_width;
	texDesc_reflect.Height = map_height;
	texDesc_reflect.MipLevels = 1;
	texDesc_reflect.ArraySize = 1;
	texDesc_reflect.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc_reflect.SampleDesc.Count = 4;
	texDesc_reflect.SampleDesc.Quality = 0;
	texDesc_reflect.Usage = D3D11_USAGE_DEFAULT;
	texDesc_reflect.BindFlags = D3D11_BIND_RENDER_TARGET;
	texDesc_reflect.CPUAccessFlags = 0;
	texDesc_reflect.MiscFlags = 0;
	ID3D11Texture2D* reflectmask_tex = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc_reflect, 0, &reflectmask_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_reason(hr, "create reflectmask render target tex error when create posttreat tex");
		return failed_reason;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(reflectmask_tex, 0, &reflectmask_RTV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_reason(hr, "create reflectmask render target view error when create posttreat tex");
		return failed_reason;
	}
	reflectmask_tex->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
*/
void Pretreatment_gbuffer::set_normalspecdepth_target(gbuffer_render_target render_target_out)
{
	ID3D11RenderTargetView* renderTargets[3] = { render_target_out.normalspec_target,render_target_out.specroughness_target,render_target_out.AtmosphereMask_target };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(3, renderTargets, render_target_out.depthmap_target);
	//d3d_pancy_basic_singleton::GetInstance()->set_render_target(normalspec_target, depthmap_target);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(render_target_out.normalspec_target, clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(render_target_out.specroughness_target, clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(render_target_out.AtmosphereMask_target, clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearDepthStencilView(render_target_out.depthmap_target, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
/*
void Pretreatment_gbuffer::set_reflect_normaldepth_target()
{
	ID3D11RenderTargetView* renderTargets[3] = { reflect_cubenormal_RTV,reflect_cubeSpecRough_RTV,reflect_AtmosphereMask_RTV };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(3, renderTargets, reflect_DSV);
	float clearColor[] = { 0.0f, 0.0f, -1.0f, 1e5f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(reflect_cubenormal_RTV, clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(reflect_cubeSpecRough_RTV, clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(reflect_AtmosphereMask_RTV, clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearDepthStencilView(reflect_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
void Pretreatment_gbuffer::set_reflect_savedepth_target(int count)
{
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(reflect_cubestencil_RTV_backbuffer[count], NULL);
	float clearColor[] = { 0.0f, 0.0f, -1.0f, 1e5f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(reflect_cubestencil_RTV_backbuffer[count], clearColor);
}
*/
void Pretreatment_gbuffer::set_multirender_target(gbuffer_render_target render_target_out)
{
	ID3D11RenderTargetView* renderTargets[3] = { render_target_out.gbuffer_diffuse_target,render_target_out.gbuffer_specular_target,render_target_out.gbuffer_atmosphere_target };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(3, renderTargets, NULL);
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
/*
void Pretreatment_gbuffer::set_reflect_multirender_target()
{
	ID3D11RenderTargetView* renderTargets[3] = { reflect_diffuse_target,reflect_specular_target,reflect_atmosphere_target };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(3, renderTargets, NULL);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(reflect_diffuse_target, clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(reflect_specular_target, clearColor);
}
void Pretreatment_gbuffer::set_posttreat_input_target()
{
	ID3D11RenderTargetView* renderTargets[2] = { posttreatment_RTV,reflectmask_RTV };
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(renderTargets, 2);
	//d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(2, renderTargets, NULL);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float mask_clearColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(posttreatment_RTV, clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(reflectmask_RTV, mask_clearColor);
}

void Pretreatment_gbuffer::release_texture()
{
	safe_release(depthmap_tex);
	safe_release(depthmap_target);
	safe_release(normalspec_target);
	safe_release(normalspec_tex);
	safe_release(specroughness_target);
	safe_release(specroughness_tex);
	safe_release(gbuffer_diffuse_target);
	safe_release(gbuffer_diffuse_tex);
	safe_release(gbuffer_specular_target);
	safe_release(gbuffer_specular_tex);
	safe_release(gbuffer_atmosphere_target);
	safe_release(gbuffer_atmosphere_tex);
	safe_release(depthmap_single_target);
	safe_release(depthmap_single_tex);
	safe_release(posttreatment_RTV);
	safe_release(reflectmask_RTV);
	safe_release(AtmosphereMask_target);
	safe_release(AtmosphereMask_tex);
}
void Pretreatment_gbuffer::upadte_reflect_render_face()
{
	if (now_reflect_render_face == 11)
	{
		environment_map_renderplace = environment_map_place;
		pancy_camera::get_instance()->get_view_position(&environment_map_place);
		std::swap(reflect_cubestencil_SRV_backbuffer, reflect_cubestencil_SRV);
		for (int i = 0; i < 6; ++i)
		{
			std::swap(reflect_cubestencil_RTV[i], reflect_cubestencil_RTV_backbuffer[i]);
		}
	}
	now_reflect_render_face = (now_reflect_render_face + 1) % 12;
}

void Pretreatment_gbuffer::render_gbuffer()
{
	//关闭alpha混合
	set_normalspecdepth_target();
	//绘制gbuffer
	XMFLOAT4X4 view_mat;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	pancy_geometry_control_singleton::get_instance()->render_gbuffer(view_mat, engine_basic::perspective_message::get_instance()->get_proj_matrix(), false);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~存储法线镜面反射光纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11Resource * normalDepthTex = 0;
	ID3D11Resource * normalDepthTex_singlesample = 0;
	normalspec_target->GetResource(&normalDepthTex);
	normalspec_tex->GetResource(&normalDepthTex_singlesample);
	//将多重采样纹理转换至非多重纹理
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ResolveSubresource(normalDepthTex_singlesample, D3D11CalcSubresource(0, 0, 1), normalDepthTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	normalspec_tex->Release();
	normalspec_tex = NULL;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(normalDepthTex_singlesample, 0, &normalspec_tex);
	normalDepthTex->Release();
	normalDepthTex_singlesample->Release();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~存储镜面反射粗糙度纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11Resource * specroughnessTex = 0;
	ID3D11Resource * specroughnessTex_singlesample = 0;
	specroughness_target->GetResource(&specroughnessTex);
	specroughness_tex->GetResource(&specroughnessTex_singlesample);
	//将多重采样纹理转换至非多重纹理
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ResolveSubresource(specroughnessTex_singlesample, D3D11CalcSubresource(0, 0, 1), specroughnessTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	specroughness_tex->Release();
	specroughness_tex = NULL;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(specroughnessTex_singlesample, 0, &specroughness_tex);
	specroughnessTex->Release();
	specroughnessTex_singlesample->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~存储大气掩码纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11Resource * atmospheremaslTex = 0;
	ID3D11Resource * atmospheremaslTex_singlesample = 0;
	AtmosphereMask_target->GetResource(&atmospheremaslTex);
	AtmosphereMask_tex->GetResource(&atmospheremaslTex_singlesample);
	//将多重采样纹理转换至非多重纹理
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ResolveSubresource(atmospheremaslTex_singlesample, D3D11CalcSubresource(0, 0, 1), atmospheremaslTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	AtmosphereMask_tex->Release();
	AtmosphereMask_tex = NULL;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(atmospheremaslTex_singlesample, 0, &AtmosphereMask_tex);
	atmospheremaslTex->Release();
	atmospheremaslTex_singlesample->Release();

	engine_basic::engine_fail_reason check_error;
	auto shader_atmosphere = shader_control::GetInstance()->get_shader_atmosphere_render(check_error);
	shader_atmosphere->set_tex_mask(AtmosphereMask_tex);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~msaa-shader重采样~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	set_resolvdepth_target();
	//	engine_basic::engine_fail_reason check_error;
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
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, NULL_target, NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech_need->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
	//绘制立方深度法线图
	D3D11_VIEWPORT shadow_map_VP;
	shadow_map_VP.TopLeftX = 0.0f;
	shadow_map_VP.TopLeftY = 0.0f;
	shadow_map_VP.Width = 1024.0f * quality_reflect;
	shadow_map_VP.Height = 1024.0f * quality_reflect;
	shadow_map_VP.MinDepth = 0.0f;
	shadow_map_VP.MaxDepth = 1.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &shadow_map_VP);
	if (now_reflect_render_face % 2 == 0 && last_reflect_render_face != now_reflect_render_face)
	{
		set_reflect_normaldepth_target();
		XMFLOAT4X4 Proj_mat_reflect;
		XMStoreFloat4x4(&Proj_mat_reflect, DirectX::XMMatrixPerspectiveFovLH(0.5f*XM_PI, 1.0f, engine_basic::perspective_message::get_instance()->get_perspective_near_plane(), engine_basic::perspective_message::get_instance()->get_perspective_far_plane()));
		XMFLOAT3 look_vec, up_vec;
		XMFLOAT4X4 view_matrix_reflect;
		look_vec = look_cube_reflect[now_reflect_render_face / 2];
		up_vec = up_cube_reflect[now_reflect_render_face / 2];
		pancy_camera::get_instance()->count_view_matrix(look_vec, up_vec, environment_map_place, &view_matrix_reflect);
		pancy_geometry_control_singleton::get_instance()->render_gbuffer(view_matrix_reflect, Proj_mat_reflect, true);
		//渲染深度到立方模板纹理贴图
		auto shader_save_depth = shader_control::GetInstance()->get_shader_reflect_savedepth(check_error);
		shader_save_depth->set_cube_count(XMFLOAT3(now_reflect_render_face / 2, 0.0f, 0.0f));
		shader_save_depth->set_depthtex_input(reflect_depthcube_SRV);
		set_reflect_savedepth_target(now_reflect_render_face / 2);
		ID3DX11EffectTechnique *tech_need;
		shader_save_depth->get_technique(&tech_need, "resolove_alpha");
		resolve_depth_render(tech_need);
		shader_save_depth->set_depthtex_input(NULL);
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, NULL_target, NULL);
		tech_need->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			tech_need->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
		}
	}
	d3d_pancy_basic_singleton::GetInstance()->reset_viewport();
}
void Pretreatment_gbuffer::render_lbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 invview_matrix, bool if_shadow)
{
	//渲染正方向光照纹理
	set_multirender_target();
	engine_basic::engine_fail_reason check_error;
	auto lbuffer_shader = shader_control::GetInstance()->get_shader_lightbuffer(check_error);
	lbuffer_shader->set_DepthMap_tex(depthmap_single_tex);
	lbuffer_shader->set_Normalspec_tex(normalspec_tex);
	lbuffer_shader->set_SpecRoughness_tex(specroughness_tex);
	XMFLOAT4 farcorner[4];
	engine_basic::perspective_message::get_instance()->get_FrustumFarCorner(farcorner);
	lbuffer_shader->set_FrustumCorners(farcorner);


	XMFLOAT4X4 view_mat, invview_mat;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	pancy_camera::get_instance()->count_invview_matrix(&invview_mat);
	lbuffer_shader->set_view_matrix(&view_mat);
	lbuffer_shader->set_invview_matrix(&invview_mat);

	XMFLOAT3 camera_pos;
	pancy_camera::get_instance()->get_view_position(&camera_pos);
	lbuffer_shader->set_camera(camera_pos);
	lbuffer_shader->set_exposure(10.0f);
	lbuffer_shader->set_tex_mask(AtmosphereMask_tex);


	ID3DX11EffectTechnique *tech_need;
	if (if_shadow == true)
	{
		lbuffer_shader->get_technique(&tech_need, "draw_common_pbr");
	}
	else
	{
		lbuffer_shader->get_technique(&tech_need, "draw_pbr_withoutshadow");
	}
	light_buffer_render(tech_need);
	auto shader_sky = shader_control::GetInstance()->get_shader_sky_draw(check_error);
	shader_sky->set_tex_atmosphere(gbuffer_atmosphere_tex);

	//渲染反射方向光照纹理
	D3D11_VIEWPORT shadow_map_VP;
	shadow_map_VP.TopLeftX = 0.0f;
	shadow_map_VP.TopLeftY = 0.0f;
	shadow_map_VP.Width = 1024.0f * quality_reflect;
	shadow_map_VP.Height = 1024.0f * quality_reflect;
	shadow_map_VP.MinDepth = 0.0f;
	shadow_map_VP.MaxDepth = 1.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &shadow_map_VP);
	if (now_reflect_render_face % 2 == 0 && last_reflect_render_face != now_reflect_render_face)
	{
		last_reflect_render_face = now_reflect_render_face;
		set_reflect_multirender_target();
		float halfwidth = engine_basic::perspective_message::get_instance()->get_perspective_far_plane() * tanf(0.5f*0.5f*XM_PI);
		XMFLOAT4 reflectcube_FarCorner[4];
		reflectcube_FarCorner[0] = DirectX::XMFLOAT4(-halfwidth, -halfwidth, engine_basic::perspective_message::get_instance()->get_perspective_far_plane(), 1.0f);
		reflectcube_FarCorner[1] = DirectX::XMFLOAT4(-halfwidth, +halfwidth, engine_basic::perspective_message::get_instance()->get_perspective_far_plane(), 1.0f);
		reflectcube_FarCorner[2] = DirectX::XMFLOAT4(+halfwidth, +halfwidth, engine_basic::perspective_message::get_instance()->get_perspective_far_plane(), 1.0f);
		reflectcube_FarCorner[3] = DirectX::XMFLOAT4(+halfwidth, -halfwidth, engine_basic::perspective_message::get_instance()->get_perspective_far_plane(), 1.0f);
		XMFLOAT4X4 Proj_mat_reflect;
		XMStoreFloat4x4(&Proj_mat_reflect, DirectX::XMMatrixPerspectiveFovLH(0.5f*XM_PI, 1.0f, engine_basic::perspective_message::get_instance()->get_perspective_near_plane(), engine_basic::perspective_message::get_instance()->get_perspective_far_plane()));
		XMFLOAT3 rec_proj_vec;
		rec_proj_vec.x = 1.0f / Proj_mat_reflect._43;
		rec_proj_vec.y = -Proj_mat_reflect._33 / Proj_mat_reflect._43;
		rec_proj_vec.z = 0.0f;
		lbuffer_shader->set_projmessage(rec_proj_vec);
		lbuffer_shader->set_DepthMap_tex(reflect_depthcube_SRV);
		lbuffer_shader->set_Normalspec_tex(reflect_cubenormal_SRV);
		lbuffer_shader->set_SpecRoughness_tex(reflect_cubeSpecRough_SRV);
		lbuffer_shader->set_FrustumCorners(reflectcube_FarCorner);
		//计算取景变换
		XMFLOAT3 look_vec, up_vec;
		XMFLOAT4X4 view_matrix_reflect, invview_matrix_reflect;
		look_vec = look_cube_reflect[now_reflect_render_face / 2];
		up_vec = up_cube_reflect[now_reflect_render_face / 2];
		pancy_camera::get_instance()->count_view_matrix(look_vec, up_vec, environment_map_place, &view_matrix_reflect);
		pancy_camera::get_instance()->count_invview_matrix(look_vec, up_vec, environment_map_place, &invview_matrix_reflect);
		lbuffer_shader->set_view_matrix(&view_matrix_reflect);
		lbuffer_shader->set_invview_matrix(&invview_matrix_reflect);

		if (if_shadow == true)
		{
			lbuffer_shader->get_technique(&tech_need, "draw_pbr_withoutMSAA");
		}
		else
		{
			lbuffer_shader->get_technique(&tech_need, "draw_pbr_withoutshadowMSAA");
		}

		light_buffer_render(tech_need);
	}
	d3d_pancy_basic_singleton::GetInstance()->reset_viewport();
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
*/
/*
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
	shader_deffered->set_normal_tex(normalspec_tex);
	shader_deffered->set_tex_specroughness_resource(specroughness_tex);
}
*/
void Pretreatment_gbuffer::release()
{
	fullscreen_Lbuffer->release();
	fullscreen_buffer->release();
	/*
	safe_release(reflect_cubenormal_SRV);
	safe_release(reflect_cubenormal_RTV);

	safe_release(reflect_cubeSpecRough_SRV);
	safe_release(reflect_cubeSpecRough_RTV);

	safe_release(reflect_AtmosphereMask_SRV);
	safe_release(reflect_AtmosphereMask_RTV);

	safe_release(reflect_depthcube_SRV);
	safe_release(reflect_DSV);
	// 全局反射lbuffer开启的纹理
	safe_release(reflect_diffuse_target);
	safe_release(reflect_diffuse_tex);
	safe_release(reflect_specular_target);
	safe_release(reflect_specular_tex);
	safe_release(reflect_atmosphere_target);
	safe_release(reflect_atmosphere_tex);

	reflect_cubestencil_SRV->Release();
	reflect_cubestencil_SRV_backbuffer->Release();
	for (int i = 0; i < 6; ++i)
	{
		reflect_cubestencil_RTV[i]->Release();
		reflect_cubestencil_RTV_backbuffer[i]->Release();
	}
	release_texture();
	*/
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
	bool if_static
	)
{
	//engine_basic::engine_fail_reason check_error;
	//设置渲染目标及视口
	set_normalspecdepth_target(*render_target_out);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &render_target_out->render_viewport);
	//开始渲染
	geometry_list->render_gbuffer(view_matrix, perspective_message->get_proj_matrix(), if_static);
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

/*
void Pretreatment_gbuffer::render_lbuffer_cube(XMFLOAT4X4 view_matrix, XMFLOAT4X4 invview_matrix, bool if_shadow)
{
	engine_basic::engine_fail_reason check_error;
	auto lbuffer_shader = shader_control::GetInstance()->get_shader_lightbuffer(check_error);
	last_reflect_render_face = now_reflect_render_face;
	set_reflect_multirender_target();
	float halfwidth = engine_basic::perspective_message::get_instance()->get_perspective_far_plane() * tanf(0.5f*0.5f*XM_PI);
	XMFLOAT4 reflectcube_FarCorner[4];
	reflectcube_FarCorner[0] = DirectX::XMFLOAT4(-halfwidth, -halfwidth, engine_basic::perspective_message::get_instance()->get_perspective_far_plane(), 1.0f);
	reflectcube_FarCorner[1] = DirectX::XMFLOAT4(-halfwidth, +halfwidth, engine_basic::perspective_message::get_instance()->get_perspective_far_plane(), 1.0f);
	reflectcube_FarCorner[2] = DirectX::XMFLOAT4(+halfwidth, +halfwidth, engine_basic::perspective_message::get_instance()->get_perspective_far_plane(), 1.0f);
	reflectcube_FarCorner[3] = DirectX::XMFLOAT4(+halfwidth, -halfwidth, engine_basic::perspective_message::get_instance()->get_perspective_far_plane(), 1.0f);
	XMFLOAT4X4 Proj_mat_reflect;
	XMStoreFloat4x4(&Proj_mat_reflect, DirectX::XMMatrixPerspectiveFovLH(0.5f*XM_PI, 1.0f, engine_basic::perspective_message::get_instance()->get_perspective_near_plane(), engine_basic::perspective_message::get_instance()->get_perspective_far_plane()));
	XMFLOAT3 rec_proj_vec;
	rec_proj_vec.x = 1.0f / Proj_mat_reflect._43;
	rec_proj_vec.y = -Proj_mat_reflect._33 / Proj_mat_reflect._43;
	rec_proj_vec.z = 0.0f;
	lbuffer_shader->set_projmessage(rec_proj_vec);
	lbuffer_shader->set_DepthMap_tex(reflect_depthcube_SRV);
	lbuffer_shader->set_Normalspec_tex(reflect_cubenormal_SRV);
	lbuffer_shader->set_SpecRoughness_tex(reflect_cubeSpecRough_SRV);
	lbuffer_shader->set_FrustumCorners(reflectcube_FarCorner);
	//计算取景变换
	lbuffer_shader->set_view_matrix(&view_matrix);
	lbuffer_shader->set_invview_matrix(&invview_matrix);
	ID3DX11EffectTechnique *tech_need;
	if (if_shadow == true)
	{
		lbuffer_shader->get_technique(&tech_need, "draw_pbr_withoutMSAA");
	}
	else
	{
		lbuffer_shader->get_technique(&tech_need, "draw_pbr_withoutshadowMSAA");
	}

	light_buffer_render(tech_need);
}

void Pretreatment_gbuffer::display(XMFLOAT4X4 view_matrix)
{
	render_gbuffer(view_matrix);
}
void Pretreatment_gbuffer::display_lbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 invview_matrix, bool if_shadow) 
{
	render_lbuffer_cube(view_matrix, invview_matrix, if_shadow);
	engine_basic::engine_fail_reason check_error;
	auto shader_deffered = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	shader_deffered->set_diffuse_light_tex(gbuffer_diffuse_tex);
	shader_deffered->set_specular_light_tex(gbuffer_specular_tex);
	shader_deffered->set_normal_tex(normalspec_tex);
	shader_deffered->set_tex_specroughness_resource(specroughness_tex);
	auto shader_sky = shader_control::GetInstance()->get_shader_sky_draw(check_error);
	shader_sky->set_tex_atmosphere(reflect_atmosphere_tex);
}
*/
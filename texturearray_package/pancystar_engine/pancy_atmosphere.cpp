#include"pancy_atmosphere.h"
atmosphere_pretreatment::atmosphere_pretreatment()
{
}
engine_basic::engine_fail_reason atmosphere_pretreatment::create()
{
	//开启透明  
	D3D11_BLEND_DESC transDesc;
	//先创建一个混合状态的描述  
	transDesc.AlphaToCoverageEnable = false;
	transDesc.IndependentBlendEnable = true;
	for (int i = 0; i < 8; ++i)
	{
		transDesc.RenderTarget[i].BlendEnable = false;
		transDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		transDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
		transDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		transDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		transDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
		transDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		transDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}


	transDesc.RenderTarget[1].BlendEnable = true;
	transDesc.RenderTarget[1].SrcBlend = D3D11_BLEND_ONE;
	transDesc.RenderTarget[1].DestBlend = D3D11_BLEND_ONE;
	transDesc.RenderTarget[1].BlendOp = D3D11_BLEND_OP_ADD;
	transDesc.RenderTarget[1].SrcBlendAlpha = D3D11_BLEND_ONE;
	transDesc.RenderTarget[1].DestBlendAlpha = D3D11_BLEND_ONE;
	transDesc.RenderTarget[1].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transDesc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	//创建ID3D11BlendState接口  
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBlendState(&transDesc, &add_blend);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create blend state error");
		return error_message;
	}
	auto check_error = init_texture();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	fullscreen_buffer = new mesh_square(false);
	check_error = fullscreen_buffer->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	fullscreen_aobuffer = new mesh_aosquare(false);
	check_error = fullscreen_aobuffer->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}


	build_atomosphere_texture();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason atmosphere_pretreatment::init_texture_2D(int width_tex, int height_tex, ID3D11ShaderResourceView **SRV_input, ID3D11RenderTargetView **RTV_input)
{
	HRESULT hr;
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width_tex;
	texDesc.Height = height_tex;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* tex2d_resource = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &tex2d_resource);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message2(hr, "create texture error");
		return error_message2;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(tex2d_resource, 0, SRV_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message2(hr, "create SRV error");
		return error_message2;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(tex2d_resource, 0, RTV_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message2(hr, "create RTV error");
		return error_message2;
	}
	tex2d_resource->Release();
	float clear_clor[] = { 0.0f,0.0f,0.0f,0.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(*RTV_input, clear_clor);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason atmosphere_pretreatment::init_texture_3D(int width_tex, int height_tex, int depth_tex, ID3D11ShaderResourceView **SRV_input, std::vector<ID3D11RenderTargetView*> &RTV_input)
{
	HRESULT hr;
	D3D11_TEXTURE3D_DESC texDesc;
	texDesc.Width = width_tex;
	texDesc.Height = height_tex;
	texDesc.Depth = depth_tex;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture3D* tex3d_resource = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture3D(&texDesc, 0, &tex3d_resource);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message2(hr, "create texture error");
		return error_message2;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(tex3d_resource, 0, SRV_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message2(hr, "create SRV error");
		return error_message2;
	}
	D3D11_RENDER_TARGET_VIEW_DESC rtv_stencil_Desc;
	rtv_stencil_Desc.Format = texDesc.Format;
	rtv_stencil_Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
	rtv_stencil_Desc.Texture3D.WSize = 1;
	for (int i = 0; i < depth_tex; ++i)
	{
		rtv_stencil_Desc.Texture3D.FirstWSlice = i;
		rtv_stencil_Desc.Texture3D.MipSlice = 0;
		ID3D11RenderTargetView *RTV_per_layer;
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(tex3d_resource, &rtv_stencil_Desc, &RTV_per_layer);
		float clear_clor[] = { 0.0f,0.0f,0.0f,0.0f };
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message2(hr, "create RTV error");
			return error_message2;
		}
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(RTV_per_layer, clear_clor);
		RTV_input.push_back(RTV_per_layer);
	}
	tex3d_resource->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason atmosphere_pretreatment::init_texture()
{
	//透射率
	engine_basic::engine_fail_reason check_error = init_texture_2D(256, 64, &transmittance_SRV, &transmittance_RTV);
	if (!check_error.check_if_failed())
	{
		engine_basic::engine_fail_reason error_message2("create transmittance texture error");
		return error_message2;
	}
	//光照
	check_error = init_texture_2D(64, 16, &Irradiance_SRV, &Irradiance_RTV);
	if (!check_error.check_if_failed())
	{
		engine_basic::engine_fail_reason error_message2("create Irradiance texture error");
		return error_message2;
	}
	//散射
	check_error = init_texture_3D(256, 128, 32, &Scattering_SRV, Scattering_RTV);
	if (!check_error.check_if_failed())
	{
		engine_basic::engine_fail_reason error_message2("create Scattering texture error");
		return error_message2;
	}
	//米氏散射
	check_error = init_texture_3D(256, 128, 32, &SinglMieScattering_SRV, SingleMieScattering_RTV);
	if (!check_error.check_if_failed())
	{
		engine_basic::engine_fail_reason error_message2("create SinglMieScattering texture error");
		return error_message2;
	}
	//散射密度
	check_error = init_texture_3D(256, 128, 32, &delta_scattering_density_SRV, delta_scattering_density_RTV);
	if (!check_error.check_if_failed())
	{
		engine_basic::engine_fail_reason error_message2("create scattering_density texture error");
		return error_message2;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~临时渲染纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//临时光照
	check_error = init_texture_2D(64, 16, &delta_Irradiance_SRV, &delta_Irradiance_RTV);
	if (!check_error.check_if_failed())
	{
		engine_basic::engine_fail_reason error_message2("create delta_Irradiance texture error");
		return error_message2;
	}
	//临时米氏散射
	check_error = init_texture_3D(256, 128, 32, &delta_MieScattering_SRV, delta_MieScattering_RTV);
	if (!check_error.check_if_failed())
	{
		engine_basic::engine_fail_reason error_message2("create MieScattering texture error");
		return error_message2;
	}
	//临时瑞利散射
	check_error = init_texture_3D(256, 128, 32, &delta_rayleigh_scattering_SRV, delta_rayleigh_scattering_RTV);
	if (!check_error.check_if_failed())
	{
		engine_basic::engine_fail_reason error_message2("create rayleigh_scattering texture error");
		return error_message2;
	}
	//临时多重散射
	check_error = init_texture_3D(256, 128, 32, &delta_multi_scattering_SRV, delta_multi_scattering_RTV);
	if (!check_error.check_if_failed())
	{
		engine_basic::engine_fail_reason error_message2("create multi_scattering texture error");
		return error_message2;
	}

	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void atmosphere_pretreatment::build_atomosphere_texture()
{
	draw_transmittance();
	draw_irradiance();
	draw_SingleScattering();
	for (int i = 2; i <= 4; ++i)
	{
		draw_Scattering_density(i);
		draw_indirect_irradiance(i);
		draw_multi_scattering(i);
	}
	engine_basic::engine_fail_reason check_error;
	auto shader_atmosphere_render = shader_control::GetInstance()->get_shader_atmosphere_render(check_error);
	auto shader_pretreat_lbuffer = shader_control::GetInstance()->get_shader_lightbuffer(check_error);
	//设定预处理纹理
	shader_atmosphere_render->set_tex_irradiance(Irradiance_SRV);
	shader_atmosphere_render->set_tex_scattering(Scattering_SRV);
	shader_atmosphere_render->set_tex_single_mie_scattering(SinglMieScattering_SRV);
	shader_atmosphere_render->set_tex_transmittance(transmittance_SRV);

	shader_pretreat_lbuffer->set_tex_irradiance(Irradiance_SRV);
	shader_pretreat_lbuffer->set_tex_scattering(Scattering_SRV);
	shader_pretreat_lbuffer->set_tex_single_mie_scattering(SinglMieScattering_SRV);
	shader_pretreat_lbuffer->set_tex_transmittance(transmittance_SRV);
	//~~~~~~~~~~~~~~设定固定参数~~~~~~~~~~~~~~~~~~~~~~~~~~~
	shader_atmosphere_render->set_white_point(XMFLOAT3(1.0f, 1.0f, 1.0f));
	shader_pretreat_lbuffer->set_white_point(XMFLOAT3(1.0f, 1.0f, 1.0f));
	//反投影
	const float kFovY = engine_basic::perspective_message::get_instance()->get_perspective_angle();
	//const float kFovY = engine_basic::perspective_message::get_instance()->get_perspective_angle();
	const float kTanFovY = tan(kFovY / 2.0);
	float aspect_ratio = static_cast<float>(d3d_pancy_basic_singleton::GetInstance()->get_wind_width()) / static_cast<float>(d3d_pancy_basic_singleton::GetInstance()->get_wind_height());
	XMFLOAT4X4 clip_matrix =
	{
		kTanFovY * aspect_ratio, 0.0, 0.0, 0.0,
		0.0, kTanFovY, 0.0, 0.0,
		0.0, 0.0, 0.0, -1.0,
		0.0, 0.0, 1.0, 1.0
	};
	shader_atmosphere_render->set_view_from_clip(clip_matrix);
	shader_pretreat_lbuffer->set_view_from_clip(clip_matrix);
	//地球信息
	float kBottomRadius = 6360000.0;
	float kLengthUnitInMeters = 1.0;
	shader_atmosphere_render->set_earth_center(XMFLOAT3(0.0f, -kBottomRadius / kLengthUnitInMeters, 0.0f));
	shader_pretreat_lbuffer->set_earth_center(XMFLOAT3(0.0f, -kBottomRadius / kLengthUnitInMeters, 0.0f));
	//太阳信息
	double kSunAngularRadius = 0.00935 / 2.0;
	shader_atmosphere_render->set_sun_size(XMFLOAT2(tan(kSunAngularRadius), cos(kSunAngularRadius)));
	shader_pretreat_lbuffer->set_sun_size(XMFLOAT2(tan(kSunAngularRadius), cos(kSunAngularRadius)));
}
void atmosphere_pretreatment::draw_transmittance()
{
	engine_basic::engine_fail_reason check_error;


	d3d_pancy_basic_singleton::GetInstance()->set_render_target(transmittance_RTV, NULL);
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 256.0f;
	viewPort.Height = 64.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	auto shader_trans = shader_control::GetInstance()->get_shader_atmosphere_pretreat(check_error);
	ID3DX11EffectTechnique *teque_need;
	shader_trans->get_technique(&teque_need, "draw_transmit");
	fullscreen_buffer->get_teque(teque_need);
	fullscreen_buffer->show_mesh();
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
}
void atmosphere_pretreatment::draw_irradiance()
{
	engine_basic::engine_fail_reason check_error;
	ID3D11RenderTargetView* renderTargets[2] = { delta_Irradiance_RTV,Irradiance_RTV };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(2, renderTargets, NULL);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(delta_Irradiance_RTV, clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(Irradiance_RTV, clearColor);
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 64.0f;
	viewPort.Height = 16.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	auto shader_trans = shader_control::GetInstance()->get_shader_atmosphere_pretreat(check_error);
	shader_trans->set_tex_transmittance(transmittance_SRV);
	ID3DX11EffectTechnique *teque_need;
	shader_trans->get_technique(&teque_need, "draw_direct_irradiance");
	fullscreen_buffer->get_teque(teque_need);
	fullscreen_buffer->show_mesh();
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
}
void atmosphere_pretreatment::draw_SingleScattering()
{
	//alpha混合
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(add_blend, blendFactor, 0xffffffff);

	engine_basic::engine_fail_reason check_error;

	D3D11_VIEWPORT viewPort;
	viewPort.Width = 256.0f;
	viewPort.Height = 128.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	auto shader_trans = shader_control::GetInstance()->get_shader_atmosphere_pretreat(check_error);
	ID3DX11EffectTechnique *teque_need;
	shader_trans->set_tex_transmittance(transmittance_SRV);

	XMFLOAT4X4 mat_luminance;
	XMStoreFloat4x4(&mat_luminance, XMMatrixIdentity());
	shader_trans->set_luminance_from_radiance(mat_luminance);
	for (unsigned int i = 0; i < 32; ++i)
	{
		ID3D11RenderTargetView* renderTargets[4] = { delta_rayleigh_scattering_RTV[i],delta_MieScattering_RTV[i] ,Scattering_RTV[i], SingleMieScattering_RTV[i] };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(4, renderTargets, NULL);
		shader_trans->set_layer(i);
		shader_trans->get_technique(&teque_need, "draw_SingleScattering");
		fullscreen_buffer->get_teque(teque_need);
		fullscreen_buffer->show_mesh();
	}
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
	//还原状态
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(NULL, blendFactor, 0xffffffff);
}
void atmosphere_pretreatment::draw_Scattering_density(int layer_scattering)
{
	engine_basic::engine_fail_reason check_error;
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 256.0f;
	viewPort.Height = 128.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	auto shader_trans = shader_control::GetInstance()->get_shader_atmosphere_pretreat(check_error);
	ID3DX11EffectTechnique *teque_need;
	shader_trans->set_tex_transmittance(transmittance_SRV);
	shader_trans->set_tex_single_rayleigh_scattering(delta_rayleigh_scattering_SRV);
	shader_trans->set_tex_single_mie_scattering(delta_MieScattering_SRV);
	shader_trans->set_tex_multiple_scattering(delta_multi_scattering_SRV);
	shader_trans->set_tex_irradiance(delta_Irradiance_SRV);
	shader_trans->set_scattering_order(layer_scattering);
	for (unsigned int i = 0; i < 32; ++i)
	{
		d3d_pancy_basic_singleton::GetInstance()->set_render_target(delta_scattering_density_RTV[i], NULL);
		shader_trans->set_layer(i);
		shader_trans->get_technique(&teque_need, "draw_ScatteringDensity");
		fullscreen_buffer->get_teque(teque_need);
		fullscreen_buffer->show_mesh();
	}
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
}
void atmosphere_pretreatment::draw_indirect_irradiance(int layer_scattering)
{
	//alpha混合
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(add_blend, blendFactor, 0xffffffff);
	//正式渲染
	engine_basic::engine_fail_reason check_error;
	ID3D11RenderTargetView* renderTargets[2] = { delta_Irradiance_RTV,Irradiance_RTV };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(2, renderTargets, NULL);
	float black[] = { 0.0f,0.0f,0.0f,0.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(delta_Irradiance_RTV, black);
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 64.0f;
	viewPort.Height = 16.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	auto shader_trans = shader_control::GetInstance()->get_shader_atmosphere_pretreat(check_error);
	ID3DX11EffectTechnique *teque_need;
	XMFLOAT4X4 mat_luminance;
	XMStoreFloat4x4(&mat_luminance, XMMatrixIdentity());
	shader_trans->set_luminance_from_radiance(mat_luminance);
	shader_trans->set_tex_single_rayleigh_scattering(delta_rayleigh_scattering_SRV);
	shader_trans->set_tex_single_mie_scattering(delta_MieScattering_SRV);
	shader_trans->set_tex_multiple_scattering(delta_multi_scattering_SRV);
	shader_trans->set_scattering_order(layer_scattering - 1);

	shader_trans->get_technique(&teque_need, "draw_Indirectirradiance");
	fullscreen_buffer->get_teque(teque_need);
	fullscreen_buffer->show_mesh();
	//还原状态
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(NULL, blendFactor, 0xffffffff);
}
void atmosphere_pretreatment::draw_multi_scattering(int layer_scattering)
{
	//alpha混合
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(add_blend, blendFactor, 0xffffffff);
	//正式渲染
	engine_basic::engine_fail_reason check_error;
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 256.0f;
	viewPort.Height = 128.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	auto shader_trans = shader_control::GetInstance()->get_shader_atmosphere_pretreat(check_error);
	ID3DX11EffectTechnique *teque_need;
	shader_trans->set_tex_transmittance(transmittance_SRV);
	shader_trans->set_tex_scattering_density(delta_scattering_density_SRV);
	XMFLOAT4X4 mat_luminance;
	XMStoreFloat4x4(&mat_luminance, XMMatrixIdentity());
	shader_trans->set_luminance_from_radiance(mat_luminance);
	for (unsigned int i = 0; i < 32; ++i)
	{
		ID3D11RenderTargetView* renderTargets[2] = { delta_multi_scattering_RTV[i],Scattering_RTV[i] };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(2, renderTargets, NULL);
		float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(delta_multi_scattering_RTV[i], black);
		shader_trans->set_layer(i);
		shader_trans->get_technique(&teque_need, "draw_MultipleScattering");
		fullscreen_buffer->get_teque(teque_need);
		fullscreen_buffer->show_mesh();
	}
	//还原状态
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(NULL, blendFactor, 0xffffffff);
}
void atmosphere_pretreatment::display(XMFLOAT3 sundir)
{
	engine_basic::engine_fail_reason check_error;
	auto shader_atmosphere_render = shader_control::GetInstance()->get_shader_atmosphere_render(check_error);
	ID3DX11EffectTechnique *teque_need;
	XMFLOAT3 camera_pos;
	pancy_camera::get_instance()->get_view_position(&camera_pos);
	shader_atmosphere_render->set_camera(camera_pos);
	XMFLOAT4X4 inv_view_mat;
	pancy_camera::get_instance()->count_invview_matrix(&inv_view_mat);
	shader_atmosphere_render->set_model_from_view(inv_view_mat);
	//rec_ans = 0x00000036fe2ff328 {-0.972948968, 0.227391526, 0.0407850109}
	//XMFLOAT3 sundir(-0.935574710, 0.230530649, 0.267498821);
	//XMFLOAT3 sundir(-0.972948968, 0.227391526, 0.0407850109);
	shader_atmosphere_render->set_sun_direction(sundir);
	shader_atmosphere_render->set_exposure(10.0f);
	shader_atmosphere_render->get_technique(&teque_need, "draw_atmosphere");
	fullscreen_aobuffer->get_teque(teque_need);
	fullscreen_aobuffer->show_mesh();
}
void atmosphere_pretreatment::release()
{
	add_blend->Release();
	fullscreen_aobuffer->release();
	//临时数据
	delta_Irradiance_SRV->Release();
	delta_Irradiance_RTV->Release();
	delta_MieScattering_SRV->Release();
	delta_rayleigh_scattering_SRV->Release();
	delta_multi_scattering_SRV->Release();
	delta_scattering_density_SRV->Release();
	for (auto data = delta_MieScattering_RTV.begin(); data != delta_MieScattering_RTV.end(); ++data)
	{
		(*data._Ptr)->Release();
	}
	for (auto data = delta_rayleigh_scattering_RTV.begin(); data != delta_rayleigh_scattering_RTV.end(); ++data)
	{
		(*data._Ptr)->Release();
	}
	for (auto data = delta_multi_scattering_RTV.begin(); data != delta_multi_scattering_RTV.end(); ++data)
	{
		(*data._Ptr)->Release();
	}
	for (auto data = delta_scattering_density_RTV.begin(); data != delta_scattering_density_RTV.end(); ++data)
	{
		(*data._Ptr)->Release();
	}
	//全局数据
	transmittance_SRV->Release();
	transmittance_RTV->Release();
	Irradiance_SRV->Release();
	Irradiance_RTV->Release();
	Scattering_SRV->Release();
	SinglMieScattering_SRV->Release();
	fullscreen_buffer->release();
	for (auto data = Scattering_RTV.begin(); data != Scattering_RTV.end(); ++data)
	{
		(*data._Ptr)->Release();
	}
	for (auto data = SingleMieScattering_RTV.begin(); data != SingleMieScattering_RTV.end(); ++data)
	{
		(*data._Ptr)->Release();
	}
}
#include"pancy_scene_design.h"
#include<math.h>
scene_root::scene_root()
{
	time_game = 0.0f;
}

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
		engine_basic::engine_fail_reason error_message(hr,"create blend state error");
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
	//设定预处理纹理
	shader_atmosphere_render->set_tex_irradiance(Irradiance_SRV);
	shader_atmosphere_render->set_tex_scattering(Scattering_SRV);
	shader_atmosphere_render->set_tex_single_mie_scattering(SinglMieScattering_SRV);
	shader_atmosphere_render->set_tex_transmittance(transmittance_SRV);
	//设定固定参数
	//白平衡
	shader_atmosphere_render->set_white_point(XMFLOAT3(1.0f, 1.0f, 1.0f));
	//反投影
	const float kFovY = engine_basic::perspective_message::get_instance()->get_perspective_angle();
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
	//地球信息
	float kBottomRadius = 6360000.0; 
	float kLengthUnitInMeters = 1000.0;
	shader_atmosphere_render->set_earth_center(XMFLOAT3(0.0f, 0.0f, kBottomRadius / kLengthUnitInMeters));
	//太阳信息
	double kSunAngularRadius = 0.00935 / 2.0;
	shader_atmosphere_render->set_sun_size(XMFLOAT2(tan(kSunAngularRadius),cos(kSunAngularRadius)));
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
	XMStoreFloat4x4(&mat_luminance,XMMatrixIdentity());
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
		d3d_pancy_basic_singleton::GetInstance()->set_render_target(delta_scattering_density_RTV[i],NULL);
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
	fullscreen_buffer->get_teque(teque_need);
	fullscreen_buffer->show_mesh();
}
void atmosphere_pretreatment::release()
{
	add_blend->Release();
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


scene_test_square::scene_test_square()
{
}
engine_basic::engine_fail_reason scene_test_square::create()
{
	XMFLOAT4X4 mat_world;
	XMStoreFloat4x4(&mat_world, XMMatrixIdentity());
	engine_basic::engine_fail_reason check_error = pancy_geometry_control_singleton::get_instance()->load_a_model_type("castelmodel\\outmodel.pancymesh", "castelmodel\\outmodel.pancymat", false, model_ID_castel);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = pancy_geometry_control_singleton::get_instance()->add_a_model_instance(model_ID_castel, mat_world, ID_model_castel);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	check_error = pancy_geometry_control_singleton::get_instance()->load_a_model_type("squaremodel\\outmodel.pancymesh", "squaremodel\\outmodel.pancymat", false, model_ID_floor);
	check_error = pancy_geometry_control_singleton::get_instance()->add_a_model_instance(model_ID_floor, mat_world, ID_model_floor);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}


	check_error = pancy_geometry_control_singleton::get_instance()->load_a_model_type("ballmodel\\outmodel.pancymesh", "ballmodel\\outmodel.pancymat", false, model_ID_sky);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = pancy_geometry_control_singleton::get_instance()->add_a_model_instance(model_ID_sky, mat_world, ID_model_sky);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	geometry_resource_view *data_view = NULL;
	pancy_geometry_control_singleton::get_instance()->get_a_model_type(&data_view, model_ID_sky);
	data_view->set_cull_front();

	check_error = pancy_geometry_control_singleton::get_instance()->load_a_model_type("testball\\testball.pancymesh", "testball\\testball.pancymat", true, model_ID_pbrtest);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = pancy_geometry_control_singleton::get_instance()->add_a_model_instance(model_ID_pbrtest, mat_world, ID_model_pbrtest);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	HRESULT hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), L"Texture_cube.dds", 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &tex_cubesky);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void scene_test_square::display()
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	shader_need->set_IBL_tex(tex_cubesky);
	show_pbr_test("LightTech");
	show_model_single("LightTech");
	show_sky_single();
	//show_floor_single();
}
void scene_test_square::display_environment(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix)
{
	show_model_single("LightTech_withoutao", &view_matrix, &proj_matrix);
	show_sky_single();
}
void scene_test_square::show_model_single(string tech_name, XMFLOAT4X4 *view_matrix, XMFLOAT4X4 *proj_matrix)
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);

	geometry_resource_view *data_view = NULL;
	pancy_geometry_control_singleton::get_instance()->get_a_model_type(&data_view, model_ID_castel);
	XMFLOAT4X4 view_mat, final_mat;
	if (view_matrix == NULL)
	{
		pancy_camera::get_instance()->count_view_matrix(&view_mat);
	}
	else
	{
		view_mat = *view_matrix;
	}
	XMMATRIX proj;
	if (proj_matrix == NULL)
	{
		proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	}
	else
	{
		proj = XMLoadFloat4x4(proj_matrix);
	}
	XMMATRIX rec_world = XMLoadFloat4x4(&data_view->get_matrix_list()[0]) * XMLoadFloat4x4(&view_mat) * proj;
	XMStoreFloat4x4(&final_mat, rec_world);


	shader_need->set_trans_world(&data_view->get_matrix_list()[0]);
	shader_need->set_trans_all(&final_mat);
	shader_need->set_tex_diffuse_array(data_view->get_texture());

	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.2f, 0.2f, 0.2f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = rec_ambient2;
	shader_need->set_material(test_Mt);

	ID3DX11EffectTechnique *teque_need;
	shader_need->get_technique(&teque_need, tech_name.c_str());

	data_view->get_technique(teque_need);

	data_view->draw(false);
}
void scene_test_square::show_floor_single()
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);

	geometry_resource_view *data_view = NULL;
	pancy_geometry_control_singleton::get_instance()->get_a_model_type(&data_view, model_ID_floor);
	XMFLOAT4X4 view_mat, final_mat, viewproj;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	XMMATRIX rec_world = XMLoadFloat4x4(&data_view->get_matrix_list()[0]) * XMLoadFloat4x4(&view_mat) * proj;
	XMStoreFloat4x4(&final_mat, rec_world);
	XMStoreFloat4x4(&viewproj, XMLoadFloat4x4(&view_mat) * proj);

	shader_need->set_trans_world(&data_view->get_matrix_list()[0]);
	shader_need->set_trans_all(&final_mat);
	shader_need->set_trans_viewproj(&viewproj);
	shader_need->set_tex_diffuse_array(data_view->get_texture());
	XMFLOAT3 view_pos;
	pancy_camera::get_instance()->get_view_position(&view_pos);
	shader_need->set_view_pos(view_pos);
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_need->set_material(test_Mt);

	ID3DX11EffectTechnique *teque_need;
	shader_need->get_technique(&teque_need, "LightTech");

	data_view->get_technique(teque_need);

	data_view->draw(false);
	/*
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_virtual_light(check_error);

	geometry_resource_view *data_view = NULL;
	pancy_geometry_control_singleton::get_instance()->get_a_model_type(&data_view, model_ID_floor);
	XMFLOAT4X4 view_mat, final_mat;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	XMMATRIX rec_world = XMLoadFloat4x4(&data_view->get_matrix_list()[0]) * XMLoadFloat4x4(&view_mat) * proj;
	XMStoreFloat4x4(&final_mat, rec_world);


	shader_need->set_trans_world(&data_view->get_matrix_list()[0]);
	shader_need->set_trans_all(&final_mat);
	shader_need->set_tex_diffuse_array(data_view->get_texture());
	ID3DX11EffectTechnique *teque_need;
	shader_need->get_technique(&teque_need, "light_tech_array");

	data_view->get_technique(teque_need);

	data_view->draw();
	*/
}
void scene_test_square::show_sky_single()
{
	engine_basic::engine_fail_reason check_error;
	auto shader_test = shader_control::GetInstance()->get_shader_sky_draw(check_error);

	geometry_resource_view *data_view = NULL;
	pancy_geometry_control_singleton::get_instance()->get_a_model_type(&data_view, model_ID_sky);
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_sky");
	shader_test->set_trans_world(&data_view->get_matrix_list()[0]);
	//设定立方贴图
	shader_test->set_tex_resource(tex_cubesky);
	//设定总变换
	XMFLOAT4X4 view_mat, final_mat, viewproj;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	XMMATRIX rec_world = XMLoadFloat4x4(&data_view->get_matrix_list()[0]) * XMLoadFloat4x4(&view_mat) * proj;


	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&data_view->get_matrix_list()[0]);
	XMMATRIX worldViewProj = world_matrix_rec*XMLoadFloat4x4(&view_mat)*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);

	data_view->get_technique(teque_need);

	data_view->draw(false);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetState(NULL);
}
void scene_test_square::show_pbr_test(string tech_name, XMFLOAT4X4 *view_matrix, XMFLOAT4X4 *proj_matrix)
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);

	geometry_resource_view *data_view = NULL;
	pancy_geometry_control_singleton::get_instance()->get_a_model_type(&data_view, model_ID_pbrtest);
	XMFLOAT4X4 view_mat, final_mat;
	if (view_matrix == NULL)
	{
		pancy_camera::get_instance()->count_view_matrix(&view_mat);
	}
	else
	{
		view_mat = *view_matrix;
	}
	XMMATRIX proj;
	if (proj_matrix == NULL)
	{
		proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	}
	else
	{
		proj = XMLoadFloat4x4(proj_matrix);
	}
	XMMATRIX rec_world = XMLoadFloat4x4(&data_view->get_matrix_list()[0]) * XMLoadFloat4x4(&view_mat) * proj;
	XMStoreFloat4x4(&final_mat, rec_world);


	shader_need->set_trans_world(&data_view->get_matrix_list()[0]);
	shader_need->set_trans_all(&final_mat);
	shader_need->set_tex_diffuse_array(data_view->get_texture());

	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.2f, 0.2f, 0.2f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = rec_ambient2;
	shader_need->set_material(test_Mt);

	ID3DX11EffectTechnique *teque_need;
	shader_need->get_technique(&teque_need, tech_name.c_str());

	data_view->get_technique(teque_need);

	data_view->draw(false);
}
void scene_test_square::update(float delta_time)
{
	float move_speed = 0.025f;
	XMMATRIX view;
	auto user_input = pancy_input::GetInstance();
	auto scene_camera = pancy_camera::get_instance();
	//user_input->get_input();
	if (user_input->check_keyboard(DIK_A))
	{
		scene_camera->walk_right(-move_speed);
	}
	if (user_input->check_keyboard(DIK_W))
	{
		scene_camera->walk_front(move_speed);
	}
	if (user_input->check_keyboard(DIK_R))
	{
		scene_camera->walk_up(move_speed);
	}
	if (user_input->check_keyboard(DIK_D))
	{
		scene_camera->walk_right(move_speed);
	}
	if (user_input->check_keyboard(DIK_S))
	{
		scene_camera->walk_front(-move_speed);
	}
	if (user_input->check_keyboard(DIK_F))
	{
		scene_camera->walk_up(-move_speed);
	}
	if (user_input->check_keyboard(DIK_Q))
	{
		scene_camera->rotation_look(0.001f);
	}
	if (user_input->check_keyboard(DIK_E))
	{
		scene_camera->rotation_look(-0.001f);
	}
	if (user_input->check_mouseDown(1))
	{
		scene_camera->rotation_up(user_input->MouseMove_X() * 0.001f);
		scene_camera->rotation_right(user_input->MouseMove_Y() * 0.001f);
	}

	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;

	XMFLOAT4X4 world_matrix;
	XMFLOAT4X4 final_matrix;
	//更新城堡
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(1, 1, 1);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	geometry_resource_view *data_need;
	pancy_geometry_control_singleton::get_instance()->update_a_model_instance(ID_model_castel, world_matrix, delta_time);
	//pancy_geometry_control_singleton::get_instance()->sleep_a_model_instance(ID_model_castel);
	//更新地面
	trans_world = XMMatrixTranslation(0.0, -5.0, 0.0);
	scal_world = XMMatrixScaling(2, 1, 2);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	pancy_geometry_control_singleton::get_instance()->update_a_model_instance(ID_model_floor, world_matrix, delta_time);
	pancy_geometry_control_singleton::get_instance()->sleep_a_model_instance(ID_model_floor);
	//更新天空
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(50, 50, 50);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	pancy_geometry_control_singleton::get_instance()->update_a_model_instance(ID_model_sky, world_matrix, delta_time);
	//更新pbr测试
	trans_world = XMMatrixTranslation(0.0, 1.0, 0.0);
	scal_world = XMMatrixScaling(0.05, 0.05, 0.05);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	pancy_geometry_control_singleton::get_instance()->update_a_model_instance(ID_model_pbrtest, world_matrix, delta_time);
	//pancy_geometry_control_singleton::get_instance()->sleep_a_model_instance(ID_model_pbrtest);
}
void scene_test_square::release()
{
	pancy_geometry_control_singleton::get_instance()->release();
	tex_cubesky->Release();
}









pancy_scene_control::pancy_scene_control()
{
	sundir = XMFLOAT3(-0.972948968, 0.227391526, 0.0407850109);
	update_time = 0.0f;
	scene_now_show = -1;
	quality_reflect = 0.5f;
	picture_buf = new mesh_square(false);
	pretreat_render = new Pretreatment_gbuffer(d3d_pancy_basic_singleton::GetInstance()->get_wind_width(), d3d_pancy_basic_singleton::GetInstance()->get_wind_height(), quality_reflect);
	ssao_render = new ssao_pancy(d3d_pancy_basic_singleton::GetInstance()->get_wind_width(), d3d_pancy_basic_singleton::GetInstance()->get_wind_height());
	globel_reflect = new render_posttreatment_RTGR();
	HDR_tonemapping = new render_posttreatment_HDR(d3d_pancy_basic_singleton::GetInstance()->get_wind_width(), d3d_pancy_basic_singleton::GetInstance()->get_wind_height());
	atmosphere_texture = new atmosphere_pretreatment();
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
}
engine_basic::engine_fail_reason pancy_scene_control::init_cube_map()
{
	HRESULT hr;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~静态cubemap纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//渲染目标
	D3D11_TEXTURE2D_DESC cubeMapDesc;
	cubeMapDesc.Width = static_cast<UINT>(1024.0f * quality_reflect);
	cubeMapDesc.Height = static_cast<UINT>(1024.0f * quality_reflect);
	cubeMapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	cubeMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	cubeMapDesc.ArraySize = 6;
	cubeMapDesc.Usage = D3D11_USAGE_DEFAULT;
	cubeMapDesc.CPUAccessFlags = 0;
	cubeMapDesc.MipLevels = 1;
	cubeMapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	cubeMapDesc.SampleDesc.Count = 1;
	cubeMapDesc.SampleDesc.Quality = 0;
	//使用以上描述创建纹理
	ID3D11Texture2D *cubeMap(NULL);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&cubeMapDesc, 0, &cubeMap);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap texture error when create ssrinput tex");
		return error_message;
	}
	//创建六个rendertarget
	D3D11_RENDER_TARGET_VIEW_DESC rtv_stencil_Desc;
	rtv_stencil_Desc.Format = cubeMapDesc.Format;
	rtv_stencil_Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtv_stencil_Desc.Texture2DArray.ArraySize = 1;
	rtv_stencil_Desc.Texture2DArray.MipSlice = 0;
	for (UINT i = 0; i < 6; ++i)
	{
		rtv_stencil_Desc.Texture2DArray.FirstArraySlice = i;
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(cubeMap, &rtv_stencil_Desc, &reflect_cube_RTV[i]);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap texture RTV error when create ssrinput tex");
			return error_message;
		}
	}
	//创建一个SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc_stencil;
	srvDesc_stencil.Format = cubeMapDesc.Format;
	srvDesc_stencil.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc_stencil.TextureCube.MipLevels = 1;
	srvDesc_stencil.TextureCube.MostDetailedMip = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(cubeMap, &srvDesc_stencil, &reflect_cube_SRV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap texture SRV error when create ssrinput tex");
		return error_message;
	}
	cubeMap->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建环境贴图后台缓冲~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11Texture2D *cubeMap_back(NULL);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&cubeMapDesc, 0, &cubeMap_back);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap backbuffer error when create ssrinput tex");
		return error_message;
	}
	//创建六个rendertarget
	for (UINT i = 0; i < 6; ++i)
	{
		rtv_stencil_Desc.Texture2DArray.FirstArraySlice = i;
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(cubeMap_back, &rtv_stencil_Desc, &reflect_cube_RTV_backbuffer[i]);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap backbuffer RTV error when create ssrinput tex");
			return error_message;
		}
	}
	//创建一个SRV
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(cubeMap_back, &srvDesc_stencil, &reflect_cube_SRV_backbuffer);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap backbuffer SRV error when create ssrinput tex");
		return error_message;
	}
	cubeMap_back->Release();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建临时的深度缓冲区~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
		engine_basic::engine_fail_reason error_message(hr, "create reflect depthtexture error when create ssrinput tex");
		return error_message;
	}
	//建立深度缓冲区访问器
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateDepthStencilView(depthMap, &dsvDesc, &reflect_cube_DSV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect depthstencilview error when create ssrinput tex");
		return error_message;
	}
	depthMap->Release();

	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_scene_control::build_brdf_texture()
{
	HRESULT hr;
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 1024;
	texDesc.Height = 1024;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* ambientTex0 = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &ambientTex0);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message2(hr, "create brdf texture error");
		return error_message2;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(ambientTex0, 0, &brdf_pic);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message2(hr, "create brdf SRV error");
		return error_message2;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(ambientTex0, 0, &brdf_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message2(hr, "create brdf RTV error");
		return error_message2;
	}
	ambientTex0->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void pancy_scene_control::render_brdf_texture()
{
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(brdf_target, NULL);
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 1024.0f;
	viewPort.Height = 1024.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	engine_basic::engine_fail_reason check_error;
	auto shader_brdf = shader_control::GetInstance()->get_shader_brdf_pre(check_error);
	ID3DX11EffectTechnique *teque_need;
	shader_brdf->get_technique(&teque_need, "draw_brdf_pre");
	picture_buf->get_teque(teque_need);
	picture_buf->show_mesh();
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
}
engine_basic::engine_fail_reason pancy_scene_control::create()
{
	auto check_error = pretreat_render->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = ssao_render->basic_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_cube_map();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = globel_reflect->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = HDR_tonemapping->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = picture_buf->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = atmosphere_texture->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	light_control_singleton::GetInstance()->add_spotlight_with_shadow_map();
	int sunlight_ID;
	light_control_singleton::GetInstance()->add_sunlight_with_shadow_map(1024, 1024, 3, sunlight_ID);
	light_control_singleton::GetInstance()->set_sunlight(sunlight_ID);

	check_error = build_brdf_texture();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	render_brdf_texture();

	engine_basic::engine_fail_reason succeed;
	return succeed;
}

void pancy_scene_control::render_environment()
{
	if (delta_time_now < 0.1f)
	{
		update_time += delta_time_now;
	}
	float update_rate = 2.2f / 12.0f;
	if (update_time > update_rate)
	{
		update_time -= update_rate;
	}
	else
	{
		return;
	}
	if (pretreat_render->get_now_reflect_render_face() % 2 == 1)
	{
		engine_basic::engine_fail_reason check_error;
		auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
		D3D11_VIEWPORT shadow_map_VP;
		shadow_map_VP.TopLeftX = 0.0f;
		shadow_map_VP.TopLeftY = 0.0f;
		shadow_map_VP.Width = 1024.0f * quality_reflect;
		shadow_map_VP.Height = 1024.0f * quality_reflect;
		shadow_map_VP.MinDepth = 0.0f;
		shadow_map_VP.MaxDepth = 1.0f;
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &shadow_map_VP);

		ID3D11RenderTargetView* renderTargets[1] = { reflect_cube_RTV_backbuffer[pretreat_render->get_now_reflect_render_face() / 2] };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTargets, reflect_cube_DSV);
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(reflect_cube_RTV_backbuffer[pretreat_render->get_now_reflect_render_face() / 2], clearColor);
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearDepthStencilView(reflect_cube_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
		if (scene_now_show >= 0 && scene_now_show < scene_list.size())
		{
			//计算取景变换
			XMFLOAT3 look_vec, up_vec;
			XMFLOAT4X4 view_matrix_reflect, Proj_mat_reflect;
			look_vec = look_cube_reflect[pretreat_render->get_now_reflect_render_face() / 2];
			up_vec = up_cube_reflect[pretreat_render->get_now_reflect_render_face() / 2];

			pancy_camera::get_instance()->count_view_matrix(look_vec, up_vec, pretreat_render->get_environment_map_place(), &view_matrix_reflect);
			XMStoreFloat4x4(&Proj_mat_reflect, DirectX::XMMatrixPerspectiveFovLH(0.5f*XM_PI, 1.0f, engine_basic::perspective_message::get_instance()->get_perspective_near_plane(), engine_basic::perspective_message::get_instance()->get_perspective_far_plane()));

			shader_need->set_diffuse_light_tex(pretreat_render->get_reflect_difusse());
			shader_need->set_specular_light_tex(pretreat_render->get_reflect_specular());
			static const XMMATRIX T(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, -0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.5f, 0.5f, 0.0f, 1.0f);
			XMMATRIX V = XMLoadFloat4x4(&view_matrix_reflect);
			XMMATRIX P = XMLoadFloat4x4(&Proj_mat_reflect);
			XMFLOAT4X4 VPT;
			XMStoreFloat4x4(&VPT, V * P * T);
			shader_need->set_trans_ssao(&VPT);

			scene_list[scene_now_show]->display_environment(view_matrix_reflect, Proj_mat_reflect);
		}
	}
	if (pretreat_render->get_now_reflect_render_face() == 11)
	{
		std::swap(reflect_cube_SRV_backbuffer, reflect_cube_SRV);
		for (int i = 0; i < 6; ++i)
		{
			std::swap(reflect_cube_RTV[i], reflect_cube_RTV_backbuffer[i]);
		}
	}
	pretreat_render->upadte_reflect_render_face();
}
void pancy_scene_control::update(float delta_time)
{
	engine_basic::engine_fail_reason check_error;
	auto shader_light_deffered = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	XMFLOAT4X4 view_mat;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	shader_light_deffered->set_trans_view(&view_mat);
	delta_time_now = delta_time;
	pancy_input::GetInstance()->get_input();
	if (pancy_input::GetInstance()->check_keyboard(DIK_LCONTROL))
	{
		if (pancy_input::GetInstance()->check_mouseDown(0))
		{
			sundir.z += pancy_input::GetInstance()->MouseMove_Y() * 0.01f;
			float average = sqrt(sundir.x * sundir.x + sundir.y * sundir.y + sundir.z * sundir.z);
			sundir.x /= average;
			sundir.y /= average;
			sundir.z /= average;
			//scene_camera->rotation_up(user_input->MouseMove_X() * 0.001f);
			//scene_camera->rotation_right(user_input->MouseMove_Y() * 0.001f);
		}
		//sundir.y += 0.1;
		//sundir.x /=
	}
	if (scene_now_show >= 0 && scene_now_show < scene_list.size())
	{
		scene_list[scene_now_show]->update(delta_time);
	}
	
	
}
void pancy_scene_control::display()
{

	//设置摄像机属性
	auto scene_camera = pancy_camera::get_instance();
	XMFLOAT4X4 inv_view_mat;
	XMFLOAT3 view_pos;
	scene_camera->count_invview_matrix(&inv_view_mat);
	scene_camera->get_view_position(&view_pos);
	//设置brdf贴图
	engine_basic::engine_fail_reason check_error;
	auto shader_deffered = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	shader_deffered->set_tex_brdflist_resource(brdf_pic);
	shader_deffered->set_trans_invview(&inv_view_mat);
	shader_deffered->set_view_pos(view_pos);
	//渲染gbuffer
	pretreat_render->display();
	//渲染AO
	ssao_render->get_normaldepthmap(pretreat_render->get_gbuffer_normalspec(), pretreat_render->get_gbuffer_depth());
	ssao_render->compute_ssaomap();
	ssao_render->blur_ssaomap();
	//计算阴影
	light_control_singleton::GetInstance()->draw_shadow();
	//计算光照
	d3d_pancy_basic_singleton::GetInstance()->reset_viewport();
	pretreat_render->display_lbuffer(true);
	//正式渲染
	//d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
	//d3d_pancy_basic_singleton::GetInstance()->set_render_target(posttreatment_RTV);
	pretreat_render->set_posttreat_input_target();
	d3d_pancy_basic_singleton::GetInstance()->clear_basicrender_target();
	if (scene_now_show >= 0 && scene_now_show < scene_list.size())
	{
		scene_list[scene_now_show]->display();
		scene_list[scene_now_show]->display_nopost();
	}
	globel_reflect->draw_reflect(pretreat_render->get_environment_map_renderplace(), pretreat_render->get_posttreat_color_map(), pretreat_render->get_posttreat_mask_map(), pretreat_render->get_gbuffer_normalspec(), pretreat_render->get_gbuffer_depth(), reflect_cube_SRV, pretreat_render->get_reflect_mask_map());
	globel_reflect->draw_to_posttarget(ssao_render->get_aomap(), pretreat_render->get_gbuffer_normalspec(), pretreat_render->get_gbuffer_specrough(), brdf_pic);
	HDR_tonemapping->display(globel_reflect->get_output_tex());
	//渲染立方环境反射
	render_environment();
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	shader_need->set_diffuse_light_tex(NULL);
	shader_need->set_specular_light_tex(NULL);
	shader_need->set_ssaotex(NULL);
	ID3DX11EffectTechnique *tech_need;
	shader_need->get_technique(&tech_need, "LightTech");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech_need->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
	//交换到屏幕
	//HRESULT hr = swapchain->Present(0, 0);
	//atmosphere_texture->build_atomosphere_texture();
	
	
	atmosphere_texture->display(sundir);
	d3d_pancy_basic_singleton::GetInstance()->end_draw();
}
engine_basic::engine_fail_reason pancy_scene_control::add_a_scene(scene_root* scene_in)
{
	if (scene_in != NULL)
	{
		scene_list.push_back(scene_in);
		engine_basic::engine_fail_reason succeed;
		return succeed;
	}
	engine_basic::engine_fail_reason failed_message("add scene error for NULL ptr input");
	return failed_message;
}
engine_basic::engine_fail_reason pancy_scene_control::change_now_scene(int scene_ID)
{
	if (scene_ID >= 0 && scene_ID < scene_list.size())
	{
		scene_now_show = scene_ID;
		engine_basic::engine_fail_reason succeed;
		return succeed;
	}
	engine_basic::engine_fail_reason failed_message("change the now_showing scene error for the scen ID is not in list");
	return failed_message;
}
void pancy_scene_control::release()
{
	HDR_tonemapping->release();
	ssao_render->release();
	pretreat_render->release();
	reflect_cube_DSV->Release();
	globel_reflect->release();
	brdf_target->Release();
	brdf_pic->Release();
	picture_buf->release();
	atmosphere_texture->release();
	for (auto data = scene_list.begin(); data != scene_list.end(); ++data)
	{
		(*data._Ptr)->release();
	}
	reflect_cube_SRV->Release();
	reflect_cube_SRV_backbuffer->Release();
	for (int i = 0; i < 6; ++i)
	{
		reflect_cube_RTV[i]->Release();
		reflect_cube_RTV_backbuffer[i]->Release();
	}
	scene_list.clear();
}


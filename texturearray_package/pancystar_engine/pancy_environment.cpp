#include"pancy_environment.h"
environment_IBL_data::environment_IBL_data(float quality_texture, XMFLOAT3 sun_dir_in)
{
	quality_reflect = quality_texture;
	sun_dir = sun_dir_in;
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
engine_basic::engine_fail_reason environment_IBL_data::create()
{
	engine_basic::engine_fail_reason check_error;
	/*
	HRESULT hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), L"Texture_cube1.dds", 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &tex_cubesky);
	*/
	check_error = create_cubemap();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	environment_texture_data = new gbuffer_out_message(1024.0f * quality_reflect, 1024.0f * quality_reflect, false);
	check_error = environment_texture_data->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason environment_IBL_data::create_cubemap()
{
	HRESULT hr;
	D3D11_TEXTURE2D_DESC cubeMapDesc;
	//渲染目标
	cubeMapDesc.Width = static_cast<UINT>(1024.0f * quality_reflect);
	cubeMapDesc.Height = static_cast<UINT>(1024.0f * quality_reflect);
	cubeMapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	cubeMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	cubeMapDesc.ArraySize = 6;
	cubeMapDesc.Usage = D3D11_USAGE_DEFAULT;
	cubeMapDesc.CPUAccessFlags = 0;
	cubeMapDesc.MipLevels = 7;
	cubeMapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	cubeMapDesc.SampleDesc.Count = 1;
	cubeMapDesc.SampleDesc.Quality = 0;
	//七层mipmap纹理
	ID3D11Texture2D *cubeMap_stencil(NULL);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&cubeMapDesc, 0, &cubeMap_stencil);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil texture error when create scene_test_environment tex");
		return error_message;
	}
	//创建六个rendertarget
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc_reflect;
	rtvDesc_reflect.Format = cubeMapDesc.Format;
	rtvDesc_reflect.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc_reflect.Texture2DArray.ArraySize = 1;
	for (UINT i = 0; i < 7; ++i)
	{
		for (UINT j = 0; j < 6; ++j)
		{
			rtvDesc_reflect.Texture2DArray.MipSlice = i;
			rtvDesc_reflect.Texture2DArray.FirstArraySlice = j;
			hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(cubeMap_stencil, &rtvDesc_reflect, &RTV_cube[i * 6 + j]);
			if (FAILED(hr))
			{
				engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil texture RTV error when create scene_test_environment tex");
				return error_message;
			}
		}
	}
	//创建一个SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc_reflect;
	srvDesc_reflect.Format = cubeMapDesc.Format;
	srvDesc_reflect.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc_reflect.TextureCube.MipLevels = 7;
	srvDesc_reflect.TextureCube.MostDetailedMip = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(cubeMap_stencil, &srvDesc_reflect, &SRV_cube);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil texture SRV error when create scene_test_environment tex");
		return error_message;
	}
	cubeMap_stencil->Release();
	//原始无mipmap纹理
	cubeMapDesc.MipLevels = 1;
	ID3D11Texture2D *cubeMap_single(NULL);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&cubeMapDesc, 0, &cubeMap_single);
	rtvDesc_reflect.Format = cubeMapDesc.Format;
	rtvDesc_reflect.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc_reflect.Texture2DArray.ArraySize = 1;
	rtvDesc_reflect.Texture2DArray.MipSlice = 0;
	for (UINT i = 0; i < 6; ++i)
	{
		rtvDesc_reflect.Texture2DArray.FirstArraySlice = i;
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(cubeMap_single, &rtvDesc_reflect, &RTV_singlecube[i]);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil texture RTV error when create scene_test_environment tex");
			return error_message;
		}
	}
	//创建一个SRV
	srvDesc_reflect.Format = cubeMapDesc.Format;
	srvDesc_reflect.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc_reflect.TextureCube.MipLevels = 1;
	srvDesc_reflect.TextureCube.MostDetailedMip = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(cubeMap_single, &srvDesc_reflect, &SRV_singlecube);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil texture SRV error when create scene_test_environment tex");
		return error_message;
	}
	cubeMap_single->Release();
	//漫反射IBL贴图
	cubeMapDesc.MipLevels = 1;
	ID3D11Texture2D *cubeMap_diffuse(NULL);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&cubeMapDesc, 0, &cubeMap_diffuse);
	rtvDesc_reflect.Format = cubeMapDesc.Format;
	rtvDesc_reflect.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc_reflect.Texture2DArray.ArraySize = 1;
	rtvDesc_reflect.Texture2DArray.MipSlice = 0;
	for (UINT i = 0; i < 6; ++i)
	{
		rtvDesc_reflect.Texture2DArray.FirstArraySlice = i;
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(cubeMap_diffuse, &rtvDesc_reflect, &RTV_diffusecube[i]);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap diffuse ibl texture RTV error when create scene_test_environment tex");
			return error_message;
		}
	}
	//创建一个SRV
	srvDesc_reflect.Format = cubeMapDesc.Format;
	srvDesc_reflect.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc_reflect.TextureCube.MipLevels = 1;
	srvDesc_reflect.TextureCube.MostDetailedMip = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(cubeMap_diffuse, &srvDesc_reflect, &SRV_diffusecube);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap diffuse ibl texture SRV error when create scene_test_environment tex");
		return error_message;
	}
	cubeMap_diffuse->Release();
	//临时深度缓冲区
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
void environment_IBL_data::display_environment(
	int model_ID_sky,
	pancy_geometry_control *geometry_buffer,
	Geometry_basic *fullscreen_buffer
	)
{
	light_control_singleton::GetInstance()->update_sunlight(sun_dir);
	for (int i = 0; i < 6; ++i)
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
		//计算取景变换
		XMFLOAT3 pos_view;
		XMFLOAT3 look_vec, up_vec;
		XMFLOAT4X4 view_matrix_reflect, invview_matrix_reflect, Proj_mat_reflect;
		look_vec = look_cube_reflect[i];
		up_vec = up_cube_reflect[i];
		pancy_camera::get_instance()->get_view_position(&pos_view);
		pancy_camera::get_instance()->count_view_matrix(look_vec, up_vec, pos_view, &view_matrix_reflect);
		pancy_camera::get_instance()->count_invview_matrix(look_vec, up_vec, pos_view, &invview_matrix_reflect);
		XMStoreFloat4x4(&Proj_mat_reflect, DirectX::XMMatrixPerspectiveFovLH(0.5f*XM_PI, 1.0f, engine_basic::perspective_message::get_instance()->get_perspective_near_plane(), engine_basic::perspective_message::get_instance()->get_perspective_far_plane()));

		engine_basic::extra_perspective_message scene_perspective(1024.0f * quality_reflect, 1024.0f * quality_reflect, 0.1f, 1000.0f, 0.5f*XM_PI);

		//反投影
		float kFovY = XM_PIDIV2;
		if (i == 2 || i == 3)
		{
			kFovY = XM_PIDIV2;
		}
		//const float kFovY = engine_basic::perspective_message::get_instance()->get_perspective_angle();
		const float kTanFovY = tan(kFovY / 2.0);
		float aspect_ratio = static_cast<float>(1);
		XMFLOAT4X4 clip_matrix =
		{
			kTanFovY * aspect_ratio, 0.0, 0.0, 0.0,
			0.0, kTanFovY, 0.0, 0.0,
			0.0, 0.0, 0.0, -1.0,
			0.0, 0.0, 1.0, 1.0
		};
		auto shader_pretreat_lbuffer = shader_control::GetInstance()->get_shader_lightbuffer(check_error);
		shader_pretreat_lbuffer->set_view_from_clip(clip_matrix);

		Pretreatment_gbuffer::get_instance()->render_gbuffer(geometry_buffer, environment_texture_data->get_gbuffer(), view_matrix_reflect, &scene_perspective, false,true);
		Pretreatment_gbuffer::get_instance()->render_lbuffer(environment_texture_data->get_gbuffer(), pos_view, view_matrix_reflect, invview_matrix_reflect, &scene_perspective, false);

		ID3D11RenderTargetView* renderTargets[1] = { RTV_singlecube[i] };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTargets, reflect_cube_DSV);
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(RTV_singlecube[i], clearColor);
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearDepthStencilView(reflect_cube_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
		//渲染天空
		auto shader_test = shader_control::GetInstance()->get_shader_sky_draw(check_error);

		geometry_resource_view *data_view = NULL;
		geometry_buffer->get_a_model_type(&data_view, model_ID_sky);
		//选定绘制路径
		ID3DX11EffectTechnique *teque_need;
		shader_test->get_technique(&teque_need, "draw_sky");
		shader_test->set_trans_world(&data_view->get_matrix_list()[0]);
		//设定立方贴图
		//shader_test->set_tex_resource(tex_cubesky);
		//设定总变换
		XMFLOAT4X4 final_mat, viewproj;

		XMMATRIX proj = XMLoadFloat4x4(&Proj_mat_reflect);
		XMMATRIX world_matrix_rec = XMLoadFloat4x4(&data_view->get_matrix_list()[0]);
		XMMATRIX worldViewProj = world_matrix_rec*XMLoadFloat4x4(&view_matrix_reflect)*proj;
		XMFLOAT4X4 world_viewrec;
		XMStoreFloat4x4(&world_viewrec, worldViewProj);
		shader_test->set_trans_all(&world_viewrec);
		static const XMMATRIX T(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);
		XMFLOAT4X4 VPT;
		XMStoreFloat4x4(&VPT, XMLoadFloat4x4(&view_matrix_reflect)*proj * T);

		shader_test->set_trans_texproj(&VPT);
		data_view->get_technique(teque_need);

		data_view->draw(false);
		//还原渲染状态
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetState(NULL);
		ID3D11RenderTargetView* nullTargets[1] = { NULL };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, nullTargets, NULL);
		RTV_singlecube[i]->Release();
	}
	//视口信息
	D3D11_VIEWPORT MIPMAP_VP;
	MIPMAP_VP.TopLeftX = 0.0f;
	MIPMAP_VP.TopLeftY = 0.0f;
	MIPMAP_VP.Width = 1024.0f * quality_reflect;
	MIPMAP_VP.Height = 1024.0f * quality_reflect;
	MIPMAP_VP.MinDepth = 0.0f;
	MIPMAP_VP.MaxDepth = 1.0f;
	
	for (int i = 0; i < 7; ++i)
	{
		for (int j = 0; j < 6; ++j)
		{
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &MIPMAP_VP);
			//设置渲染路径
			ID3D11RenderTargetView* renderTargets[1] = { RTV_cube[i * 6 + j] };
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTargets, NULL);
			float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(RTV_cube[i * 6 + j], clearColor);
			//正式渲染
			engine_basic::engine_fail_reason check_error;
			auto shader_IBL_spec = shader_control::GetInstance()->get_shader_IBL_specular(check_error);
			shader_IBL_spec->set_input_message(XMFLOAT2(0, 0), j, i);
			shader_IBL_spec->set_input_CubeTex(SRV_singlecube);
			ID3DX11EffectTechnique *teque_need;
			shader_IBL_spec->get_technique(&teque_need, "IBL_Specular");
			fullscreen_buffer->get_teque(teque_need);
			fullscreen_buffer->show_mesh();
			//释放资源
			ID3D11RenderTargetView* nullrenderTargets[1] = { NULL };
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, nullrenderTargets, NULL);
			RTV_cube[i * 6 + j]->Release();
		}
		MIPMAP_VP.Width /= 2;
		MIPMAP_VP.Height /= 2;
	}
	for (int i = 0; i < 6; ++i)
	{
		D3D11_VIEWPORT Diffuse_VP;
		Diffuse_VP.TopLeftX = 0.0f;
		Diffuse_VP.TopLeftY = 0.0f;
		Diffuse_VP.Width = 1024.0f * quality_reflect;
		Diffuse_VP.Height = 1024.0f * quality_reflect;
		Diffuse_VP.MinDepth = 0.0f;
		Diffuse_VP.MaxDepth = 1.0f;
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &Diffuse_VP);
		//设置渲染路径
		ID3D11RenderTargetView* renderTargets[1] = { RTV_diffusecube[i] };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTargets, NULL);
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(RTV_diffusecube[i], clearColor);
		//正式渲染
		engine_basic::engine_fail_reason check_error;
		auto shader_IBL_spec = shader_control::GetInstance()->get_shader_IBL_specular(check_error);
		shader_IBL_spec->set_input_message(XMFLOAT2(0, 0), i, 0);
		shader_IBL_spec->set_input_CubeTex(SRV_singlecube);
		ID3DX11EffectTechnique *teque_need;
		shader_IBL_spec->get_technique(&teque_need, "IBL_Diffuse");
		fullscreen_buffer->get_teque(teque_need);
		fullscreen_buffer->show_mesh();
		//释放资源
		ID3D11RenderTargetView* nullrenderTargets[1] = { NULL };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, nullrenderTargets, NULL);
		RTV_diffusecube[i]->Release();
	}
	environment_texture_data->release();
	SRV_singlecube->Release();
	reflect_cube_DSV->Release();
}
void environment_IBL_data::release()
{
	//environment_texture_data->release();
	//tex_cubesky->Release();
	//SRV_singlecube->Release();
	SRV_cube->Release();
	SRV_diffusecube->Release();
}

environment_IBL_control::environment_IBL_control(int divide_num_mid_in, int divide_num_up_in,float quality_sky_in)
{
	divide_num_mid = divide_num_mid_in;
	divide_num_up = divide_num_up_in;
	environment_num = divide_num_mid * 2 + divide_num_up;
	quality_sky = quality_sky_in;
	/*
	time_range.x = 0 - sun_render_range;
	time_range.y = XM_PI + sun_render_range;
	environment_num = divide_num;
	int sun_up_num = environment_num / 6;
	int sun_mid_num = (environment_num - sun_up_num) / 2;
	for (int i = 0; i < sun_mid_num; ++i)
	{
		float delta_time = (0.25 - (-0.25f)) / static_cast<float>(sun_mid_num);
		float now_time = static_cast<float>(i) * delta_time - 0.25f;
		time_list.push_back(now_time);
	}
	for (int i = 0; i < sun_up_num; ++i)
	{
		float delta_time = ((XM_PI-0.25f) - 0.25f) / static_cast<float>(sun_up_num);
		float now_time = static_cast<float>(i) * delta_time + 0.25f;
		time_list.push_back(now_time);
	}
	for (int i = 0; i < sun_mid_num; ++i)
	{
		float delta_time = ((XM_PI + 0.25f) - (XM_PI - 0.25f)) / static_cast<float>(sun_mid_num);
		float now_time = static_cast<float>(i) * delta_time + (XM_PI - 0.25f);
		time_list.push_back(now_time);
	}
	*/
	geometry_sky = new pancy_geometry_control();
}
engine_basic::engine_fail_reason environment_IBL_control::create()
{
	engine_basic::engine_fail_reason check_error;
	//加载天空
	check_error = geometry_sky->load_a_model_type("ballmodel\\outmodel.pancymesh", "ballmodel\\outmodel.pancymat", false, model_ID_sky);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	XMFLOAT4X4 mat_world;
	XMStoreFloat4x4(&mat_world, XMMatrixIdentity());
	check_error = geometry_sky->add_a_model_instance(model_ID_sky, mat_world, ID_model_sky);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	geometry_resource_view *data_view = NULL;
	geometry_sky->get_a_model_type(&data_view, model_ID_sky);
	data_view->set_cull_front();
	//全屏幕平面
	fullscreen_buffer = new mesh_square(false);
	check_error = fullscreen_buffer->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	for (int i = 0; i < environment_num; ++i)
	{
		XMFLOAT3 sun_dir;
		//float time_length = time_range.y - time_range.x;
		//float vector_delta = time_length / static_cast<float>(environment_num);
		//float vector_angle = time_range.x + static_cast<float>(i) * vector_delta;
		float vector_angle = Transform_IntTime_ToFloat(i);
		fmod(vector_angle, XM_2PI);
		sun_dir.x = cos(vector_angle);
		sun_dir.y = -sin(vector_angle);
		sun_dir.z = 0;

		check_error = add_an_IBL_data(i, quality_sky, sun_dir);
		if (!check_error.check_if_failed())
		{
			return check_error;
		}
	}
	display_an_IBL_data();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason environment_IBL_control::add_an_IBL_data(int time, float quality, XMFLOAT3 sun_dir)
{
	environment_IBL_data *new_IBL_data = new environment_IBL_data(quality, sun_dir);
	engine_basic::engine_fail_reason check_error = new_IBL_data->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	std::pair<int, environment_IBL_data> data_need(time, *new_IBL_data);
	auto check_iferror = IBL_list.insert(data_need);
	if (!check_iferror.second)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "insert IBL data to list error");
		return error_message;
	}
	delete new_IBL_data;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
bool environment_IBL_control::display_an_IBL_data()
{
	for (auto data_now = IBL_list.begin(); data_now != IBL_list.end(); ++data_now)
	{
		data_now->second.display_environment(model_ID_sky, geometry_sky, fullscreen_buffer);
	}
	/*
	auto data_now = IBL_list.find(display_count);
	if (display_count >= environment_num || data_now == IBL_list.end())
	{
		return false;
	}
	data_now->second.display_environment(model_ID_sky, geometry_sky, fullscreen_buffer);
	display_count += 1;
	*/
	return true;
}
environment_IBL_data* environment_IBL_control::get_IBL_data_by_time(float time)
{
	int IBL_data_ID = Transform_FloatTime_ToInt(time);
	if (IBL_data_ID == -1) 
	{
		return NULL;
	}
	auto data_now = IBL_list.find(IBL_data_ID);
	if (IBL_data_ID >= environment_num || data_now == IBL_list.end())
	{
		return NULL;
	}
	return &data_now->second;
}
void environment_IBL_control::update_sky_data(float delta_time)
{
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;

	XMFLOAT4X4 world_matrix;
	XMFLOAT4X4 final_matrix;
	//更新天空
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(50, 50, 50);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	geometry_sky->update_a_model_instance(ID_model_sky, world_matrix, delta_time);
}

int   environment_IBL_control::Transform_FloatTime_ToInt(float float_time)
{
	int int_time = -1;
	float delta_mid = 0.5f / static_cast<float>(divide_num_mid);
	float delta_up = (XM_PI - 0.5f) / static_cast<float>(divide_num_up);
	if (float_time > -0.249f)
	{
		if (float_time < 0.249f)
		{
			float now_int_num = (float_time + 0.25f) / delta_mid;
			int_time = static_cast<int>(now_int_num + 0.5f);
		}
		else if (float_time < (XM_PI - 0.251f))
		{
			float now_int_num = (float_time - 0.25f) / delta_up;
			int_time = static_cast<int>(now_int_num + 0.5f) + divide_num_mid;
		}
		else if (float_time < (XM_PI + 0.251f))
		{
			float now_int_num = (float_time - XM_PI + 0.25f) / delta_mid;
			int_time = static_cast<int>(now_int_num + 0.5f) + divide_num_mid + divide_num_up;
		}
	}
	return int_time;
}
float environment_IBL_control::Transform_IntTime_ToFloat(int int_time)
{
	float float_time = -XM_PIDIV2;
	float delta_mid = 0.5f / static_cast<float>(divide_num_mid);
	float delta_up = (XM_PI - 0.5f) / static_cast<float>(divide_num_up);
	if (int_time >= 0)
	{
		if (int_time < divide_num_mid)
		{
			float_time = -0.25f + delta_mid * static_cast<float>(int_time);
		}
		else if (int_time < (divide_num_mid + divide_num_up))
		{
			float_time = 0.25f + delta_up * static_cast<float>(int_time - divide_num_mid);
		}
		else if (int_time <= (divide_num_mid + divide_num_up + divide_num_mid))
		{
			float_time = XM_PI - 0.25f + delta_mid * static_cast<float>(int_time - divide_num_mid - divide_num_up);
		}
	}
	return float_time;
}
void environment_IBL_control::release()
{
	geometry_sky->release();
	fullscreen_buffer->release();
	for (auto data = IBL_list.begin(); data != IBL_list.end(); ++data)
	{
		data->second.release();
	}
	IBL_list.clear();
}

#include"pancy_scene_design.h"
scene_root::scene_root()
{
	time_game = 0.0f;
}

scene_test_square::scene_test_square()
{
}
engine_basic::engine_fail_reason scene_test_square::create()
{
	XMFLOAT4X4 mat_world;
	XMStoreFloat4x4(&mat_world, XMMatrixIdentity());
	engine_basic::engine_fail_reason check_error = pancy_geometry_control_singleton::get_instance()->load_a_model_type("castelmodel\\outmodel.pancymesh", "castelmodel\\outmodel.pancymat", model_ID_castel);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = pancy_geometry_control_singleton::get_instance()->add_a_model_instance(model_ID_castel, mat_world, ID_model_castel);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	check_error = pancy_geometry_control_singleton::get_instance()->load_a_model_type("squaremodel\\outmodel.pancymesh", "squaremodel\\outmodel.pancymat", model_ID_floor);
	check_error = pancy_geometry_control_singleton::get_instance()->add_a_model_instance(model_ID_floor, mat_world, ID_model_floor);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}


	check_error = pancy_geometry_control_singleton::get_instance()->load_a_model_type("ballmodel\\outmodel.pancymesh", "ballmodel\\outmodel.pancymat", model_ID_sky);
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


	check_error = pancy_geometry_control_singleton::get_instance()->load_a_model_type("ballmodel\\outmodel.pancymesh", "ballmodel\\outmodel.pancymat", model_ID_ball);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	for (int i = 0; i < 24; ++i)
	{
		check_error = pancy_geometry_control_singleton::get_instance()->add_a_model_instance(model_ID_ball, mat_world, ID_model_ball[i]);
		if (!check_error.check_if_failed())
		{
			return check_error;
		}
	}
	HRESULT hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), L"Texture_cube.dds", 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &tex_cubesky);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void scene_test_square::display()
{
	show_ball_single();
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

	data_view->draw();
	/*
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_virtual_light(check_error);

	geometry_resource_view *data_view = NULL;
	pancy_geometry_control_singleton::get_instance()->get_a_model_type(&data_view, model_ID_castel);
	XMFLOAT4X4 view_mat,final_mat;
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

	data_view->draw();
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
void scene_test_square::show_ball_single()
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);

	geometry_resource_view *data_view = NULL;
	pancy_geometry_control_singleton::get_instance()->get_a_model_type(&data_view, model_ID_ball);
	XMFLOAT4X4 view_mat, final_mat, viewproj;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	XMMATRIX rec_world = XMLoadFloat4x4(&data_view->get_matrix_list()[0]) * XMLoadFloat4x4(&view_mat) * proj;
	XMStoreFloat4x4(&final_mat, rec_world);
	XMStoreFloat4x4(&viewproj, XMLoadFloat4x4(&view_mat) * proj);

	shader_need->set_trans_world(&data_view->get_matrix_list()[0]);

	XMFLOAT4X4 world_mat_ball[100];
	int count = 0;
	auto matrix_list = data_view->get_matrix_list();
	for (auto mat_data = matrix_list.begin(); mat_data != matrix_list.end(); mat_data++)
	{
		world_mat_ball[count++] = *mat_data._Ptr;
	}
	shader_need->set_world_matrix_array(world_mat_ball, count);

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
	test_Mt.reflect = rec_ambient2;
	shader_need->set_material(test_Mt);

	ID3DX11EffectTechnique *teque_need;
	shader_need->get_technique(&teque_need, "LightTech_instance");

	data_view->get_technique(teque_need);

	data_view->draw();
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

	data_view->draw();
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetState(NULL);
}
void scene_test_square::update(float delta_time)
{
	float move_speed = 0.025f;
	XMMATRIX view;
	auto user_input = pancy_input::GetInstance();
	auto scene_camera = pancy_camera::get_instance();
	user_input->get_input();
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
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(1, 1, 1);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	geometry_resource_view *data_need;
	pancy_geometry_control_singleton::get_instance()->update_a_model_instance(ID_model_castel, world_matrix, delta_time);
	trans_world = XMMatrixTranslation(0.0, -5.0, 0.0);
	scal_world = XMMatrixScaling(2, 1, 2);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	pancy_geometry_control_singleton::get_instance()->update_a_model_instance(ID_model_floor, world_matrix, delta_time);
	pancy_geometry_control_singleton::get_instance()->sleep_a_model_instance(ID_model_floor);

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 6; ++j) 
		{
			float count_x = i*2.5f;
			float count_z = j*2.5f;
			trans_world = XMMatrixTranslation(-5.0f + count_x, 1.0, count_z);
			scal_world = XMMatrixScaling(0.05f, 0.05f, 0.05f);
			XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
			pancy_geometry_control_singleton::get_instance()->update_a_model_instance(ID_model_ball[i*6+j], world_matrix, delta_time);
		}
	}



	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(50, 50, 50);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	pancy_geometry_control_singleton::get_instance()->update_a_model_instance(ID_model_sky, world_matrix, delta_time);

}
void scene_test_square::release()
{
	pancy_geometry_control_singleton::get_instance()->release();
	tex_cubesky->Release();
}









pancy_scene_control::pancy_scene_control()
{
	update_time = 0.0f;
	scene_now_show = -1;
	quality_reflect = 0.5f;
	pretreat_render = new Pretreatment_gbuffer(d3d_pancy_basic_singleton::GetInstance()->get_wind_width(), d3d_pancy_basic_singleton::GetInstance()->get_wind_height(), quality_reflect);
	ssao_render = new ssao_pancy(d3d_pancy_basic_singleton::GetInstance()->get_wind_width(), d3d_pancy_basic_singleton::GetInstance()->get_wind_height());
	globel_reflect = new render_posttreatment_RTGR();
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
	light_control_singleton::GetInstance()->add_spotlight_with_shadow_map();
	int sunlight_ID;
	light_control_singleton::GetInstance()->add_sunlight_with_shadow_map(1024, 1024, 3, sunlight_ID);
	light_control_singleton::GetInstance()->set_sunlight(sunlight_ID);




	engine_basic::engine_fail_reason succeed;
	return succeed;
}

void pancy_scene_control::render_environment()
{
	if (delta_time_now < 0.1f)
	{
		update_time += delta_time_now;
	}
	float update_rate = 4.0f / 12.0f;
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
	delta_time_now = delta_time;
	if (scene_now_show >= 0 && scene_now_show < scene_list.size())
	{
		scene_list[scene_now_show]->update(delta_time);
	}
}
void pancy_scene_control::display()
{
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
	//渲染立方环境反射
	render_environment();
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
	engine_basic::engine_fail_reason check_error;
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
	ssao_render->release();
	pretreat_render->release();
	reflect_cube_DSV->Release();
	globel_reflect->release();
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


#include"pancy_scene_design.h"
#include<math.h>

scene_root::scene_root()
{
	time_game = 0.0f;
	geometry_buffer = new pancy_geometry_control();
}
engine_basic::engine_fail_reason scene_root::create_basic()
{
	engine_basic::engine_fail_reason check_error;
	//gbuffer����
	gbuffer_texture_data = new gbuffer_out_message(d3d_pancy_basic_singleton::GetInstance()->get_wind_width(), d3d_pancy_basic_singleton::GetInstance()->get_wind_height(), true);
	check_error = gbuffer_texture_data->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	//������ȾĿ��
	post_buffer_target = new postRTGR_out_message(d3d_pancy_basic_singleton::GetInstance()->get_wind_width(), d3d_pancy_basic_singleton::GetInstance()->get_wind_height());
	check_error = post_buffer_target->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	//HDR��ȾĿ��
	HDR_buffer_target = new postHDR_out_message(d3d_pancy_basic_singleton::GetInstance()->get_wind_width(), d3d_pancy_basic_singleton::GetInstance()->get_wind_height());
	check_error = HDR_buffer_target->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	//����ϵͳ
	physic_scene = new pancy_physx_scene();
	check_error = physic_scene->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason scene_root::release_basic()
{
	geometry_buffer->release();
	HDR_buffer_target->release();
	gbuffer_texture_data->release();
	post_buffer_target->release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

real_time_environment::real_time_environment(float quality_environment_in)
{
	now_render_face = 0;
	gbuffer_render_turn = true;
	quality_environment = quality_environment_in;
	environment_VP.TopLeftX = 0.0f;
	environment_VP.TopLeftY = 0.0f;
	environment_VP.Width = 1024.0f * quality_environment_in;
	environment_VP.Height = 1024.0f * quality_environment_in;
	environment_VP.MinDepth = 0.0f;
	environment_VP.MaxDepth = 1.0f;
	gbuffer_texture_data = new gbuffer_out_message(quality_environment * 1024.0f, quality_environment * 1024.0f, false);
	scene_perspective = new engine_basic::extra_perspective_message(1024.0f * quality_environment, 1024.0f * quality_environment, 0.1f, 1000.0f, DirectX::XM_PIDIV2);
	front_scene_center = XMFLOAT3(0, 0, 0);
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
engine_basic::engine_fail_reason real_time_environment::init_cube_texture(DXGI_FORMAT tex_format, ID3D11ShaderResourceView **SRV_in, ID3D11RenderTargetView *RTV_in[6], string texture_name)
{
	D3D11_TEXTURE2D_DESC cubeMapDesc;
	//��ȾĿ��
	cubeMapDesc.Width = static_cast<UINT>(1024.0f * quality_environment);
	cubeMapDesc.Height = static_cast<UINT>(1024.0f * quality_environment);
	cubeMapDesc.Format = tex_format;
	cubeMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	cubeMapDesc.ArraySize = 6;
	cubeMapDesc.Usage = D3D11_USAGE_DEFAULT;
	cubeMapDesc.CPUAccessFlags = 0;
	cubeMapDesc.MipLevels = 1;
	cubeMapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	cubeMapDesc.SampleDesc.Count = 1;
	cubeMapDesc.SampleDesc.Quality = 0;
	//ʹ������������������
	ID3D11Texture2D *cubeMap_data(NULL);
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&cubeMapDesc, 0, &cubeMap_data);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create cube " + texture_name + " texdata error when create real_time_environment");
		return error_message;
	}

	//��������rendertarget
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc_reflect;
	rtvDesc_reflect.Format = cubeMapDesc.Format;
	rtvDesc_reflect.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc_reflect.Texture2DArray.ArraySize = 1;
	rtvDesc_reflect.Texture2DArray.MipSlice = 0;
	for (UINT i = 0; i < 6; ++i)
	{
		rtvDesc_reflect.Texture2DArray.FirstArraySlice = i;
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(cubeMap_data, &rtvDesc_reflect, &RTV_in[i]);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil texture RTV error when create ssrinput tex");
			return error_message;
		}
	}
	//����һ��SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc_reflect;
	srvDesc_reflect.Format = cubeMapDesc.Format;
	srvDesc_reflect.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc_reflect.TextureCube.MipLevels = 1;
	srvDesc_reflect.TextureCube.MostDetailedMip = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(cubeMap_data, &srvDesc_reflect, SRV_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil texture SRV error when create ssrinput tex");
		return error_message;
	}
	cubeMap_data->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason real_time_environment::init_texture()
{
	engine_basic::engine_fail_reason check_error;
	check_error = init_cube_texture(DXGI_FORMAT_R32G32_FLOAT, &cube_depthstencil_SRV, cube_depthstencil_RTV, "depthstencil_texture_cube");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	init_cube_texture(DXGI_FORMAT_R32G32_FLOAT, &cube_depthstencil_backSRV, cube_depthstencil_backRTV, "depthstencil_backtexture_cube");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	init_cube_texture(DXGI_FORMAT_R16G16B16A16_FLOAT, &cube_rendercolor_SRV, cube_rendercolor_RTV, "render_texture_cube");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	init_cube_texture(DXGI_FORMAT_R16G16B16A16_FLOAT, &cube_rendercolor_backSRV, cube_rendercolor_backRTV, "render_backtexture_cube");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	//������ʱ��Ȼ�����
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = static_cast<int>(1024.0f * quality_environment);
	texDesc.Height = static_cast<int>(1024.0f * quality_environment);
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	//����������Դ
	ID3D11Texture2D* depthMap = 0;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &depthMap);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect depthtexture error when create ssrinput tex");
		return error_message;
	}
	//������Ȼ�����������
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
engine_basic::engine_fail_reason real_time_environment::create()
{
	engine_basic::engine_fail_reason check_error;
	fullscreen_buffer = new mesh_square(false);
	check_error = fullscreen_buffer->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = gbuffer_texture_data->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = init_texture();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	pancy_camera::get_instance()->get_view_position(&center_position);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void real_time_environment::get_ViewMatrix(XMFLOAT4X4 *view_matrix, XMFLOAT4X4 *invview_matrix = NULL)
{
	XMFLOAT3 look_vec, up_vec;
	//XMFLOAT4X4 view_matrix_reflect, inv_view_matrix_reflect, Proj_mat_reflect;
	look_vec = look_cube_reflect[now_render_face];
	up_vec = up_cube_reflect[now_render_face];
	pancy_camera::get_instance()->count_view_matrix(look_vec, up_vec, center_position, view_matrix);
	if (invview_matrix != NULL)
	{
		pancy_camera::get_instance()->count_invview_matrix(look_vec, up_vec, center_position, invview_matrix);
	}
}
void real_time_environment::display_backbuffer(scene_root *environment_scene)
{
	//����ȡ���任
	XMFLOAT4X4 view_matrix_reflect, inv_view_matrix_reflect;
	get_ViewMatrix(&view_matrix_reflect, &inv_view_matrix_reflect);

	Pretreatment_gbuffer::get_instance()->render_gbuffer(environment_scene->get_geometry_buffer(), gbuffer_texture_data->get_gbuffer(), view_matrix_reflect, scene_perspective, true,false);
	Pretreatment_gbuffer::get_instance()->render_gbuffer(environment_scene->get_geometry_buffer(), gbuffer_texture_data->get_gbuffer(), view_matrix_reflect, scene_perspective, true, true);
	Pretreatment_gbuffer::get_instance()->render_lbuffer(gbuffer_texture_data->get_gbuffer(), center_position, view_matrix_reflect, inv_view_matrix_reflect, scene_perspective, true);

	engine_basic::engine_fail_reason check_error;
	//��Ⱦ��ȵ�����ģ��������ͼ
	auto shader_save_depth = shader_control::GetInstance()->get_shader_reflect_savedepth(check_error);
	shader_save_depth->set_cube_count(XMFLOAT3(now_render_face, 0.0f, 0.0f));
	shader_save_depth->set_depthtex_input(gbuffer_texture_data->get_gbuffer()->depthmap_single_tex);

	d3d_pancy_basic_singleton::GetInstance()->set_render_target(cube_depthstencil_backRTV[now_render_face], NULL);
	float clearColor[] = { 0.0f, 0.0f, -1.0f, 1e5f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(cube_depthstencil_backRTV[now_render_face], clearColor);

	ID3DX11EffectTechnique *tech_need;
	shader_save_depth->get_technique(&tech_need, "resolove_alpha");
	fullscreen_buffer->get_teque(tech_need);
	fullscreen_buffer->show_mesh();

	D3DX11_TECHNIQUE_DESC techDesc;
	shader_save_depth->set_depthtex_input(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, NULL_target, NULL);
	tech_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech_need->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
}
void real_time_environment::display_environment(scene_root *environment_scene)
{
	//���ù�����ͼ
	engine_basic::engine_fail_reason check_error;
	auto shader_deffered = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	shader_deffered->set_diffuse_light_tex(gbuffer_texture_data->get_gbuffer()->gbuffer_diffuse_tex);
	shader_deffered->set_specular_light_tex(gbuffer_texture_data->get_gbuffer()->gbuffer_specular_tex);
	shader_deffered->set_normal_tex(gbuffer_texture_data->get_gbuffer()->normalspec_tex);
	shader_deffered->set_tex_specroughness_resource(gbuffer_texture_data->get_gbuffer()->specroughness_tex);

	XMFLOAT4X4 view_matrix_reflect, inv_view_matrix_reflect;
	get_ViewMatrix(&view_matrix_reflect);

	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &environment_VP);

	ID3D11RenderTargetView* renderTargets[1] = { cube_rendercolor_backRTV[now_render_face] };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTargets, reflect_cube_DSV);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(cube_rendercolor_backRTV[now_render_face], clearColor);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearDepthStencilView(reflect_cube_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	static const XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);
	XMMATRIX V = XMLoadFloat4x4(&view_matrix_reflect);
	XMMATRIX P = XMLoadFloat4x4(&scene_perspective->get_proj_matrix());
	XMFLOAT4X4 VPT;
	XMStoreFloat4x4(&VPT, V * P * T);

	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	shader_need->set_trans_ssao(&VPT);
	auto shader_sky = shader_control::GetInstance()->get_shader_sky_draw(check_error);
	shader_sky->set_trans_texproj(&VPT);

	environment_scene->display_environment(view_matrix_reflect, scene_perspective->get_proj_matrix());
}
void real_time_environment::display_a_turn(scene_root *environment_scene)
{
	if (gbuffer_render_turn)
	{
		display_backbuffer(environment_scene);
	}
	else
	{
		display_environment(environment_scene);
	}
	//�л���Ⱦģʽ(��Ⱦgbuffer/color)
	gbuffer_render_turn = !gbuffer_render_turn;
	if (gbuffer_render_turn)
	{
		//�л���Ⱦ��
		now_render_face = (now_render_face + 1) % 6;
	}
	if (now_render_face == 0 && gbuffer_render_turn)
	{
		//�л���ȼ�¼����������
		std::swap(cube_depthstencil_SRV, cube_depthstencil_backSRV);
		for (int i = 0; i < 6; ++i)
		{
			std::swap(cube_depthstencil_RTV[i], cube_depthstencil_backRTV[i]);
		}
		//�л���ɫ��¼����������
		std::swap(cube_rendercolor_SRV, cube_rendercolor_backSRV);
		for (int i = 0; i < 6; ++i)
		{
			std::swap(cube_rendercolor_RTV[i], cube_rendercolor_backRTV[i]);
		}
		//�����������ĵ�
		front_scene_center = center_position;
		pancy_camera::get_instance()->get_view_position(&center_position);
	}
}
void real_time_environment::release()
{
	fullscreen_buffer->release();
	gbuffer_texture_data->release();
	cube_depthstencil_SRV->Release();
	cube_depthstencil_backSRV->Release();
	cube_rendercolor_SRV->Release();
	cube_rendercolor_backSRV->Release();
	reflect_cube_DSV->Release();
	for (int i = 0; i < 6; ++i)
	{
		cube_depthstencil_RTV[i]->Release();
		cube_depthstencil_backRTV[i]->Release();
		cube_rendercolor_RTV[i]->Release();
		cube_rendercolor_backRTV[i]->Release();
	}
	delete scene_perspective;
}

scene_test_square::scene_test_square()
{
	simulate_ocean = new OceanSimulator();
	render_ocean = new FFT_ocean();
}
engine_basic::engine_fail_reason scene_test_square::create()
{
	engine_basic::engine_fail_reason check_error;
	//������������Դ
	check_error = create_basic();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	XMFLOAT4X4 mat_world;
	XMStoreFloat4x4(&mat_world, XMMatrixIdentity());
	check_error = geometry_buffer->load_a_model_type("castelmodel\\outmodel.pancymesh", "castelmodel\\outmodel.pancymat", false, model_ID_castel);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->add_a_model_instance(model_ID_castel, mat_world, ID_model_castel);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	check_error = geometry_buffer->load_a_model_type("squaremodel\\outmodel.pancymesh", "squaremodel\\outmodel.pancymat", false, model_ID_floor);
	check_error = geometry_buffer->add_a_model_instance(model_ID_floor, mat_world, ID_model_floor);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}


	check_error = geometry_buffer->load_a_model_type("ballmodel\\outmodel.pancymesh", "ballmodel\\outmodel.pancymat", false, model_ID_sky);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->add_a_model_instance(model_ID_sky, mat_world, ID_model_sky);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	geometry_resource_view *data_view = NULL;
	geometry_buffer->get_a_model_type(&data_view, model_ID_sky);
	data_view->set_cull_front();

	check_error = geometry_buffer->load_a_model_type("testball\\testball.pancymesh", "testball\\testball.pancymat", true, model_ID_pbrtest);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->add_a_model_instance(model_ID_pbrtest, mat_world, ID_model_pbrtest);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	// Ocean object
	OceanParameter ocean_param;

	// The size of displacement map. In this sample, it's fixed to 512.
	ocean_param.dmap_dim = 512;
	// The side length (world space) of square patch
	ocean_param.patch_length = 2000.0f;
	// Adjust this parameter to control the simulation speed
	ocean_param.time_scale = 0.8f;
	// A scale to control the amplitude. Not the world space height
	ocean_param.wave_amplitude = 0.35f;
	// 2D wind direction. No need to be normalized
	ocean_param.wind_dir = XMFLOAT2(0.8f, 0.6f);
	// The bigger the wind speed, the larger scale of wave crest.
	// But the wave scale can be no larger than patch_length
	ocean_param.wind_speed = 600.0f;
	// Damp out the components opposite to wind direction.
	// The smaller the value, the higher wind dependency
	ocean_param.wind_dependency = 0.07f;
	// Control the scale of horizontal movement. Higher value creates
	// pointy crests.
	ocean_param.choppy_scale = 1.3f;
	simulate_ocean->create(ocean_param);
	// Update the simulation for the first time.
	simulate_ocean->updateDisplacementMap(10);
	DXGI_SURFACE_DESC desk_need;
	desk_need.Width = d3d_pancy_basic_singleton::GetInstance()->get_wind_width();
	desk_need.Height = d3d_pancy_basic_singleton::GetInstance()->get_wind_height();
	desk_need.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desk_need.SampleDesc.Count = 4;
	desk_need.SampleDesc.Quality = 0;
	check_error = render_ocean->create(ocean_param, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), &desk_need);
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


	ID3D11ShaderResourceView* tex_displacement = simulate_ocean->getD3D11DisplacementMap();
	ID3D11ShaderResourceView* tex_gradient = simulate_ocean->getD3D11GradientMap();
	render_ocean->renderdraw(tex_displacement, tex_gradient, (float)0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	//show_floor_single();
}
void scene_test_square::display_environment(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix)
{
	show_model_single("LightTech_withoutao", &view_matrix, &proj_matrix);
	show_sky_single(view_matrix, &proj_matrix);
}
void scene_test_square::show_model_single(string tech_name, XMFLOAT4X4 *view_matrix, XMFLOAT4X4 *proj_matrix)
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);

	geometry_resource_view *data_view = NULL;
	geometry_buffer->get_a_model_type(&data_view, model_ID_castel);
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
	geometry_buffer->get_a_model_type(&data_view, model_ID_floor);
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
	geometry_buffer->get_a_model_type(&data_view, model_ID_sky);
	//ѡ������·��
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_sky");
	shader_test->set_trans_world(&data_view->get_matrix_list()[0]);
	//�趨������ͼ
	shader_test->set_tex_resource(tex_cubesky);
	//�趨�ܱ任
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
void scene_test_square::show_sky_single(XMFLOAT4X4 view_matrix, XMFLOAT4X4 *proj_matrix)
{
	engine_basic::engine_fail_reason check_error;
	auto shader_test = shader_control::GetInstance()->get_shader_sky_draw(check_error);

	geometry_resource_view *data_view = NULL;
	geometry_buffer->get_a_model_type(&data_view, model_ID_sky);
	//ѡ������·��
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_sky");
	shader_test->set_trans_world(&data_view->get_matrix_list()[0]);
	//�趨������ͼ
	shader_test->set_tex_resource(tex_cubesky);
	//�趨�ܱ任
	XMFLOAT4X4 view_mat, final_mat, viewproj;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	view_mat = view_matrix;
	XMMATRIX proj = XMLoadFloat4x4(proj_matrix);
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
	geometry_buffer->get_a_model_type(&data_view, model_ID_pbrtest);
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
	all_time_need += delta_time;
	simulate_ocean->updateDisplacementMap(all_time_need);
	float move_speed = 0.225f;
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
	//���³Ǳ�
	trans_world = XMMatrixTranslation(0.0, 5.0, 0.0);
	scal_world = XMMatrixScaling(1, 1, 1);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	geometry_resource_view *data_need;
	geometry_buffer->update_a_model_instance(ID_model_castel, world_matrix, delta_time);
	//pancy_geometry_control_singleton::get_instance()->sleep_a_model_instance(ID_model_castel);
	//���µ���
	trans_world = XMMatrixTranslation(0.0, -5.0, 0.0);
	scal_world = XMMatrixScaling(2, 1, 2);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	geometry_buffer->update_a_model_instance(ID_model_floor, world_matrix, delta_time);
	geometry_buffer->sleep_a_model_instance(ID_model_floor);
	//�������
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(200, 200, 200);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	geometry_buffer->update_a_model_instance(ID_model_sky, world_matrix, delta_time);
	//����pbr����
	trans_world = XMMatrixTranslation(0.0, 6.0, 0.0);
	scal_world = XMMatrixScaling(0.05, 0.05, 0.05);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	geometry_buffer->update_a_model_instance(ID_model_pbrtest, world_matrix, delta_time);
	//pancy_geometry_control_singleton::get_instance()->sleep_a_model_instance(ID_model_pbrtest);
}
void scene_test_square::release()
{
	//�ͷŻ�����Դ
	release_basic();
	//�Ƿ�������Դ
	geometry_buffer->release();
	tex_cubesky->Release();
	render_ocean->release();
	simulate_ocean->release();

}



scene_test_environment::scene_test_environment()
{
	test_model = new model_reader_PancySkinMesh("yuriskin\\yuri.pancyskinmesh", "yuriskin\\yuri.pancymat", "yuriskin\\yuri.pancyskin");
	/*
	quality_reflect = 0.5f;
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
	*/
}
engine_basic::engine_fail_reason scene_test_environment::create()
{
	engine_basic::engine_fail_reason check_error;
	//������������Դ
	check_error = create_basic();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	//������Դ
	test_IBL = new environment_IBL_control(20, 4, 0.25f);
	check_error = test_IBL->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	/*
	environment_texture_data = new gbuffer_out_message(1024.0f * quality_reflect, 1024.0f * quality_reflect, false);
	check_error = environment_texture_data->create();
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
	*/
	check_error = test_model->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	model_reader_PancySkinMesh *pt = dynamic_cast<model_reader_PancySkinMesh*>(test_model);
	pt->load_animation_list("yuriskin\\yuri.pancyanimation", animation_id);
	pt->set_animation_byindex(0);
	pt->update_animation(1 / 100.0f);
	XMFLOAT4X4 mat_world;
	XMStoreFloat4x4(&mat_world, XMMatrixIdentity());
	//���سǱ�
	/*
	check_error = geometry_buffer->load_a_model_type("castelmodel\\outmodel.pancymesh", "castelmodel\\outmodel.pancymat", false, model_ID_castel);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->add_a_model_instance(model_ID_castel, mat_world, ID_model_castel);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	*/
	//����ֲ��
	check_error = geometry_buffer->load_a_plantmodel_type("plant\\Palmetto_Low\\Palmetto_Low.pancymesh", "plant\\Palmetto_Low\\Palmetto_Low.pancymat","plant\\Palmetto_Low\\Palmetto_Low.pancymeshanim", false, model_ID_plant);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->add_a_model_instance(model_ID_plant, mat_world, ID_model_plant);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	//�������
	check_error = geometry_buffer->load_a_model_type("ballmodel\\outmodel.pancymesh", "ballmodel\\outmodel.pancymat", false, model_ID_sky);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->add_a_model_instance(model_ID_sky, mat_world, ID_model_sky);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	geometry_resource_view *data_view = NULL;
	geometry_buffer->get_a_model_type(&data_view, model_ID_sky);
	data_view->set_cull_front();
	//���ز���ģ��
	check_error = geometry_buffer->load_a_skinmodel_type("yuriskin\\yuri.pancyskinmesh", "yuriskin\\yuri.pancymat", "yuriskin\\yuri.pancyskin", false, model_ID_skin, 100);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->load_a_skinmodel_animation(model_ID_skin, "yuriskin\\yuri.pancyanimation", animation_id);

	check_error = geometry_buffer->add_a_model_instance(model_ID_skin, mat_world, ID_model_skin);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->add_a_model_instance(model_ID_skin, mat_world, ID_model_skin2);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->set_a_instance_animation(ID_model_skin, animation_id);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->set_a_instance_animation(ID_model_skin2, animation_id);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	/*
	HRESULT hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), L"Texture_cube1.dds", 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &tex_cubesky);
	check_error = create_cubemap();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	*/
	/*
	terrain_file_path terrain_file;

	terrain_file.height_rawdata_name = "terrain_data\\terrain.raw";
	terrain_file.normal_texdata_name = "terrain_data\\terrainnorm1lk.dds";
	terrain_file.tangent_texdata_name = "terrain_data\\terraintangent1k.dds";
	terrain_file.blend_texdata_name = "terrain_data\\terrainblend.dds";

	terrain_file.color_albe_texdata_name[0] = "terrain_data\\texdds\\ground_desertSandDuneless_2k_alb_s.dds";
	terrain_file.color_norm_texdata_name[0] = "terrain_data\\texdds\\ground_desertSandDuneless_2k_n.dds";

	terrain_file.color_albe_texdata_name[1] = "terrain_data\\texdds\\Ground_ScrubGrassField_2k_alb_s.dds";
	terrain_file.color_norm_texdata_name[1] = "terrain_data\\texdds\\Ground_ScrubGrassField_2k_n.dds";

	terrain_file.color_albe_texdata_name[2] = "terrain_data\\texdds\\Ground_MountainTerrain_2k_alb_s.dds";
	terrain_file.color_norm_texdata_name[2] = "terrain_data\\texdds\\Ground_MountainTerrain_2k_n.dds";

	terrain_file.color_albe_texdata_name[3] = "terrain_data\\texdds\\Rock_GuiMossyRock_2k_alb_s.dds";
	terrain_file.color_norm_texdata_name[3] = "terrain_data\\texdds\\Rock_GuiMossyRock_2k_n.dds";
	*/
	/*
	terrain_need = new pancy_terrain_part(2048.0f, 200, 100.0f, 1000.0f, XMFLOAT2(0.0f, 0.0f), "terrain_data\\pancy.ptl");
	check_error = terrain_need->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	*/
	//���ز���ɽ��
	check_error = geometry_buffer->load_terrain(physic_scene, "terrain_data\\pancy.ptn", 4096.0f, 30, 100.0f, 2000.0f, 1.0f - 1.0f / 3.0f);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->get_terrain_data()->load_terrain_material("terrain_data\\terrain_material\\Ground_NeedlyDirtGround\\Ground_NeedlyDirtGround.ptt",terrain_material_mid);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->get_terrain_data()->load_terrain_material("terrain_data\\terrain_material\\Ground_RainforestCover\\Ground_RainforestCover.ptt", terrain_material_mid);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->get_terrain_data()->load_terrain_material("terrain_data\\terrain_material\\grass_MixedAutumnGrass\\grass_MixedAutumnGrass.ptt", terrain_material_mid);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->get_terrain_data()->load_terrain_material("terrain_data\\terrain_material\\Brick_RuinsBrickWithMixedRubble\\Brick_RuinsBrickWithMixedRubble.ptt", terrain_material_mid);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	/*
	terrain_test = new pancy_terrain_control(physic_scene,"terrain_data\\pancy.ptn", 2048.0f, 200, 100.0f, 1000.0f, 1.0f - 1.0f / 3.0f);
	check_error = terrain_test->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	*/
	//���ز�������
	particle_fire = new particle_looping<point_ParticleBasic>(5000);
	check_error = particle_fire->create(L"flare0.dds");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	//���ز����������
	check_error = geometry_buffer->load_a_model_type("square_test\\square.pancymesh", "square_test\\square.pancymat", false, model_ID_box);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->add_a_model_instance(model_ID_box, mat_world, ID_model_box);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	physx::PxTransform box_pos = physx::PxTransform(physx::PxVec3(0.0f, 1200.0f, 0.0f));
	physx::PxBoxGeometry box_geo = physx::PxBoxGeometry(physx::PxVec3(0.5f, 0.5f, 0.5f));
	physic_scene->add_a_dynamic_object(box_pos, box_geo,XMFLOAT3(0.5,0.5,0.5),box_physx);
	physic_scene->wakeup_a_dynamic(box_physx);
	//todo����
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void scene_test_environment::display()
{
	//show_model_single("LightTech");
	show_animation_test();
	show_sky_single();
	show_terrain();
	show_physic_box();
	show_plant_test();
	show_particle();
	show_sky_cube();
	
}
void scene_test_environment::show_particle()
{
	particle_fire->draw_particle();
}
void scene_test_environment::display_environment(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix)
{
}
void scene_test_environment::update(float delta_time)
{
	time_need = delta_time;
	float move_speed = 0.525f;
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
	//���³Ǳ�
	/*
	trans_world = XMMatrixTranslation(0.0, 5.0, 0.0);
	scal_world = XMMatrixScaling(1, 1, 1);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	geometry_resource_view *data_need;
	geometry_buffer->update_a_model_instance(ID_model_castel, world_matrix, delta_time);
	*/
	//����ֲ��
	trans_world = XMMatrixTranslation(0.0, 210.0, 0.0);
	rotation_world = XMMatrixRotationX(XM_PIDIV2);
	scal_world = XMMatrixScaling(1.0, 1, 1);
	XMStoreFloat4x4(&world_matrix, scal_world * rotation_world * trans_world);
	geometry_resource_view *data_need;
	geometry_buffer->update_a_model_instance(ID_model_plant, world_matrix, 0.005f);
	//�������
	test_IBL->update_sky_data(delta_time);
	XMFLOAT3 view_pos;
	scene_camera->get_view_position(&view_pos);
	trans_world = XMMatrixTranslation(view_pos.x, view_pos.y, view_pos.z);
	scal_world = XMMatrixScaling(150, 150, 150);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	geometry_buffer->update_a_model_instance(ID_model_sky, world_matrix, delta_time);
	//��������
	float rand_time = static_cast<float>(rand() % 1000) / 20.0f;
	XMFLOAT4X4 view_mat, proj_mat;
	scene_camera->count_view_matrix(&view_mat);
	proj_mat = engine_basic::perspective_message::get_instance()->get_proj_matrix();
	XMFLOAT4X4 part_proj;
	XMStoreFloat4x4(&part_proj, XMLoadFloat4x4(&view_mat) * XMLoadFloat4x4(&proj_mat));
	particle_fire->update(delta_time, rand_time, &part_proj, &view_pos);
	particle_fire->set_particle_direct(&XMFLOAT3(0, 0, 0), &XMFLOAT3(0, 0, 0));
	//���µ���
	geometry_buffer->get_terrain_data()->update(view_pos, view_mat, proj_mat);
	//��������

	physic_scene->update(0.003);
	XMFLOAT3 pos_physic, rotation_vec;
	float rotation_angle;
	physic_scene->get_position_dynamic(box_physx, pos_physic);
	trans_world = XMMatrixTranslation(pos_physic.x, pos_physic.y, pos_physic.z);
	physic_scene->get_rotation_dynamic(box_physx, rotation_angle, rotation_vec);
	rotation_world = XMMatrixRotationAxis(XMLoadFloat3(&rotation_vec), rotation_angle);
	XMStoreFloat4x4(&world_matrix, rotation_world  * trans_world);
	geometry_buffer->update_a_model_instance(ID_model_box, world_matrix,delta_time);
}
void scene_test_environment::release()
{
	//�ͷŻ�����Դ
	release_basic();
	//�ͷ�������Դ
	particle_fire->release();
	test_IBL->release();

	//terrain_need->release();
	//terrain_test->release();
	test_model->release();
	//pancy_geometry_control_singleton::get_instance()->release();
	/*
	environment_texture_data->release();
	tex_cubesky->Release();
	SRV_cube->Release();
	reflect_cube_DSV->Release();
	for (int i = 0; i < 7; ++i)
	{
		for (int j = 0; j < 6; ++j)
		{
			RTV_cube[i * 6 + j]->Release();
		}
	}
	SRV_singlecube->Release();
	for (int i = 0; i < 6; ++i)
	{
		RTV_singlecube[i]->Release();
	}
	SRV_diffusecube->Release();
	for (int i = 0; i < 6; ++i)
	{
		RTV_diffusecube[i]->Release();
	}
	fullscreen_buffer->release();
	*/

}
void scene_test_environment::show_sky_single()
{
	engine_basic::engine_fail_reason check_error;
	auto shader_test = shader_control::GetInstance()->get_shader_sky_draw(check_error);
	auto shader_light_deffered = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	geometry_resource_view *data_view = NULL;
	geometry_buffer->get_a_model_type(&data_view, model_ID_sky);
	//ѡ������·��
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_sky");
	shader_test->set_trans_world(&data_view->get_matrix_list()[0]);
	//�趨������ͼ
	auto sky_data = test_IBL->get_IBL_data_by_time(time_need);
	if (sky_data != NULL)
	{
		shader_light_deffered->set_IBL_tex(sky_data->get_SRV_spec());
		shader_light_deffered->set_IBL_diffuse_tex(sky_data->get_SRV_diff());
		shader_test->set_tex_resource(sky_data->get_SRV_spec());
	}
	//shader_test->set_tex_atmosphere_occlusion();
	//shader_test->set_tex_resource(tex_cubesky);
	//�趨�ܱ任
	XMFLOAT4X4 view_mat, final_mat, viewproj;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());


	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&data_view->get_matrix_list()[0]);
	XMMATRIX worldViewProj = world_matrix_rec*XMLoadFloat4x4(&view_mat)*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);

	data_view->get_technique(teque_need);

	data_view->draw(false);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetState(NULL);
}
void scene_test_environment::show_physic_box()
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);

	geometry_resource_view *data_view = NULL;
	geometry_buffer->get_a_model_type(&data_view, model_ID_box);
	XMFLOAT4X4 view_mat, final_mat, viewproj;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());


	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&data_view->get_matrix_list()[0]);
	XMMATRIX worldViewProj = world_matrix_rec*XMLoadFloat4x4(&view_mat)*proj;

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
	shader_need->get_technique(&teque_need, "LightTech");

	data_view->get_technique(teque_need);

	data_view->draw(false);
}
void scene_test_environment::show_plant_test()
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);

	geometry_resource_view *data_view = NULL;
	geometry_buffer->get_a_model_type(&data_view, model_ID_plant);
	XMFLOAT4X4 view_mat, final_mat, viewproj;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());


	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&data_view->get_matrix_list()[0]);
	XMMATRIX worldViewProj = world_matrix_rec*XMLoadFloat4x4(&view_mat)*proj;

	XMMATRIX rec_world = XMLoadFloat4x4(&data_view->get_matrix_list()[0]) * XMLoadFloat4x4(&view_mat) * proj;
	XMStoreFloat4x4(&final_mat, rec_world);


	shader_need->set_trans_world(&data_view->get_matrix_list()[0]);
	shader_need->set_trans_all(&final_mat);
	shader_need->set_tex_diffuse_array(data_view->get_texture());

	ID3DX11EffectTechnique *teque_need;
	shader_need->get_technique(&teque_need, "LightTech_plant");
	shader_need->set_animation_offset(data_view->get_meshanim_singledata());
	shader_need->set_animation_buffer(data_view->get_mesh_animation_list());
	data_view->get_technique(teque_need);
	data_view->draw(false);
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(0, blendFactor, 0xffffffff);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetDepthStencilState(0, 0);
}
void scene_test_environment::show_sky_cube()
{
	/*
	bool check_finish = test_IBL->display_an_IBL_data();
	if (!check_finish)
	{
		if_finish = true;
	}
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



		//����ȡ���任
		XMFLOAT3 pos_view;
		XMFLOAT3 look_vec, up_vec;
		XMFLOAT4X4 view_matrix_reflect, invview_matrix_reflect, Proj_mat_reflect;
		look_vec = look_cube_reflect[i];
		up_vec = up_cube_reflect[i];
		pancy_camera::get_instance()->get_view_position(&pos_view);
		pancy_camera::get_instance()->count_view_matrix(look_vec, up_vec, pos_view, &view_matrix_reflect);
		pancy_camera::get_instance()->count_invview_matrix(look_vec, up_vec, pos_view, &invview_matrix_reflect);
		XMStoreFloat4x4(&Proj_mat_reflect, DirectX::XMMatrixPerspectiveFovLH(0.5f*XM_PI, 1.0f, engine_basic::perspective_message::get_instance()->get_perspective_near_plane(), engine_basic::perspective_message::get_instance()->get_perspective_far_plane()));

		engine_basic::extra_perspective_message *scene_perspective = new engine_basic::extra_perspective_message(1024.0f * quality_reflect, 1024.0f * quality_reflect, 0.1f, 1000.0f, DirectX::XM_PIDIV2);
		Pretreatment_gbuffer::get_instance()->render_gbuffer(geometry_buffer, environment_texture_data->get_gbuffer(), view_matrix_reflect, scene_perspective, false);
		Pretreatment_gbuffer::get_instance()->render_lbuffer(environment_texture_data->get_gbuffer(), pos_view, view_matrix_reflect, invview_matrix_reflect, scene_perspective, false);
		//pretreat_render->display(view_matrix_reflect);
		//pretreat_render->display_lbuffer(view_matrix_reflect, invview_matrix_reflect, false);

		ID3D11RenderTargetView* renderTargets[1] = { RTV_singlecube[i] };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTargets, reflect_cube_DSV);
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(RTV_singlecube[i], clearColor);
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearDepthStencilView(reflect_cube_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
		//��Ⱦ���
		auto shader_test = shader_control::GetInstance()->get_shader_sky_draw(check_error);

		geometry_resource_view *data_view = NULL;
		geometry_buffer->get_a_model_type(&data_view, model_ID_sky);
		//ѡ������·��
		ID3DX11EffectTechnique *teque_need;
		shader_test->get_technique(&teque_need, "draw_sky_gamma");
		shader_test->set_trans_world(&data_view->get_matrix_list()[0]);
		//�趨������ͼ
		shader_test->set_tex_resource(tex_cubesky);
		//�趨�ܱ任
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
		//��ԭ��Ⱦ״̬
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetState(NULL);
		ID3D11RenderTargetView* nullTargets[1] = { NULL };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, nullTargets, NULL);
	}
	//�ӿ���Ϣ
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
			//������Ⱦ·��
			ID3D11RenderTargetView* renderTargets[1] = { RTV_cube[i * 6 + j] };
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTargets, NULL);
			float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(RTV_cube[i * 6 + j], clearColor);
			//��ʽ��Ⱦ
			engine_basic::engine_fail_reason check_error;
			auto shader_IBL_spec = shader_control::GetInstance()->get_shader_IBL_specular(check_error);
			shader_IBL_spec->set_input_message(XMFLOAT2(0, 0), j, i);
			shader_IBL_spec->set_input_CubeTex(tex_cubesky);
			ID3DX11EffectTechnique *teque_need;
			shader_IBL_spec->get_technique(&teque_need, "IBL_Specular");
			fullscreen_buffer->get_teque(teque_need);
			fullscreen_buffer->show_mesh();
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
		//������Ⱦ·��
		ID3D11RenderTargetView* renderTargets[1] = { RTV_diffusecube[i] };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTargets, NULL);
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(RTV_diffusecube[i], clearColor);
		//��ʽ��Ⱦ
		engine_basic::engine_fail_reason check_error;
		auto shader_IBL_spec = shader_control::GetInstance()->get_shader_IBL_specular(check_error);
		shader_IBL_spec->set_input_message(XMFLOAT2(0, 0), 0, i);
		shader_IBL_spec->set_input_CubeTex(tex_cubesky);
		ID3DX11EffectTechnique *teque_need;
		shader_IBL_spec->get_technique(&teque_need, "IBL_Diffuse");
		fullscreen_buffer->get_teque(teque_need);
		fullscreen_buffer->show_mesh();
	}
	*/
}
void scene_test_environment::show_model_single(string tech_name, XMFLOAT4X4 *view_matrix, XMFLOAT4X4 *proj_matrix)
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);

	geometry_resource_view *data_view = NULL;
	geometry_buffer->get_a_model_type(&data_view, model_ID_castel);
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
void scene_test_environment::show_animation_test()
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);


	XMFLOAT4X4 world_mat, world_mat2, view_mat, final_mat;

	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj;
	proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	XMStoreFloat4x4(&world_mat, XMMatrixTranslation(2.0f, 0.0f, 0.0f));
	XMStoreFloat4x4(&world_mat2, XMMatrixIdentity());
	XMMATRIX rec_world = XMMatrixIdentity() * XMLoadFloat4x4(&view_mat) * proj;
	//XMMATRIX rec_world2 = XMMatrixTranslation(1.0f,0.0f,0.0f) * XMLoadFloat4x4(&view_mat) * proj;
	XMStoreFloat4x4(&final_mat, rec_world);


	shader_need->set_trans_world(&world_mat);
	//shader_need->set_trans_all(&final_mat);
	shader_need->set_trans_viewproj(&final_mat);
	shader_need->set_tex_diffuse_array(test_model->get_texture());




	geometry_resource_view *data_view = NULL;
	geometry_buffer->get_a_model_type(&data_view, model_ID_skin);
	data_view->update(ID_model_skin.model_instance, world_mat, 0.2f);
	data_view->update(ID_model_skin2.model_instance, world_mat2, 0.1f);
	auto data_bone_matlist = data_view->get_bone_matrix_list();
	//XMFLOAT4X4 bone_mat[100];
	XMFLOAT4X4 world_mat_list[] = { world_mat ,world_mat2 };
	int bone_size = 0;

	shader_need->set_bonemat_buffer(data_view->get_bone_matrix_list());
	shader_need->set_world_matrix_array(world_mat_list, 2);
	shader_need->set_bone_num(data_view->get_bone_mat_num());
	ID3DX11EffectTechnique *teque_need;
	//model_reader_PancySkinMesh *pt = dynamic_cast<model_reader_PancySkinMesh*>(test_model);
	//pt->update_animation(0.02f);
	//shader_need->set_bone_matrix(pt->get_bone_matrix(), pt->get_bone_num());
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec_inputdesc[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT" ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXDIFFNORM",0  ,DXGI_FORMAT_R32G32B32A32_FLOAT      ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXOTHER",0  ,DXGI_FORMAT_R32G32B32A32_FLOAT      ,0    ,68 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "BONEINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,84 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "WEIGHTS"     ,0  ,DXGI_FORMAT_R32G32B32A32_FLOAT ,0    ,100 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
	};
	UINT num_member = sizeof(rec_inputdesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	shader_need->get_technique(rec_inputdesc, num_member, &teque_need, "LightTech_instance_bone");

	data_view->get_technique(teque_need);
	data_view->draw(false);
	//test_model->get_technique(teque_need);
	//test_model->draw_mesh();
}
void scene_test_environment::show_terrain()
{
	engine_basic::engine_fail_reason check_error;
	//�趨�ܱ任
	/*
	XMFLOAT4X4 view_mat, proj_mat;
	XMFLOAT3 view_pos;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	pancy_camera::get_instance()->get_view_position(&view_pos);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	XMStoreFloat4x4(&proj_mat, proj);
	terrain_need->render_terrain(view_pos, view_mat, proj_mat);
	*/
	geometry_buffer->get_terrain_data()->display();

}
/*
engine_basic::engine_fail_reason scene_test_environment::create_cubemap()
{
	HRESULT hr;
	D3D11_TEXTURE2D_DESC cubeMapDesc;
	//��ȾĿ��
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
	//�߲�mipmap����
	ID3D11Texture2D *cubeMap_stencil(NULL);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&cubeMapDesc, 0, &cubeMap_stencil);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap stencil texture error when create scene_test_environment tex");
		return error_message;
	}
	//��������rendertarget
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
	//����һ��SRV
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
	//ԭʼ��mipmap����
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
	//����һ��SRV
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
	//������IBL��ͼ
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
	//����һ��SRV
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
	//��ʱ��Ȼ�����
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
	//����������Դ
	ID3D11Texture2D* depthMap = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &depthMap);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect depthtexture error when create ssrinput tex");
		return error_message;
	}
	//������Ȼ�����������
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
*/

scene_test_plant::scene_test_plant()
{
}
engine_basic::engine_fail_reason scene_test_plant::create()
{
	engine_basic::engine_fail_reason check_error;
	//������������Դ
	check_error = create_basic();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	//������Դ
	XMFLOAT4X4 mat_world;
	XMStoreFloat4x4(&mat_world, XMMatrixIdentity());
	//�������
	check_error = geometry_buffer->load_a_model_type("ballmodel\\outmodel.pancymesh", "ballmodel\\outmodel.pancymat", false, model_ID_sky);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->add_a_model_instance(model_ID_sky, mat_world, ID_model_sky);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	geometry_resource_view *data_view = NULL;
	geometry_buffer->get_a_model_type(&data_view, model_ID_sky);
	data_view->set_cull_front();
	//���ع�������׵��
	check_error = geometry_buffer->load_a_model_type("bone_test\\bone_test.pancymesh", "bone_test\\bone_test.pancymat", false, model_ID_bone);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	//���ع����ṹ
	check_error = load_skintree("testtree\\test.pancyskin");
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	update_root(root_skin, mat_world,-1);
	get_bone_matrix();
	//���ز���ģ��
	check_error = geometry_buffer->load_a_skinmodel_type("testtree\\test.pancyskinmesh", "testtree\\test.pancymat", "testtree\\test.pancyskin", false, model_ID_skin, 100);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->load_a_skinmodel_animation(model_ID_skin, "testtree\\test.pancyanimation", animation_id);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->add_a_model_instance(model_ID_skin, mat_world, ID_model_skin);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = geometry_buffer->set_a_instance_animation(ID_model_skin, animation_id);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	//��Ӳ���ʵ��
	for (int i = 0; i < bone_num; ++i)
	{
		check_error = geometry_buffer->add_a_model_instance(model_ID_bone, mat_world, ID_model_bone[i]);
		if (!check_error.check_if_failed())
		{
			return check_error;
		}
	}

	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void scene_test_plant::display()
{
	show_sky_single();
	show_bone();
	show_skinmesh();
}
void scene_test_plant::display_environment(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix)
{
}
void scene_test_plant::update(float delta_time)
{
	float move_speed = 0.525f;
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
	//�������
	XMFLOAT3 view_pos;
	scene_camera->get_view_position(&view_pos);
	trans_world = XMMatrixTranslation(view_pos.x, view_pos.y, view_pos.z);
	scal_world = XMMatrixScaling(50, 50, 50);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	geometry_buffer->update_a_model_instance(ID_model_sky, world_matrix, delta_time);
	//���¹���������
	
	//geometry_buffer->update_a_model_instance(ID_model_bone, world_matrix, delta_time);
	for (int i = 0; i < bone_num; ++i)
	{
		trans_world = XMLoadFloat4x4(&bone_matrix_array[i]);
		//scal_world = XMMatrixRotationX(-XM_PIDIV2);
		scal_world = XMMatrixScaling(0.1,0.1,0.1);
		XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
		geometry_buffer->update_a_model_instance(ID_model_bone[i], world_matrix, delta_time);
	}
}
void scene_test_plant::release()
{
	//�ͷŻ�����Դ
	release_basic();
	//�ͷ�������Դ

}
void scene_test_plant::show_sky_single()
{
	engine_basic::engine_fail_reason check_error;
	auto shader_test = shader_control::GetInstance()->get_shader_sky_draw(check_error);

	geometry_resource_view *data_view = NULL;
	geometry_buffer->get_a_model_type(&data_view, model_ID_sky);
	//ѡ������·��
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_sky");
	shader_test->set_trans_world(&data_view->get_matrix_list()[0]);
	//�趨������ͼ
	//auto sky_data = test_IBL->get_IBL_data_by_time(time_need);
	//if (sky_data != NULL)
	//{
	//	shader_test->set_tex_resource(sky_data->get_SRV_spec());
	//}
	//shader_test->set_tex_resource(tex_cubesky);
	//�趨�ܱ任
	XMFLOAT4X4 view_mat, final_mat, viewproj;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());



	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&data_view->get_matrix_list()[0]);
	XMMATRIX worldViewProj = world_matrix_rec*XMLoadFloat4x4(&view_mat)*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);

	data_view->get_technique(teque_need);

	data_view->draw(false);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetState(NULL);
}
void scene_test_plant::show_bone()
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);

	geometry_resource_view *data_view = NULL;
	geometry_buffer->get_a_model_type(&data_view, model_ID_bone);
	XMFLOAT4X4 view_mat, final_mat,view_proj;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj;
	proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());

	XMMATRIX rec_world = XMLoadFloat4x4(&data_view->get_matrix_list()[0]) * XMLoadFloat4x4(&view_mat) * proj;
	XMMATRIX rec_viewproj = XMLoadFloat4x4(&view_mat) * proj;
	XMStoreFloat4x4(&final_mat, rec_world);
	XMStoreFloat4x4(&view_proj, rec_viewproj);

	auto matrix_list = data_view->get_matrix_list();
	XMFLOAT4X4 *data_worldmat_array = new XMFLOAT4X4[matrix_list.size()];
	int count_num = 0;
	for (auto mat_need = matrix_list.begin(); mat_need != matrix_list.end(); ++mat_need)
	{
		data_worldmat_array[count_num++] = *mat_need._Ptr;
	}

	shader_need->set_trans_all(&final_mat);
	shader_need->set_tex_diffuse_array(data_view->get_texture());
	shader_need->set_world_matrix_array(data_worldmat_array, data_view->get_matrix_list().size());
	shader_need->set_trans_viewproj(&view_proj);
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
	shader_need->get_technique(&teque_need, "LightTech_instance");

	data_view->get_technique(teque_need);

	data_view->draw(false);
}
void scene_test_plant::read_bone_tree(skin_tree *now)
{
	char data[11];
	skin_instream.read(reinterpret_cast<char*>(now), sizeof(*now));
	now->brother = NULL;
	now->son = NULL;
	skin_instream.read(data, sizeof(data));
	while (strcmp(data, "*heaphead*") == 0)
	{
		//��ջ���ţ������ӽڵ�
		skin_tree *now_point = new skin_tree();
		read_bone_tree(now_point);
		now_point->brother = now->son;
		now->son = now_point;
		skin_instream.read(data, sizeof(data));
	}
}
engine_basic::engine_fail_reason scene_test_plant::load_skintree(string filename)
{
	skin_instream.open(filename, ios::binary);
	if (!skin_instream.is_open())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "open file " + filename + " error");
		return error_message;
	}
	//��ȡƫ�ƾ���
	int bone_num_need;
	skin_instream.read(reinterpret_cast<char*>(&bone_num), sizeof(bone_num));
	skin_instream.read(reinterpret_cast<char*>(offset_matrix_array), bone_num*sizeof(XMFLOAT4X4));
	//�ȶ�ȡ��һ����ջ��
	char data[11];
	skin_instream.read(reinterpret_cast<char*>(data), sizeof(data));
	root_skin = new skin_tree();
	//�ݹ��ؽ�������
	read_bone_tree(root_skin);
	//�ر��ļ�
	skin_instream.close();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void scene_test_plant::update_root(skin_tree *root, XMFLOAT4X4 matrix_parent,int ID_parent)
{
	if (root == NULL)
	{
		return;
	}
	XMMATRIX rec = XMLoadFloat4x4(&root->animation_matrix);
	XMStoreFloat4x4(&root->now_matrix, rec * XMLoadFloat4x4(&matrix_parent));
	if (root->bone_number >= 0)
	{
		bone_matrix_array[root->bone_number] = root->now_matrix;
		parent_ID[root->bone_number] = ID_parent;
	}
	update_root(root->brother, matrix_parent, root->bone_number);
	update_root(root->son, root->now_matrix, root->bone_number);
}
void scene_test_plant::get_bone_matrix()
{
	for (int i = 0; i < bone_num; ++i)
	{
		XMStoreFloat4x4(&final_matrix_array[i], XMLoadFloat4x4(&offset_matrix_array[i]) * XMLoadFloat4x4(&bone_matrix_array[i]));
	}
}
void scene_test_plant::show_skinmesh() 
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	//auto data = geometry_buffer->

	XMFLOAT4X4 world_mat, view_mat, final_mat;

	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj;
	proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	XMStoreFloat4x4(&world_mat, XMMatrixIdentity());
	XMMATRIX rec_world = XMMatrixIdentity() * XMLoadFloat4x4(&view_mat) * proj;
	//XMMATRIX rec_world2 = XMMatrixTranslation(1.0f,0.0f,0.0f) * XMLoadFloat4x4(&view_mat) * proj;
	XMStoreFloat4x4(&final_mat, rec_world);


	shader_need->set_trans_world(&world_mat);
	shader_need->set_trans_viewproj(&final_mat);
	




	geometry_resource_view *data_view = NULL;
	geometry_buffer->get_a_model_type(&data_view, model_ID_skin);
	data_view->update(ID_model_skin.model_instance, world_mat, 0.2f);
	shader_need->set_tex_diffuse_array(data_view->get_texture());
	auto data_bone_matlist = data_view->get_bone_matrix_list();
	//XMFLOAT4X4 bone_mat[100];
	XMFLOAT4X4 world_mat_list[] = { world_mat};
	int bone_size = 0;

	shader_need->set_bonemat_buffer(data_view->get_bone_matrix_list());
	shader_need->set_world_matrix_array(world_mat_list, 1);
	shader_need->set_bone_num(data_view->get_bone_mat_num());
	ID3DX11EffectTechnique *teque_need;
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec_inputdesc[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT" ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXDIFFNORM",0  ,DXGI_FORMAT_R32G32B32A32_FLOAT      ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXOTHER",0  ,DXGI_FORMAT_R32G32B32A32_FLOAT      ,0    ,68 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "BONEINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,84 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "WEIGHTS"     ,0  ,DXGI_FORMAT_R32G32B32A32_FLOAT ,0    ,100 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
	};
	UINT num_member = sizeof(rec_inputdesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	shader_need->get_technique(rec_inputdesc, num_member, &teque_need, "LightTech_instance_bone");

	data_view->get_technique(teque_need);
	data_view->draw(false);

	XMFLOAT4X4 *mat_list = NULL;
	int bone_num_now = 0;
	data_view->get_bonematrix_singledata(&mat_list, bone_num_now);
	for (int i = 0; i < bone_num_now; ++i)
	{
		final_matrix_array[i] = mat_list[i];
	}
}

pancy_scene_control::pancy_scene_control()
{
	sundir = XMFLOAT3(0, 0.227391526, 0.0407850109);
	update_time = 0.0f;
	scene_now_show = -1;
	quality_reflect = 0.5f;
	picture_buf = new mesh_square(false);
	ssao_render = new ssao_pancy(d3d_pancy_basic_singleton::GetInstance()->get_wind_width(), d3d_pancy_basic_singleton::GetInstance()->get_wind_height());
	atmosphere_texture = new atmosphere_pretreatment();
	environment_map_list = new real_time_environment(1.0f);
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
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~��̬cubemap����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//��ȾĿ��
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
	//ʹ������������������
	ID3D11Texture2D *cubeMap(NULL);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&cubeMapDesc, 0, &cubeMap);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap texture error when create ssrinput tex");
		return error_message;
	}
	//��������rendertarget
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
	//����һ��SRV
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
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~����������ͼ��̨����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11Texture2D *cubeMap_back(NULL);
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&cubeMapDesc, 0, &cubeMap_back);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap backbuffer error when create ssrinput tex");
		return error_message;
	}
	//��������rendertarget
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
	//����һ��SRV
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(cubeMap_back, &srvDesc_stencil, &reflect_cube_SRV_backbuffer);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect cubemap backbuffer SRV error when create ssrinput tex");
		return error_message;
	}
	cubeMap_back->Release();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~������ʱ����Ȼ�����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
	//����������Դ
	ID3D11Texture2D* depthMap = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &depthMap);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create reflect depthtexture error when create ssrinput tex");
		return error_message;
	}
	//������Ȼ�����������
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
	engine_basic::engine_fail_reason check_error;
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
	check_error = environment_map_list->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void pancy_scene_control::render_environment()
{
}

/*
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
			//����ȡ���任
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
			auto shader_sky = shader_control::GetInstance()->get_shader_sky_draw(check_error);
			shader_sky->set_tex_atmosphere(pretreat_render->get_reflect_atmosphere());
			shader_sky->set_trans_texproj(&VPT);
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
*/
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
			time_count += pancy_input::GetInstance()->MouseMove_Y() * 0.01f;
			if (time_count > XM_2PI)
			{
				time_count -= XM_2PI;
			}
			sundir.x = cos(time_count);
			sundir.y = -sin(time_count);
			sundir.z = 0;
		}
	}
	if (scene_now_show >= 0 && scene_now_show < scene_list.size())
	{
		scene_list[scene_now_show]->update(time_count);
	}
}
void pancy_scene_control::display()
{
	//�������������
	auto scene_camera = pancy_camera::get_instance();
	XMFLOAT4X4 view_mat, inv_view_mat;
	XMFLOAT3 view_pos;
	scene_camera->count_invview_matrix(&inv_view_mat);
	scene_camera->get_view_position(&view_pos);
	scene_camera->count_view_matrix(&view_mat);
	//����brdf��ͼ
	engine_basic::engine_fail_reason check_error;
	auto shader_deffered = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	shader_deffered->set_tex_brdflist_resource(brdf_pic);
	shader_deffered->set_trans_invview(&inv_view_mat);
	shader_deffered->set_view_pos(view_pos);
	//��Ⱦgbuffer

	if (scene_now_show >= 0 && scene_now_show < scene_list.size())
	{
		//��ͶӰ
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
		auto shader_pretreat_lbuffer = shader_control::GetInstance()->get_shader_lightbuffer(check_error);
		shader_pretreat_lbuffer->set_view_from_clip(clip_matrix);
		
		
		//gbuffer
		engine_basic::extra_perspective_message scene_perspective(d3d_pancy_basic_singleton::GetInstance()->get_wind_width(), d3d_pancy_basic_singleton::GetInstance()->get_wind_height(), engine_basic::perspective_message::get_instance()->get_perspective_near_plane(), engine_basic::perspective_message::get_instance()->get_perspective_far_plane(), DirectX::XM_PIDIV4);
		Pretreatment_gbuffer::get_instance()->render_gbuffer(scene_list[scene_now_show]->get_geometry_buffer(), scene_list[scene_now_show]->get_gbuffer_renderdata(), view_mat, &scene_perspective, false,false);

		//��ȾAO
		ssao_render->get_normaldepthmap(scene_list[scene_now_show]->get_gbuffer_renderdata()->normalspec_tex, scene_list[scene_now_show]->get_gbuffer_renderdata()->depthmap_single_tex);
		ssao_render->compute_ssaomap();
		ssao_render->blur_ssaomap();
		
		//pretreat_render->display();
		//������Ӱ
		light_control_singleton::GetInstance()->draw_shadow(scene_list[scene_now_show]->get_geometry_buffer());
		//�������
		Pretreatment_gbuffer::get_instance()->render_lbuffer(scene_list[scene_now_show]->get_gbuffer_renderdata(), view_pos, view_mat, inv_view_mat, &scene_perspective, true);
		//���ӳٹ�����ȼ�¼
		Pretreatment_gbuffer::get_instance()->render_gbuffer(scene_list[scene_now_show]->get_geometry_buffer(), scene_list[scene_now_show]->get_gbuffer_renderdata(), view_mat, &scene_perspective, false,true);
		//���ù�����ͼ
		auto shader_deffered = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
		shader_deffered->set_diffuse_light_tex(scene_list[scene_now_show]->get_gbuffer_renderdata()->gbuffer_diffuse_tex);
		shader_deffered->set_specular_light_tex(scene_list[scene_now_show]->get_gbuffer_renderdata()->gbuffer_specular_tex);
		shader_deffered->set_normal_tex(scene_list[scene_now_show]->get_gbuffer_renderdata()->normalspec_tex);
		shader_deffered->set_tex_specroughness_resource(scene_list[scene_now_show]->get_gbuffer_renderdata()->specroughness_tex);
		shader_deffered->set_tex_atmosphere_fog(scene_list[scene_now_show]->get_gbuffer_renderdata()->gbuffer_atmosphere_tex);
		//deffered shading��ɫ��Ⱦ
		auto reflect_buffer = scene_list[scene_now_show]->get_postbuffer_data();
		ID3D11RenderTargetView* renderTargets[2] = { reflect_buffer->rtgr_input_target,reflect_buffer->rtgr_InputMask_target };
		d3d_pancy_basic_singleton::GetInstance()->set_render_target(renderTargets, 2);
		d3d_pancy_basic_singleton::GetInstance()->clear_basicrender_target();
		//d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(2, renderTargets, NULL);
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float mask_clearColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(reflect_buffer->rtgr_input_target, clearColor);
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(reflect_buffer->rtgr_InputMask_target, mask_clearColor);
		scene_list[scene_now_show]->display();
		scene_list[scene_now_show]->display_nopost();
		//������ɫ����
		render_posttreatment_RTGR::get_instance()->draw_reflect(
			scene_list[scene_now_show]->get_gbuffer_renderdata(),
			reflect_buffer, environment_map_list->get_scene_center(),
			environment_map_list->get_env_color_texture(),
			environment_map_list->get_env_depth_texture()
			);
		//HDR����
		render_posttreatment_HDR::get_instance()->display(
			reflect_buffer->reflect_out_tex,
			scene_list[scene_now_show]->get_HDRbuffer_data(),
			d3d_pancy_basic_singleton::GetInstance()->get_back_buffer()
			);
		
		environment_map_list->display_a_turn(scene_list[scene_now_show]);
	}
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
	light_control_singleton::GetInstance()->update_sunlight(sundir);
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
	reflect_cube_DSV->Release();
	brdf_target->Release();
	brdf_pic->Release();
	picture_buf->release();
	atmosphere_texture->release();
	environment_map_list->release();
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


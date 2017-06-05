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
	engine_basic::engine_fail_reason check_error = pancy_geometry_control_singleton::get_instance()->load_a_model_type("outmodel.pancymesh", "outmodel.pancymat", model_ID_castel);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	XMFLOAT4X4 mat_world;
	XMStoreFloat4x4(&mat_world,XMMatrixIdentity());
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

	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void scene_test_square::display()
{
	show_model_single();
	show_floor_single();
}
void scene_test_square::show_model_single()
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_lightdeffered(check_error);

	geometry_resource_view *data_view = NULL;
	pancy_geometry_control_singleton::get_instance()->get_a_model_type(&data_view, model_ID_castel);
	XMFLOAT4X4 view_mat, final_mat;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	XMMATRIX rec_world = XMLoadFloat4x4(&data_view->get_matrix_list()[0]) * XMLoadFloat4x4(&view_mat) * proj;
	XMStoreFloat4x4(&final_mat, rec_world);


	shader_need->set_trans_world(&data_view->get_matrix_list()[0]);
	shader_need->set_trans_all(&final_mat);
	shader_need->set_tex_diffuse_array(data_view->get_texture());
	ID3DX11EffectTechnique *teque_need;
	shader_need->get_technique(&teque_need, "LightTech");

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
	XMFLOAT4X4 view_mat, final_mat;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	XMMATRIX rec_world = XMLoadFloat4x4(&data_view->get_matrix_list()[0]) * XMLoadFloat4x4(&view_mat) * proj;
	XMStoreFloat4x4(&final_mat, rec_world);


	shader_need->set_trans_world(&data_view->get_matrix_list()[0]);
	shader_need->set_trans_all(&final_mat);
	shader_need->set_tex_diffuse_array(data_view->get_texture());
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
void scene_test_square::update(float delta_time)
{
	float move_speed = 0.15f;
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
	trans_world = XMMatrixTranslation(0.0, -5.0, 0.0);
	scal_world = XMMatrixScaling(1, 1, 1);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	geometry_resource_view *data_need;
	pancy_geometry_control_singleton::get_instance()->update_a_model_instance(ID_model_castel, world_matrix, delta_time);
	trans_world = XMMatrixTranslation(0.0, -5.0, 0.0);
	scal_world = XMMatrixScaling(2, 1, 2);
	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	pancy_geometry_control_singleton::get_instance()->update_a_model_instance(ID_model_floor, world_matrix, delta_time);
	//pancy_geometry_control_singleton::get_instance()->sleep_a_model_instance(ID_model_floor);

}
void scene_test_square::release()
{
	pancy_geometry_control_singleton::get_instance()->release();
}


pancy_scene_control::pancy_scene_control()
{
	scene_now_show = -1;
	pretreat_render = new Pretreatment_gbuffer(d3d_pancy_basic_singleton::GetInstance()->get_wind_width(), d3d_pancy_basic_singleton::GetInstance()->get_wind_height());
	ssao_render = new ssao_pancy(d3d_pancy_basic_singleton::GetInstance()->get_wind_width(), d3d_pancy_basic_singleton::GetInstance()->get_wind_height());
}
engine_basic::engine_fail_reason pancy_scene_control::create()
{
	auto check_eroor = pretreat_render->create();
	if (!check_eroor.check_if_failed()) 
	{
		return check_eroor;
	}
	check_eroor = ssao_render->basic_create();
	if (!check_eroor.check_if_failed())
	{
		return check_eroor;
	}
	if (!check_eroor.check_if_failed())
	{
		return check_eroor;
	}
	light_control_singleton::GetInstance()->add_spotlight_with_shadow_map();
	light_control_singleton::GetInstance()->add_sunlight_with_shadow_map(1024,1024,3);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void pancy_scene_control::update(float delta_time)
{
	if (scene_now_show >= 0 && scene_now_show < scene_list.size())
	{
		scene_list[scene_now_show]->update(delta_time);
	}
}
void pancy_scene_control::display()
{
	//äÖÈ¾gbuffer
	pretreat_render->display();
	//äÖÈ¾AO
	ssao_render->get_normaldepthmap(pretreat_render->get_gbuffer_normalspec(), pretreat_render->get_gbuffer_depth());
	ssao_render->compute_ssaomap();
	ssao_render->blur_ssaomap();
	//¼ÆËãÒõÓ°
	light_control_singleton::GetInstance()->draw_shadow();
	//¼ÆËã¹âÕÕ
	pretreat_render->display_lbuffer(true);
	//ÕýÊ½äÖÈ¾
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
	d3d_pancy_basic_singleton::GetInstance()->clear_basicrender_target();
	d3d_pancy_basic_singleton::GetInstance()->reset_viewport();
	if (scene_now_show >= 0 && scene_now_show < scene_list.size())
	{
		scene_list[scene_now_show]->display();
		scene_list[scene_now_show]->display_nopost();
	}
	//½»»»µ½ÆÁÄ»
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
	for (auto data = scene_list.begin(); data != scene_list.end(); ++data)
	{
		(*data._Ptr)->release();
	}
	scene_list.clear();
}


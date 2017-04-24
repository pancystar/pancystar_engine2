#include"pancy_scene_design.h"
scene_root::scene_root()
{
	time_game = 0.0f;
}

scene_test_square::scene_test_square()
{
	rec = 0.0f;
	mesh_need = new mesh_cube(false);
	picture_buf = new mesh_square(false);
	mesh_model = new model_reader_assimp<point_common>("castelmodel\\castel.obj", "castelmodel\\");
}
engine_basic::engine_fail_reason scene_test_square::create()
{
	engine_basic::engine_fail_reason check_error = mesh_need->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = picture_buf->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = mesh_model->model_create(false, 0, NULL);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 2048;
	texDesc.Height = 2048;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* posttreatment_tex = 0;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &posttreatment_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr,"create screen texture error");
		return check_error;
	}
	posttreatment_tex->Release();
	/*
	D3D11_SHADER_RESOURCE_VIEW_DESC desc_need;
	material_list rec_need;
	mesh_model->get_texture(&rec_need, 0);
	rec_need.tex_diffuse_resource->GetDesc(&desc_need);
	*/

	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void scene_test_square::display()
{
	show_model();
}
void scene_test_square::show_model()
{
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_virtual_light(check_error);
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	XMFLOAT4X4 final_matrix;
	rec += 0.001f;
	trans_world = XMMatrixTranslation(0.0, -5.0, 0.0);
	scal_world = XMMatrixScaling(1, 1, 1);
	//XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, 800.0f / 600.0f, 0.1f, 1000.f);
	XMFLOAT4X4 view_mat;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	rec_world = scal_world  *  trans_world * XMLoadFloat4x4(&view_mat) * proj;

	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	XMStoreFloat4x4(&final_matrix, rec_world);
	shader_need->set_trans_world(&world_matrix);
	shader_need->set_trans_all(&final_matrix);
	ID3DX11EffectTechnique *teque_need;
	shader_need->get_technique(&teque_need, "light_tech");
	mesh_model->get_technique(teque_need);
	for (int i = 0; i < mesh_model->get_meshnum(); ++i)
	{
		material_list rec_need;
		mesh_model->get_texture(&rec_need, i);
		shader_need->set_tex_diffuse(rec_need.tex_diffuse_resource);
		mesh_model->draw_part(i);
	}
}
void scene_test_square::show_square() 
{

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
}
void scene_test_square::release()
{
	if (mesh_need != NULL)
	{
		mesh_need->release();
	}
	if (mesh_model != NULL)
	{
		mesh_model->release();
	}
	if (picture_buf != NULL) 
	{
		picture_buf->release();
	}
}


pancy_scene_control::pancy_scene_control()
{
	scene_now_show = -1;
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
	d3d_pancy_basic_singleton::GetInstance()->clear_basicrender_target();
	if (scene_now_show >= 0 && scene_now_show < scene_list.size())
	{
		scene_list[scene_now_show]->display();
		scene_list[scene_now_show]->display_nopost();
	}
	//½»»»µ½ÆÁÄ»
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
	for (auto data = scene_list.begin(); data != scene_list.end(); ++data)
	{
		(*data._Ptr)->release();
	}
	scene_list.clear();
}


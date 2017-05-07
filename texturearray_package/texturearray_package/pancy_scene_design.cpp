#include"pancy_scene_design.h"
scene_root::scene_root()
{
	time_game = 0.0f;
}
//纹理合并算法
texture_combine::texture_combine(int input_pic_num, int *input_width_list, int *input_height_list, int out_pic_width, int out_pic_height)
{
	SRV_array = NULL;
	picture_type_width = out_pic_width;
	picture_type_height = out_pic_height;
	all_pic = input_pic_num;
	for (int i = 0; i < input_pic_num; ++i)
	{
		width[i] = input_width_list[i];
		height[i] = input_height_list[i];
	}
	for (int i = 0; i < 1000; ++i)
	{
		check_if_use[i] = true;
	}
	while (true)
	{
		int_list data_rec = count_dfs(0, 0, picture_type_width, picture_type_height);
		if (data_rec.data_num > 0)
		{
			picture_combine_list.push_back(data_rec);
		}
		else
		{
			break;
		}
	}
}
engine_basic::engine_fail_reason texture_combine::create()
{
	return init_testure();
}
int_list texture_combine::get_texture_data(int index)
{
	if (index < picture_combine_list.size())
	{
		return picture_combine_list[index];
	}
	int_list empty;
	empty.data_num = 0;
	return empty;
}
int_list texture_combine::count_dfs(int now_st_x, int now_st_y, int now_width, int now_height)
{
	int_list data_out;
	data_out.data_num = 0;
	for (int i = 0; i < all_pic; ++i)
	{
		if (width[i] < now_width && height[i] < now_height && check_if_use[i] == true)
		{
			check_if_use[i] = false;
			data_out.data_num = 1;
			data_out.data[0].pic_index = i;
			data_out.data[0].x_st = now_st_x;
			data_out.data[0].y_st = now_st_y;
			//横切
			int_list data_rec1 = count_dfs(now_st_x, now_st_y + height[i], now_width, now_height - height[i]);
			int_list data_rec2 = count_dfs(now_st_x + width[i], now_st_y, now_width - width[i], height[i]);
			int use_check1 = 0;
			for (int j = 0; j < data_rec1.data_num; ++j)
			{
				//拷贝子图片的分割数据
				data_out.data[data_out.data_num] = data_rec1.data[j];
				//计算子图片的利用率
				use_check1 += width[data_rec1.data[j].pic_index] * height[data_rec1.data[j].pic_index];
				//计数器
				data_out.data_num += 1;
			}
			for (int j = 0; j < data_rec2.data_num; ++j)
			{
				//拷贝子图片的分割数据
				data_out.data[data_out.data_num] = data_rec2.data[j];
				//计算子图片的利用率
				use_check1 += width[data_rec2.data[j].pic_index] * height[data_rec2.data[j].pic_index];
				//计数器
				data_out.data_num += 1;
			}

			//竖切
			//还原横切的数据
			for (int j = 0; j < data_rec1.data_num; ++j)
			{
				check_if_use[data_rec1.data[j].pic_index] = true;
			}
			for (int j = 0; j < data_rec2.data_num; ++j)
			{
				check_if_use[data_rec2.data[j].pic_index] = true;
			}
			//尝试竖切
			int use_check2 = 0;
			data_rec1 = count_dfs(now_st_x + width[i], now_st_y, now_width - width[i], now_height);
			data_rec2 = count_dfs(now_st_x, now_st_y + height[i], width[i], now_height - height[i]);
			for (int j = 0; j < data_rec1.data_num; ++j)
			{
				//计算子图片的利用率
				use_check2 += width[data_rec1.data[j].pic_index] * height[data_rec1.data[j].pic_index];
			}
			for (int j = 0; j < data_rec2.data_num; ++j)
			{
				//计算子图片的利用率
				use_check2 += width[data_rec2.data[j].pic_index] * height[data_rec2.data[j].pic_index];
			}
			//对比两种切割效果
			if (use_check2 > use_check1)
			{
				data_out.data_num = 1;
				for (int j = 0; j < data_rec1.data_num; ++j)
				{
					data_out.data[data_out.data_num] = data_rec1.data[j];
					data_out.data_num += 1;
				}
				for (int j = 0; j < data_rec2.data_num; ++j)
				{
					data_out.data[data_out.data_num] = data_rec2.data[j];
					data_out.data_num += 1;
				}
			}
			else
			{
				//还原竖切的数据
				for (int j = 0; j < data_rec1.data_num; ++j)
				{
					check_if_use[data_rec1.data[j].pic_index] = true;
				}
				for (int j = 0; j < data_rec2.data_num; ++j)
				{
					check_if_use[data_rec2.data[j].pic_index] = true;
				}
				//改为横切
				for (int j = 0; j < data_out.data_num; ++j)
				{
					check_if_use[data_out.data[j].pic_index] = false;
				}
			}
			break;
		}
	}
	return data_out;
}
engine_basic::engine_fail_reason texture_combine::init_testure()
{
	//创建纹理数组资源
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = picture_type_width;
	texDesc.Height = picture_type_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = picture_combine_list.size();
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* texpack_texarray = 0;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &texpack_texarray);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create texture pack array resource error");
		return check_error;
	}
	//创建纹理数组打包访问器
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd = {
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
	};
	dsrvd.Texture2DArray.ArraySize = picture_combine_list.size();
	dsrvd.Texture2DArray.FirstArraySlice = 0;
	dsrvd.Texture2DArray.MipLevels = 1;
	dsrvd.Texture2DArray.MostDetailedMip = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(texpack_texarray, &dsrvd, &SRV_array);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create texture pack array resourceview error");
		return check_error;
	}
	//创建纹理数组独立访问器
	for (int index_need = 0; index_need < picture_combine_list.size(); ++index_need)
	{
		D3D11_RENDER_TARGET_VIEW_DESC srvDesc =
		{
			DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D11_RTV_DIMENSION_TEXTURE2DARRAY
		};
		ID3D11RenderTargetView *DTV_rec;
		srvDesc.Texture2DArray.ArraySize = 1;
		srvDesc.Texture2DArray.FirstArraySlice = index_need;
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(texpack_texarray, &srvDesc, &DTV_rec);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason check_error(hr, "create texture pack single rendertarget error");
			return check_error;
		}
		SRV_list.push_back(DTV_rec);
	}
	texpack_texarray->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
ID3D11ShaderResourceView* texture_combine::get_SRV_texarray()
{
	return SRV_array;
}
ID3D11RenderTargetView * texture_combine::get_RTV_texarray(int index)
{
	if (index >= 0 && index < SRV_list.size())
	{
		return SRV_list[index];
	}
	return NULL;
}
texture_rebuild_data texture_combine::get_diffusetexture_data_byID(int ID_tex)
{
	for (int i = 0; i < picture_combine_list.size(); ++i)
	{
		for (int j = 0; j < picture_combine_list[i].data_num; ++j)
		{
			if (ID_tex == picture_combine_list[i].data[j].pic_index)
			{
				texture_rebuild_data data_need;
				data_need.now_index = i;
				data_need.place_data = picture_combine_list[i].data[j];
				data_need.pic_width = width[ID_tex];
				data_need.pic_height = height[ID_tex];
				return data_need;
			}
		}
	}
}
void texture_combine::releae()
{
	if (SRV_array != NULL)
	{
		SRV_array->Release();
	}
	for (int i = 0; i < SRV_list.size(); ++i)
	{
		SRV_list[i]->Release();
	}
}




scene_test_square::scene_test_square()
{
	picture_type_width = 2048;
	picture_type_height = 2048;
	rec = 0.0f;
	mesh_need = new mesh_cube(false);
	picture_buf = new mesh_square(false);
	mesh_model_need = new model_reader_assimp<point_common>("castelmodel\\castel.obj", "castelmodel\\");
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
	check_error = mesh_model_need->model_create(false, 0, NULL);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	int width_list[1000], height_list[1000];
	for (int i = 0; i < mesh_model_need->get_texnum(); ++i)
	{
		material_list rec_need;
		mesh_model_need->get_texture_byindex(&rec_need, i);
		//漫反射贴图
		if (rec_need.tex_diffuse_resource != NULL)
		{
			std::pair<string, ID3D11ShaderResourceView*> data_need(rec_need.texture_diffuse, rec_need.tex_diffuse_resource);
			auto check_iferror = rec_texture_packmap.insert(data_need);
			if (!check_iferror.second)
			{
				continue;
			}
			picture_namelist.push_back(rec_need.texture_diffuse);
			ID3D11Texture2D *resource_rec;
			rec_need.tex_diffuse_resource->GetResource((ID3D11Resource**)&resource_rec);
			D3D11_TEXTURE2D_DESC desc_tex;
			resource_rec->GetDesc(&desc_tex);
			width_list[rec_texture_packmap.size() - 1] = desc_tex.Width;
			height_list[rec_texture_packmap.size() - 1] = desc_tex.Height;
			resource_rec->Release();
		}
		//法线贴图
		if (rec_need.texture_normal_resource != NULL)
		{
			std::pair<string, ID3D11ShaderResourceView*> data_need(rec_need.texture_normal, rec_need.texture_normal_resource);
			auto check_iferror = rec_texture_packmap.insert(data_need);
			if (!check_iferror.second)
			{
				continue;
			}
			picture_namelist.push_back(rec_need.texture_normal);
			ID3D11Texture2D *resource_rec;
			rec_need.texture_normal_resource->GetResource((ID3D11Resource**)&resource_rec);
			D3D11_TEXTURE2D_DESC desc_tex;
			resource_rec->GetDesc(&desc_tex);
			width_list[rec_texture_packmap.size() - 1] = desc_tex.Width;
			height_list[rec_texture_packmap.size() - 1] = desc_tex.Height;
			resource_rec->Release();
		}
		//高光贴图
		if (rec_need.texture_specular_resource != NULL)
		{
			std::pair<string, ID3D11ShaderResourceView*> data_need(rec_need.texture_specular, rec_need.texture_specular_resource);
			auto check_iferror = rec_texture_packmap.insert(data_need);
			if (!check_iferror.second)
			{
				continue;
			}
			picture_namelist.push_back(rec_need.texture_specular);
			ID3D11Texture2D *resource_rec;
			rec_need.texture_specular_resource->GetResource((ID3D11Resource**)&resource_rec);
			D3D11_TEXTURE2D_DESC desc_tex;
			resource_rec->GetDesc(&desc_tex);
			width_list[rec_texture_packmap.size() - 1] = desc_tex.Width;
			height_list[rec_texture_packmap.size() - 1] = desc_tex.Height;
			resource_rec->Release();
		}
		/*
		新的贴图种类在这里填写
		*/

	}
	texture_deal = new texture_combine(rec_texture_packmap.size(), width_list, height_list, 2048, 2048);
	engine_basic::engine_fail_reason error_message = texture_deal->create();
	if (!error_message.check_if_failed())
	{
		return error_message;
	}


	/*
	D3D11_SHADER_RESOURCE_VIEW_DESC desc_need;
	material_list rec_need;
	mesh_model->get_texture(&rec_need, 0);
	rec_need.tex_diffuse_resource->GetDesc(&desc_need);
	*/
	/*
	freopen("test.txt", "w", stdout);
	for (int i = 0; i < rec_texture_packmap.size(); ++i)
	{
		printf("%d,", width_list[i]);
	}
	printf("\n");
	for (int i = 0; i < rec_texture_packmap.size(); ++i)
	{
		printf("%d,", height_list[i]);
	}
	*/
	point_common *data_point_need;
	UINT *data_index_need;
	int vertex_num, index_num;
	mesh_model_need->get_model_pack_num(vertex_num, index_num);
	data_point_need = new point_common[vertex_num];
	data_index_need = new UINT[index_num];
	mesh_model_need->get_model_pack_data(data_point_need, data_index_need);

	change_model_texcoord(data_point_need, vertex_num);

	model_out_test = new mesh_model<point_common>(data_point_need, data_index_need, vertex_num, index_num, false);
	error_message = model_out_test->create_object();
	if (!error_message.check_if_failed())
	{
		return error_message;
	}

	show_square();
	string data_tail[] = { "a","b","c","d","e","f","g","h","i","j","k","l" };
	for (int i = 0; i < texture_deal->get_texture_num(); ++i)
	{
		auto rendertarget = texture_deal->get_RTV_texarray(i);
		ID3D11Resource *resource_rec;
		rendertarget->GetResource(&resource_rec);
		d3d_pancy_basic_singleton::GetInstance()->save_texture(resource_rec,"tex_pack_" + data_tail[i] + ".dds");
	}

	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void scene_test_square::change_model_texcoord(point_common *vertex_need, int point_num)
{
	string lastname;
	texture_rebuild_data last_data;
	texture_rebuild_data texture_message;
	for (int i = 0; i < point_num; ++i)
	{
		//先获取顶点对应的纹理名
		int id_pre_need = vertex_need[i].tex_id.x;
		material_list rec_need;
		mesh_model_need->get_texture(&rec_need, id_pre_need);
		string name_texture = rec_need.texture_diffuse;
		if (name_texture == lastname)
		{
			texture_message = last_data;
		}
		else
		{
			//查找该纹理名对应的纹理ID
			int index_check;
			for (int i = 0; i < picture_namelist.size(); ++i)
			{
				if (picture_namelist[i] == name_texture)
				{
					index_check = i;
					lastname = name_texture;
					break;
				}
			}
			//寻找其位置及ID
			texture_message = texture_deal->get_diffusetexture_data_byID(index_check);
			last_data = texture_message;
		}
		vertex_need[i].tex_id.x = texture_message.now_index;
		//纹理坐标归一化
		while (vertex_need[i].tex.x < 0.0f)
		{
			vertex_need[i].tex.x += 1.0f;
		}
		while (vertex_need[i].tex.x > 1.0f)
		{
			vertex_need[i].tex.x -= 1.0f;
		}
		while (vertex_need[i].tex.y < 0.0f)
		{
			vertex_need[i].tex.y += 1.0f;
		}
		while (vertex_need[i].tex.y > 1.0f)
		{
			vertex_need[i].tex.y -= 1.0f;
		}
		//更新纹理坐标位置
		vertex_need[i].tex.x *= static_cast<float>(texture_message.pic_width) / static_cast<float>(picture_type_width);
		vertex_need[i].tex.y *= static_cast<float>(texture_message.pic_height) / static_cast<float>(picture_type_height);

		vertex_need[i].tex.x += static_cast<float>(texture_message.place_data.x_st) / static_cast<float>(picture_type_width);
		vertex_need[i].tex.y += static_cast<float>(texture_message.place_data.y_st) / static_cast<float>(picture_type_height);
	}
}
void scene_test_square::display()
{

	//show_model();
	show_model_single();
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
	mesh_model_need->get_technique(teque_need);
	for (int i = 0; i < mesh_model_need->get_meshnum(); ++i)
	{
		material_list rec_need;
		mesh_model_need->get_texture(&rec_need, i);
		shader_need->set_tex_diffuse(rec_need.tex_diffuse_resource);
		mesh_model_need->draw_part(i);
	}
}
void scene_test_square::show_model_single()
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
	shader_need->get_technique(&teque_need, "light_tech_array");

	model_out_test->get_teque(teque_need);
	//material_list rec_need;
	//mesh_model_need->get_texture(&rec_need, 0);
	//shader_need->set_tex_diffuse(rec_need.tex_diffuse_resource);
	shader_need->set_tex_diffuse_array(texture_deal->get_SRV_texarray());
	model_out_test->show_mesh();
}
void scene_test_square::show_square()
{
	engine_basic::engine_fail_reason check_error;
	auto shader_picture = shader_control::GetInstance()->get_shader_picture(check_error);
	for (int i = 0; i < texture_deal->get_texture_num(); ++i)
	{
		//修改视口大小
		D3D11_VIEWPORT viewPort;
		viewPort.Width = static_cast<float>(picture_type_width);
		viewPort.Height = static_cast<float>(picture_type_height);
		viewPort.MaxDepth = 1.0f;
		viewPort.MinDepth = 0.0f;
		viewPort.TopLeftX = 0.0f;
		viewPort.TopLeftY = 0.0f;
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
		d3d_pancy_basic_singleton::GetInstance()->set_render_target(texture_deal->get_RTV_texarray(i), NULL);
		auto data_testtex = texture_deal->get_texture_data(i);
		for (int i = 0; i < data_testtex.data_num; ++i)
		{
			data_testtex.data[i].pic_index;
			material_list rec_need;
			//mesh_model->get_texture_byindex(&rec_need, data_testtex.data[i].pic_index + 1);
			auto data_texture = rec_texture_packmap.find(picture_namelist[data_testtex.data[i].pic_index]);

			shader_picture->set_tex_color_resource(data_texture->second);

			int width_need, height_need;
			texture_deal->get_texture_range(width_need, height_need, data_testtex.data[i].pic_index);
			float width_picture = static_cast<float>(width_need) / static_cast<float>(picture_type_width);
			float height_picture = static_cast<float>(height_need) / static_cast<float>(picture_type_height);

			float offset_pos_x = static_cast<float>(data_testtex.data[i].x_st) / (static_cast<float>(picture_type_width) / 2.0f);
			float offset_pos_y = static_cast<float>(data_testtex.data[i].y_st) / (static_cast<float>(picture_type_height) / 2.0f);
			float position_x = -1.0f + offset_pos_x + width_picture;
			float position_y = 1.0f - offset_pos_y - height_picture;

			shader_picture->set_UI_position(XMFLOAT4(position_x, position_y, 0.0f, 0.0f));
			shader_picture->set_UI_scal(XMFLOAT4(width_picture, height_picture, 0.0f, 0.0f));
			ID3DX11EffectTechnique *teque_need;
			shader_picture->get_technique(&teque_need, "draw_ui");
			picture_buf->get_teque(teque_need);
			picture_buf->show_mesh();
		}
	}
	d3d_pancy_basic_singleton::GetInstance()->restore_render_target();
	d3d_pancy_basic_singleton::GetInstance()->clear_basicrender_target();

	/*
	ID3D11Texture2D *resource_rec;
	rec_need.tex_diffuse_resource->GetResource((ID3D11Resource**)&resource_rec);
	D3D11_TEXTURE2D_DESC desc_tex;
	resource_rec->GetDesc(&desc_tex);
	resource_rec->Release();
	*/

	/*
	float width_picture = static_cast<float>(desc_tex.Width) / 2048.0f;
	float height_picture = static_cast<float>(desc_tex.Height) / 2048.0f;
	float position_x = width_picture + (-1.0f);
	float position_y = -height_picture + (1.0f);
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
}
void scene_test_square::release()
{
	if (mesh_need != NULL)
	{
		mesh_need->release();
	}
	if (mesh_model_need != NULL)
	{
		mesh_model_need->release();
	}
	if (picture_buf != NULL)
	{
		picture_buf->release();
	}
	model_out_test->release();
	texture_deal->releae();
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
	//交换到屏幕
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


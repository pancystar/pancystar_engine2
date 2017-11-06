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
		if (width[i] <= now_width && height[i] <= now_height && check_if_use[i] == true)
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
	anim_read = NULL;
	bone_read = NULL;
	if_have_bone = false;
	mesh_model_need = NULL;
	now_show_part = 0;
	model_out_test = NULL;
	test_resource = NULL;
	data_point_need = NULL;
	data_index_need = NULL;
	if_button_down = false;
	if_click = false;
	if_have_model = false;
	picture_type_width = 2048;
	picture_type_height = 2048;
	rec = 0.0f;
	ballmesh_need = new mesh_ball(false, 50, 50);
	mesh_need = new mesh_cube(false);
	picture_buf = new mesh_square(false);
	//mesh_model_need = new model_reader_assimp<point_common>("ball\\ball.obj", "ball\\");
}
engine_basic::engine_fail_reason scene_test_square::read_texture_from_file(std::vector<string> file_name_list)
{
	if (test_resource != NULL)
	{
		test_resource->Release();
		test_resource = NULL;
	}
	std::vector<ID3D11Texture2D*> srcTex(file_name_list.size());
	for (int i = 0; i < file_name_list.size(); ++i)
	{
		size_t length_need = strlen(file_name_list[i].c_str()) + 1;
		size_t converted = 0;
		wchar_t *data_output;
		data_output = (wchar_t*)malloc(length_need*sizeof(wchar_t));
		mbstowcs_s(&converted, data_output, length_need, file_name_list[i].c_str(), _TRUNCATE);
		HRESULT hr = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), data_output, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ, 0, false, (ID3D11Resource**)&srcTex[i], 0, 0);
		//HRESULT hr = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), file_name_list[i], 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ, 0, 0, false, (ID3D11Resource**)&srcTex[i], NULL);
		//HRESULT hr = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), file_name_list[i], 0, D3D11_USAGE_STAGING, 0, D3D11_BIND_SHADER_RESOURCE, 0, false, NULL, &srcres[i], 0);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "load model texture" + file_name_list[i] + "error");
			return error_message;
		}
	}
	D3D11_TEXTURE2D_DESC texElementDesc;
	srcTex[0]->GetDesc(&texElementDesc);
	//创建纹理数组
	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width = texElementDesc.Width;
	texArrayDesc.Height = texElementDesc.Height;
	texArrayDesc.MipLevels = texElementDesc.MipLevels;
	texArrayDesc.ArraySize = file_name_list.size();
	texArrayDesc.Format = texElementDesc.Format;
	texArrayDesc.SampleDesc.Count = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags = 0;
	texArrayDesc.MiscFlags = 0;

	ID3D11Texture2D* texArray = 0;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texArrayDesc, 0, &texArray);
	//填充纹理数组
	for (UINT texElement = 0; texElement < file_name_list.size(); ++texElement)
	{
		// for each mipmap level...
		for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
		{
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			HRESULT hr;
			hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D);
			if (FAILED(hr))
			{
				engine_basic::engine_fail_reason error_message(hr, "load model texture" + file_name_list[0] + "error");
				return error_message;
			}
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->UpdateSubresource(texArray, D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels), 0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);

			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Unmap(srcTex[texElement], mipLevel);
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = file_name_list.size();
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(texArray, &viewDesc, &test_resource);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "load model texture" + file_name_list[0] + "error");
		return error_message;
	}
	texArray->Release();
	for (UINT i = 0; i < file_name_list.size(); ++i)
		srcTex[i]->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason scene_test_square::init_clip_texture()
{
	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.Width = d3d_pancy_basic_singleton::GetInstance()->get_wind_width();
	dsDesc.Height = d3d_pancy_basic_singleton::GetInstance()->get_wind_height();
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;
	ID3D11Texture2D* depthStencilBuffer;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&dsDesc, 0, &depthStencilBuffer);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateDepthStencilView(depthStencilBuffer, 0, &clip_DSV);
	depthStencilBuffer->Release();
	//clip纹理
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.Width = d3d_pancy_basic_singleton::GetInstance()->get_wind_width();
	texDesc.Height = d3d_pancy_basic_singleton::GetInstance()->get_wind_height();
	texDesc.Format = DXGI_FORMAT_R32_UINT;

	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &clipTex0);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create clip map texture1 error");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(clipTex0, 0, &clip_SRV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create clip map SRV error");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(clipTex0, 0, &clip_RTV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create clip map RTV error");
		return error_message;
	}
	hr = CreateCPUaccessBuf(texDesc, &CPU_read_buffer);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create clip map CPUread tex error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void scene_test_square::save_bone_tree(skin_tree *bone_data)
{
	out_stream.write("*heaphead*", sizeof("*heaphead*"));
	out_stream.write((char *)bone_data, sizeof(*bone_data));
	if (bone_data->son != NULL)
	{
		save_bone_tree(bone_data->son);
	}
	out_stream.write("*heaptail*", sizeof("*heaptail*"));
	if (bone_data->brother != NULL)
	{
		save_bone_tree(bone_data->brother);
	}
}
void scene_test_square::read_bone_tree(skin_tree *now)
{
	char data[11];
	in_stream.read(reinterpret_cast<char*>(now), sizeof(*now));
	now->brother = NULL;
	now->son = NULL;
	in_stream.read(data, sizeof(data));
	while (strcmp(data, "*heaphead*") == 0) 
	{
		//入栈符号，代表子节点
		skin_tree *now_point = new skin_tree();
		read_bone_tree(now_point);
		now_point->brother = now->son;
		now->son = now_point;
		in_stream.read(data, sizeof(data));
	}
	/*
	if (strcmp(data, "*heaphead*") == 0)
	{
		//入栈符号，代表子节点
		now->son = new skin_tree();
		read_bone_tree(now->son);
	}
	if (strcmp(data, "*heaptail*") == 0)
	{
		if (strcmp(data, "*heaphead*") == 0)
		{
			//出栈符号，代表兄弟节点
			now->brother = new skin_tree();
			read_bone_tree(now->brother);
		}
	}
	*/

}
void scene_test_square::free_tree(skin_tree *now)
{
	if (now != NULL)
	{
		if (now->brother != NULL)
		{
			free_tree(now->brother);
		}
		if (now->son != NULL)
		{
			free_tree(now->son);
		}
		free(now);
	}
}
void scene_test_square::save_anim_data(animation_set *anim_data) 
{
	int anim_set_num = 0,animdata_num = 0;
	animation_set *now_anim = anim_data;
	while(now_anim != NULL)
	{
		anim_set_num++;
		out_stream.write(now_anim->animation_name, sizeof(now_anim->animation_name));
		out_stream.write(reinterpret_cast<char*>(&now_anim->animation_length), sizeof(now_anim->animation_length));
		out_stream.write(reinterpret_cast<char*>(&now_anim->number_animation), sizeof(now_anim->number_animation));
		animation_data *now_animdata = now_anim->head_animition;
		while (now_animdata != NULL)
		{
			animdata_num++;
			//骨骼名称
			out_stream.write(now_animdata->bone_name, sizeof(now_animdata->bone_name));
			//旋转四元数
			out_stream.write(reinterpret_cast<char*>(&now_animdata->number_rotation), sizeof(now_animdata->number_rotation));
			if (now_animdata->number_rotation != 0) 
			{
				out_stream.write(reinterpret_cast<char*>(now_animdata->rotation_key), now_animdata->number_rotation * sizeof(quaternion_animation));
			}
			//缩放向量
			out_stream.write(reinterpret_cast<char*>(&now_animdata->number_scaling), sizeof(now_animdata->number_scaling));
			if (now_animdata->number_scaling != 0)
			{
				out_stream.write(reinterpret_cast<char*>(now_animdata->scaling_key), now_animdata->number_scaling * sizeof(vector_animation));
			}
			//平移向量
			out_stream.write(reinterpret_cast<char*>(&now_animdata->number_translation), sizeof(now_animdata->number_translation));
			if (now_animdata->number_translation != 0)
			{
				out_stream.write(reinterpret_cast<char*>(now_animdata->translation_key), now_animdata->number_translation * sizeof(vector_animation));
			}
			/*
			//其他变换矩阵
			out_stream.write(reinterpret_cast<char*>(&now_animdata->number_transform), sizeof(now_animdata->number_transform));
			if (now_animdata->number_transform != 0)
			{
				out_stream.write(reinterpret_cast<char*>(&now_animdata->transform_key), now_animdata->number_transform * sizeof(matrix_animation));
			}
			*/
			now_animdata = now_animdata->next;
		}
		now_anim = now_anim->next;
	}
}
void scene_test_square::read_anim_data() 
{
	anim_read = new animation_set();
	animation_set *now_anim = anim_read;
	if (now_anim != NULL)
	{
		in_stream.read(now_anim->animation_name, sizeof(now_anim->animation_name));
		in_stream.read(reinterpret_cast<char*>(&now_anim->animation_length), sizeof(now_anim->animation_length));
		in_stream.read(reinterpret_cast<char*>(&now_anim->number_animation), sizeof(now_anim->number_animation));
		now_anim->head_animition = new animation_data();
		animation_data *now_animdata = now_anim->head_animition;
		for (int i = 0; i < now_anim->number_animation;++i)
		{
			//骨骼名称
			in_stream.read(now_animdata->bone_name, sizeof(now_animdata->bone_name));
			//旋转四元数
			in_stream.read(reinterpret_cast<char*>(&now_animdata->number_rotation), sizeof(now_animdata->number_rotation));
			if (now_animdata->number_rotation != 0)
			{
				now_animdata->rotation_key = new quaternion_animation[now_animdata->number_rotation];
				in_stream.read(reinterpret_cast<char*>(now_animdata->rotation_key), now_animdata->number_rotation * sizeof(quaternion_animation));
			}
			//缩放向量
			in_stream.read(reinterpret_cast<char*>(&now_animdata->number_scaling), sizeof(now_animdata->number_scaling));
			if (now_animdata->number_scaling != 0)
			{
				now_animdata->scaling_key = new vector_animation[now_animdata->number_scaling];
				in_stream.read(reinterpret_cast<char*>(now_animdata->scaling_key), now_animdata->number_scaling * sizeof(vector_animation));
			}
			//平移向量
			in_stream.read(reinterpret_cast<char*>(&now_animdata->number_translation), sizeof(now_animdata->number_translation));
			if (now_animdata->number_translation != 0)
			{
				now_animdata->translation_key = new vector_animation[now_animdata->number_translation];
				in_stream.read(reinterpret_cast<char*>(now_animdata->translation_key), now_animdata->number_translation * sizeof(vector_animation));
			}
			if (i != now_anim->number_animation - 1) 
			{
				now_animdata->next = new animation_data();
				now_animdata = now_animdata->next;
			}

		}
	}
}

engine_basic::engine_fail_reason scene_test_square::load_model(string filename, string tex_path)
{
	assimp_basic *rec = new model_reader_assimp<point_common>(filename.c_str(), tex_path.c_str());
	engine_basic::engine_fail_reason check_error = rec->model_create(false, 0, NULL);
	if_have_bone = false;
	if (rec->check_if_anim())
	{
		if_have_bone = true;
		rec->release();
		rec = new model_reader_skin(filename.c_str(), tex_path.c_str());
		check_error = rec->model_create(false, 0, NULL);
		model_reader_skin*rec1 = dynamic_cast<model_reader_skin*>(rec);
		rec1->update_mesh_offset();
	}
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	if (if_have_model == true)
	{
		mesh_model_need->release();
		//texture_deal->releae();
		//delete texture_deal;
		//texture_deal = NULL;
		rec_texture_packmap.clear();
		picture_namelist.clear();
	}
	mesh_model_need = rec;

	for (auto mat_data = pbr_list.begin(); mat_data != pbr_list.end(); ++mat_data)
	{
		if (mat_data._Ptr->metallic_name != "basic_metallic")
		{
			mat_data._Ptr->metallic->Release();
		}
		if (mat_data._Ptr->roughness_name != "basic_roughness")
		{
			mat_data._Ptr->roughness->Release();
		}
	}
	pbr_list.clear();

	for (int i = 0; i < rec->get_meshnum(); ++i)
	{
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~金属度~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		string metallic_name = tex_path + rec->get_mesh_name_bypart(i) + "_metallic.dds";
		//转换文件名为unicode
		size_t len = strlen(metallic_name.c_str()) + 1;
		size_t converted = 0;
		wchar_t *texture_name;
		texture_name = (wchar_t*)malloc(len*sizeof(wchar_t));
		mbstowcs_s(&converted, texture_name, len, metallic_name.c_str(), _TRUNCATE);
		pbr_material now_need;
		auto hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), texture_name, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &now_need.metallic);
		if (FAILED(hr_need))
		{
			now_need.metallic = mat_need_pbrbasic.metallic;
			now_need.metallic_name = mat_need_pbrbasic.metallic_name;
		}
		else
		{
			now_need.metallic_name = metallic_name;
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~粗糙度~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		string roughness_name = tex_path + rec->get_mesh_name_bypart(i) + "_roughness.dds";
		//转换文件名为unicode
		len = strlen(roughness_name.c_str()) + 1;
		texture_name = (wchar_t*)malloc(len*sizeof(wchar_t));
		mbstowcs_s(&converted, texture_name, len, roughness_name.c_str(), _TRUNCATE);
		hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), texture_name, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &now_need.roughness);
		if (FAILED(hr_need))
		{
			now_need.roughness = mat_need_pbrbasic.roughness;
			now_need.roughness_name = mat_need_pbrbasic.roughness_name;
		}
		else
		{
			now_need.roughness_name = roughness_name;
		}
		pbr_list.push_back(now_need);
	}
	if_have_model = true;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason scene_test_square::export_model(string filepath, string filename)
{
	if (data_point_need != NULL)
	{
		delete[] data_point_need;
		vertex_num = 0;
	}
	if (data_index_need != NULL)
	{
		delete[] data_index_need;
		index_num = 0;
	}
	if (model_out_test != NULL)
	{
		model_out_test->release();
		delete model_out_test;
		model_out_test = NULL;
	}
	picture_namelist.clear();
	rec_texture_packmap.clear();
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
			if (check_iferror.second)
			{
				picture_namelist.push_back(rec_need.texture_diffuse);
				ID3D11Texture2D *resource_rec;
				rec_need.tex_diffuse_resource->GetResource((ID3D11Resource**)&resource_rec);
				D3D11_TEXTURE2D_DESC desc_tex;
				resource_rec->GetDesc(&desc_tex);
				width_list[rec_texture_packmap.size() - 1] = desc_tex.Width;
				height_list[rec_texture_packmap.size() - 1] = desc_tex.Height;
				resource_rec->Release();
			}

		}
		//法线贴图
		if (rec_need.texture_normal_resource != NULL)
		{
			std::pair<string, ID3D11ShaderResourceView*> data_need(rec_need.texture_normal, rec_need.texture_normal_resource);
			auto check_iferror = rec_texture_packmap.insert(data_need);
			if (check_iferror.second)
			{
				picture_namelist.push_back(rec_need.texture_normal);
				ID3D11Texture2D *resource_rec;
				rec_need.texture_normal_resource->GetResource((ID3D11Resource**)&resource_rec);
				D3D11_TEXTURE2D_DESC desc_tex;
				resource_rec->GetDesc(&desc_tex);
				width_list[rec_texture_packmap.size() - 1] = desc_tex.Width;
				height_list[rec_texture_packmap.size() - 1] = desc_tex.Height;
				resource_rec->Release();
			}
		}
		//高光贴图
		if (rec_need.texture_specular_resource != NULL)
		{
			std::pair<string, ID3D11ShaderResourceView*> data_need(rec_need.texture_specular, rec_need.texture_specular_resource);
			auto check_iferror = rec_texture_packmap.insert(data_need);
			if (check_iferror.second)
			{
				picture_namelist.push_back(rec_need.texture_specular);
				ID3D11Texture2D *resource_rec;
				rec_need.texture_specular_resource->GetResource((ID3D11Resource**)&resource_rec);
				D3D11_TEXTURE2D_DESC desc_tex;
				resource_rec->GetDesc(&desc_tex);
				width_list[rec_texture_packmap.size() - 1] = desc_tex.Width;
				height_list[rec_texture_packmap.size() - 1] = desc_tex.Height;
				resource_rec->Release();
			}

		}
		/*
		新的贴图种类在这里填写
		*/
	}
	for (int i = 0; i < mesh_model_need->get_meshnum(); ++i)
	{
		//金属度贴图
		std::pair<string, ID3D11ShaderResourceView*> data_need_metallic(pbr_list[i].metallic_name, pbr_list[i].metallic);
		auto check_iferror = rec_texture_packmap.insert(data_need_metallic);
		if (check_iferror.second)
		{
			picture_namelist.push_back(pbr_list[i].metallic_name);
			ID3D11Texture2D *resource_rec;
			pbr_list[i].metallic->GetResource((ID3D11Resource**)&resource_rec);
			D3D11_TEXTURE2D_DESC desc_tex;
			resource_rec->GetDesc(&desc_tex);
			width_list[rec_texture_packmap.size() - 1] = desc_tex.Width;
			height_list[rec_texture_packmap.size() - 1] = desc_tex.Height;
			resource_rec->Release();
		}

		//粗糙度贴图
		std::pair<string, ID3D11ShaderResourceView*> data_need_roughness(pbr_list[i].roughness_name, pbr_list[i].roughness);
		check_iferror = rec_texture_packmap.insert(data_need_roughness);
		if (check_iferror.second)
		{
			picture_namelist.push_back(pbr_list[i].roughness_name);
			ID3D11Texture2D *resource_rec;
			pbr_list[i].roughness->GetResource((ID3D11Resource**)&resource_rec);
			D3D11_TEXTURE2D_DESC desc_tex;
			resource_rec->GetDesc(&desc_tex);
			width_list[rec_texture_packmap.size() - 1] = desc_tex.Width;
			height_list[rec_texture_packmap.size() - 1] = desc_tex.Height;
			resource_rec->Release();
		}

	}
	auto texture_deal = new texture_combine(rec_texture_packmap.size(), width_list, height_list, picture_type_width, picture_type_height);
	engine_basic::engine_fail_reason error_message = texture_deal->create();
	if (!error_message.check_if_failed())
	{
		return error_message;
	}

	//转换顶点数据
	if (!if_have_bone)
	{
		model_reader_assimp<point_common> *rec_data = dynamic_cast<model_reader_assimp<point_common>*>(mesh_model_need);
		rec_data->get_model_pack_num(vertex_num, index_num);
		data_point_need = new point_common[vertex_num];
		data_index_need = new UINT[index_num];
		rec_data->get_model_pack_data(data_point_need, data_index_need);
		//ifstream in_stream;
		out_stream.open(filename + ".pancymesh", ios::binary);
		out_stream.write((char *)&vertex_num, sizeof(vertex_num));
		out_stream.write((char *)&index_num, sizeof(index_num));
		int texture_num = texture_deal->get_texture_num();
		out_stream.write((char *)&texture_num, sizeof(texture_num));
		change_model_texcoord(texture_deal, data_point_need, vertex_num);
		for (int i = 0; i < index_num; ++i)
		{
			out_stream.write((char *)&data_index_need[i], sizeof(data_index_need[i]));
		}
		out_stream.close();
		model_out_test = new mesh_model<point_common>(data_point_need, data_index_need, vertex_num, index_num, false);
		error_message = model_out_test->create_object();
		if (!error_message.check_if_failed())
		{
			return error_message;
		}
	}
	else
	{
		model_reader_skin *rec_data = dynamic_cast<model_reader_skin*>(mesh_model_need);
		rec_data->get_model_pack_num(vertex_num, index_num);
		point_skincommon *data_point = new point_skincommon[vertex_num];
		data_index_need = new UINT[index_num];
		rec_data->get_model_pack_data(data_point, data_index_need);
		//ifstream in_stream;
		out_stream.open(filename + ".pancyskinmesh", ios::binary);
		out_stream.write((char *)&vertex_num, sizeof(vertex_num));
		out_stream.write((char *)&index_num, sizeof(index_num));
		int texture_num = texture_deal->get_texture_num();
		out_stream.write((char *)&texture_num, sizeof(texture_num));
		change_model_texcoord(texture_deal, data_point, vertex_num);
		for (int i = 0; i < index_num; ++i)
		{
			out_stream.write((char *)&data_index_need[i], sizeof(data_index_need[i]));
		}
		//~~~~~~~~~~~~~~存储骨骼信息~~~~~~~~~~~~~~~~~~~~~~
		out_stream.close();
		out_stream.open(filename + ".pancyskin", ios::binary);

		//偏移矩阵
		int bone_num_rec = rec_data->get_bone_num();
		out_stream.write((char *)(&bone_num_rec), sizeof(int));
		XMFLOAT4X4* offset_mat = rec_data->get_offset_mat();
		out_stream.write((char *)(offset_mat), bone_num_rec * sizeof(XMFLOAT4X4));

		//骨骼树
		save_bone_tree(rec_data->get_bone_tree());
		out_stream.close();
		//~~~~~~~~~~~~~~存储动画信息~~~~~~~~~~~~~~~~~~~~~~
		out_stream.open(filename + ".pancyanimation", ios::binary);
		animation_set *anim_data = rec_data->get_animation_data();
		save_anim_data(anim_data);
		out_stream.close();
		//~~~~~~~~~~~~~读取并检验骨骼信息~~~~~~~~~~~~~~~~~
		in_stream.open(filename + ".pancyskin", ios::binary);
		free_tree(bone_read);
		bone_read = NULL;
		//读取偏移矩阵
		int bone_num_need;
		in_stream.read(reinterpret_cast<char*>(&bone_num_need), sizeof(bone_num_need));
		in_stream.read(reinterpret_cast<char*>(offset_matrix), bone_num_need*sizeof(XMFLOAT4X4));
		//先读取第一个入栈符
		char data[11];
		in_stream.read(reinterpret_cast<char*>(data), sizeof(data));
		bone_read = new skin_tree();
		read_bone_tree(bone_read);
		in_stream.close();
		//~~~~~~~~~~~~~读取并检验动画信息~~~~~~~~~~~~~~~~~
		in_stream.open(filename + ".pancyanimation", ios::binary);
		read_anim_data();
		in_stream.close();
		/*
		model_out_test = new mesh_model<point_skincommon>(data_point, data_index_need, vertex_num, index_num, false);
		error_message = model_out_test->create_object();
		if (!error_message.check_if_failed())
		{
			return error_message;
		}
		*/
	}

	//存储纹理数据
	show_square(texture_deal);
	auto rendertarget = texture_deal->get_SRV_texarray();
	ID3D11Resource *resource_rec;
	rendertarget->GetResource(&resource_rec);
	std::vector<string> file_name_saving;
	out_stream.open(filename + ".pancymat");
	for (int i = 0; i < texture_deal->get_texture_num(); ++i)
	{
		stringstream stream;
		stream << i;
		auto string_temp = stream.str();

		string file_name_rec = filepath + "tex_pack_" + string_temp + ".dds";
		file_name_saving.push_back(file_name_rec);
		d3d_pancy_basic_singleton::GetInstance()->save_texture(resource_rec, file_name_rec, i);
		out_stream.write(file_name_rec.c_str(), file_name_rec.size() * sizeof(char));
		out_stream.write("\n", sizeof(char));
	}
	out_stream.close();

	read_texture_from_file(file_name_saving);

	resource_rec->Release();

	texture_deal->releae();
	delete texture_deal;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason scene_test_square::create()
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
	auto check_error = init_clip_texture();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	ZeroMemory(szPath, sizeof(szPath));
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = TEXT("dds纹理\0*.dds");
	ofn.lpstrFile = szPath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_DONTADDTORECENT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST;

	ZeroMemory(szPath_file, sizeof(szPath_file));
	ZeroMemory(&omodelfn, sizeof(OPENFILENAME));
	omodelfn.lStructSize = sizeof(OPENFILENAME);
	omodelfn.hwndOwner = NULL;
	omodelfn.lpstrFilter = TEXT("obj模型(*.obj)\0*.obj\0fbx模型(*.fbx)\0*.fbx\0所有文件\0*.*\0\0");
	omodelfn.lpstrFile = szPath_file;
	omodelfn.nMaxFile = MAX_PATH;
	omodelfn.lpstrInitialDir = NULL;
	omodelfn.Flags = OFN_DONTADDTORECENT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST;

	memset(szPath_save, 0, 256);
	memset(szPathcurrent_save, 0, 256);
	memset(&ooutfn, 0, sizeof(ooutfn));
	ooutfn.lStructSize = sizeof(ooutfn);
	ooutfn.hwndOwner = NULL;
	ooutfn.lpstrFilter = TEXT("pancystar engine模型(*.pancymesh)\0*.pancymesh");
	ooutfn.lpstrFileTitle = szPath_save;
	ooutfn.nMaxFileTitle = 256;
	ooutfn.lpstrFile = szPathcurrent_save;
	ooutfn.nMaxFile = 256;
	ooutfn.lpstrTitle = L"Select a file to open...";
	ooutfn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;



	check_error = ballmesh_need->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = mesh_need->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = picture_buf->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	HRESULT hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), L"floor.dds", 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &tex_floor);
	if (FAILED(hr_need))
	{
		engine_basic::engine_fail_reason error_message2(hr_need, "load model texture floor.dds error");
		return error_message2;
	}

	hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), L"pbr_basic\\black.dds", 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &mat_need_pbrbasic.metallic);
	if (FAILED(hr_need))
	{
		engine_basic::engine_fail_reason error_message2(hr_need, "load model texture ball\\Sphere002_metallic.dds error");
		return error_message2;
	}
	hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), L"pbr_basic\\white.dds", 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &mat_need_pbrbasic.roughness);
	if (FAILED(hr_need))
	{
		engine_basic::engine_fail_reason error_message2(hr_need, "load model texture ball\\Sphere002_roughness.dds error");
		return error_message2;
	}

	hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), L"UI\\metallic.dds", 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &metallic_choose_tex);
	if (FAILED(hr_need))
	{
		engine_basic::engine_fail_reason error_message2(hr_need, "load model UI\\metallic.dds error");
		return error_message2;
	}
	hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), L"UI\\roughness.dds", 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &roughness_choose_tex);
	if (FAILED(hr_need))
	{
		engine_basic::engine_fail_reason error_message2(hr_need, "load model UI\\roughness.dds error");
		return error_message2;
	}

	hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), L"UI\\readmodel.dds", 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &read_model_tex);
	if (FAILED(hr_need))
	{
		engine_basic::engine_fail_reason error_message2(hr_need, "load model UI\\readmodel.dds error");
		return error_message2;
	}
	hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), L"UI\\exportmodel.dds", 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &export_model_tex);
	if (FAILED(hr_need))
	{
		engine_basic::engine_fail_reason error_message2(hr_need, "load model UI\\exportmodel.dds error");
		return error_message2;
	}
	hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), L"Cubemap.dds", 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &cubemap_resource);
	if (FAILED(hr_need))
	{
		engine_basic::engine_fail_reason error_message2(hr_need, "load cubemap resource error");
		return error_message2;
	}


	ID3D11ShaderResourceView *read_model_tex;
	ID3D11ShaderResourceView *load_model_tex;
	//开启透明  
	D3D11_BLEND_DESC transDesc;
	//先创建一个混合状态的描述  
	transDesc.AlphaToCoverageEnable = false;
	transDesc.IndependentBlendEnable = false;
	transDesc.RenderTarget[0].BlendEnable = true;
	transDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr_need = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBlendState(&transDesc, &AlphaToCoverageBS);
	if (FAILED(hr_need))
	{
		engine_basic::engine_fail_reason error_message2(hr_need, "create alpha to coverage state error");
		return error_message2;
	}

	mat_need_pbrbasic.metallic_name = "basic_metallic";
	mat_need_pbrbasic.roughness_name = "basic_roughness";
	pbr_list.push_back(mat_need_pbrbasic);







	draw_brdfdata();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void scene_test_square::show_cube()
{
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 800.0f;
	viewPort.Height = 600.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 200.0f;
	if (if_have_model)
	{
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	}
	else
	{
		d3d_pancy_basic_singleton::GetInstance()->clear_basicrender_target(viewPort);
	}
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
	scal_world = XMMatrixScaling(1000, 2, 1000);
	//XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, 800.0f / 600.0f, 0.1f, 1000.f);
	XMFLOAT4X4 view_mat;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	rec_world = scal_world  *  trans_world * XMLoadFloat4x4(&view_mat) * proj;

	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	XMStoreFloat4x4(&final_matrix, rec_world);
	shader_need->set_trans_world(&world_matrix);
	shader_need->set_trans_all(&final_matrix);
	shader_need->set_tex_diffuse(tex_floor);
	ID3DX11EffectTechnique *teque_need;
	shader_need->get_technique(&teque_need, "light_tech");
	mesh_need->get_teque(teque_need);
	mesh_need->show_mesh();
}
void scene_test_square::show_sky()
{
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 800.0f;
	viewPort.Height = 600.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 200.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_sky_draw(check_error);
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	XMFLOAT4X4 final_matrix;
	rec += 0.001f;
	XMFLOAT3 view_pos;
	pancy_camera::get_instance()->get_view_position(&view_pos);
	trans_world = XMMatrixTranslation(view_pos.x, view_pos.y, view_pos.z);
	scal_world = XMMatrixScaling(400, 400, 400);
	//XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, 800.0f / 600.0f, 0.1f, 1000.f);
	XMFLOAT4X4 view_mat;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	rec_world = scal_world  *  trans_world * XMLoadFloat4x4(&view_mat) * proj;

	XMStoreFloat4x4(&world_matrix, scal_world *  trans_world);
	XMStoreFloat4x4(&final_matrix, rec_world);




	shader_need->set_trans_world(&world_matrix);
	shader_need->set_trans_all(&final_matrix);
	shader_need->set_tex_resource(cubemap_resource);

	shader_need->set_view_pos(view_pos);
	ID3DX11EffectTechnique *teque_need;
	shader_need->get_technique(&teque_need, "draw_sky");
	ballmesh_need->get_teque(teque_need);
	ballmesh_need->show_mesh();
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetState(NULL);
}
void scene_test_square::change_model_texcoord(texture_combine *texture_deal, point_common *vertex_need, int point_num)
{
	string lastname;
	texture_rebuild_data last_data;

	string last_normal_name;
	texture_rebuild_data last_normal_data;

	string last_metallic_name;
	texture_rebuild_data last_metallic_data;

	string last_roughness_name;
	texture_rebuild_data last_roughness_data;


	texture_rebuild_data texture_message;
	//freopen("testerror.dat","wb",stdout);
	for (int i = 0; i < point_num; ++i)
	{
		//暂时存储顶点的原始纹理坐标
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

		float tex_coord_x_rec = vertex_need[i].tex.x;
		float tex_coord_y_rec = vertex_need[i].tex.y;
		//先获取顶点对应的纹理名
		int id_pre_need = vertex_need[i].tex_id.x;
		material_list rec_need;
		mesh_model_need->get_texture(&rec_need, id_pre_need);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新漫反射纹理坐标~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
		//更新纹理坐标位置
		vertex_need[i].tex.x = tex_coord_x_rec * static_cast<float>(texture_message.pic_width) / static_cast<float>(picture_type_width);
		vertex_need[i].tex.y = tex_coord_y_rec * static_cast<float>(texture_message.pic_height) / static_cast<float>(picture_type_height);

		vertex_need[i].tex.x += static_cast<float>(texture_message.place_data.x_st) / static_cast<float>(picture_type_width);
		vertex_need[i].tex.y += static_cast<float>(texture_message.place_data.y_st) / static_cast<float>(picture_type_height);
		auto texture_message1 = texture_message;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新法线贴图纹理坐标~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		string name_normal = rec_need.texture_normal;
		if (rec_need.texture_normal_resource == NULL)
		{
			vertex_need[i].tex_id.y = -1;
		}
		else
		{
			if (name_normal == last_normal_name)
			{
				texture_message = last_normal_data;
			}
			else
			{
				//查找该纹理名对应的纹理ID
				int index_check;
				for (int i = 0; i < picture_namelist.size(); ++i)
				{
					if (picture_namelist[i] == name_normal)
					{
						index_check = i;
						last_normal_name = name_normal;
						break;
					}
				}
				//寻找其位置及ID
				texture_message = texture_deal->get_diffusetexture_data_byID(index_check);
				last_normal_data = texture_message;
			}
			vertex_need[i].tex_id.y = texture_message.now_index;
			//更新纹理坐标位置
			vertex_need[i].tex.z = tex_coord_x_rec * static_cast<float>(texture_message.pic_width) / static_cast<float>(picture_type_width);
			vertex_need[i].tex.w = tex_coord_y_rec * static_cast<float>(texture_message.pic_height) / static_cast<float>(picture_type_height);
			vertex_need[i].tex.z += static_cast<float>(texture_message.place_data.x_st) / static_cast<float>(picture_type_width);
			vertex_need[i].tex.w += static_cast<float>(texture_message.place_data.y_st) / static_cast<float>(picture_type_height);
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新金属度贴图纹理坐标~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		string name_metallic = pbr_list[id_pre_need].metallic_name;
		if (name_metallic == last_metallic_name)
		{
			texture_message = last_metallic_data;
		}
		else
		{
			//查找该纹理名对应的纹理ID
			int index_check;
			for (int i = 0; i < picture_namelist.size(); ++i)
			{
				if (picture_namelist[i] == name_metallic)
				{
					index_check = i;
					last_metallic_name = name_metallic;
					break;
				}
			}
			//寻找其位置及ID
			texture_message = texture_deal->get_diffusetexture_data_byID(index_check);
			last_metallic_data = texture_message;
		}
		vertex_need[i].tex_id.z = texture_message.now_index;
		//更新纹理坐标位置
		vertex_need[i].tex2.x = tex_coord_x_rec * static_cast<float>(texture_message.pic_width) / static_cast<float>(picture_type_width);
		vertex_need[i].tex2.y = tex_coord_y_rec * static_cast<float>(texture_message.pic_height) / static_cast<float>(picture_type_height);
		vertex_need[i].tex2.x += static_cast<float>(texture_message.place_data.x_st) / static_cast<float>(picture_type_width);
		vertex_need[i].tex2.y += static_cast<float>(texture_message.place_data.y_st) / static_cast<float>(picture_type_height);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新粗糙度贴图纹理坐标~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		string name_roughness = pbr_list[id_pre_need].roughness_name;
		if (name_roughness == last_roughness_name)
		{
			texture_message = last_roughness_data;
		}
		else
		{
			//查找该纹理名对应的纹理ID
			int index_check;
			for (int i = 0; i < picture_namelist.size(); ++i)
			{
				if (picture_namelist[i] == name_roughness)
				{
					index_check = i;
					last_roughness_name = name_roughness;
					break;
				}
			}
			//寻找其位置及ID
			texture_message = texture_deal->get_diffusetexture_data_byID(index_check);
			last_roughness_data = texture_message;
		}
		vertex_need[i].tex_id.w = texture_message.now_index;
		//更新纹理坐标位置
		vertex_need[i].tex2.z = tex_coord_x_rec * static_cast<float>(texture_message.pic_width) / static_cast<float>(picture_type_width);
		vertex_need[i].tex2.w = tex_coord_y_rec * static_cast<float>(texture_message.pic_height) / static_cast<float>(picture_type_height);
		vertex_need[i].tex2.z += static_cast<float>(texture_message.place_data.x_st) / static_cast<float>(picture_type_width);
		vertex_need[i].tex2.w += static_cast<float>(texture_message.place_data.y_st) / static_cast<float>(picture_type_height);


		out_stream.write((char *)&vertex_need[i], sizeof(vertex_need[i]));
		/*
		out_stream.write((char *)&vertex_need[i].position.x, sizeof(vertex_need[i].position.x));
		out_stream.write((char *)&vertex_need[i].position.y, sizeof(vertex_need[i].position.y));
		out_stream.write((char *)&vertex_need[i].position.z, sizeof(vertex_need[i].position.z));

		out_stream.write((char *)&vertex_need[i].normal.x, sizeof(vertex_need[i].normal.x));
		out_stream.write((char *)&vertex_need[i].normal.y, sizeof(vertex_need[i].normal.y));
		out_stream.write((char *)&vertex_need[i].normal.z, sizeof(vertex_need[i].normal.z));

		out_stream.write((char *)&vertex_need[i].tangent.x, sizeof(vertex_need[i].tangent.x));
		out_stream.write((char *)&vertex_need[i].tangent.y, sizeof(vertex_need[i].tangent.y));
		out_stream.write((char *)&vertex_need[i].tangent.z, sizeof(vertex_need[i].tangent.z));

		out_stream.write((char *)&vertex_need[i].tex_id.x, sizeof(vertex_need[i].tex_id.x));
		out_stream.write((char *)&vertex_need[i].tex_id.y, sizeof(vertex_need[i].tex_id.y));
		out_stream.write((char *)&vertex_need[i].tex_id.z, sizeof(vertex_need[i].tex_id.z));
		out_stream.write((char *)&vertex_need[i].tex_id.w, sizeof(vertex_need[i].tex_id.z));

		out_stream.write((char *)&vertex_need[i].tex.x, sizeof(vertex_need[i].tex.x));
		out_stream.write((char *)&vertex_need[i].tex.y, sizeof(vertex_need[i].tex.y));
		out_stream.write((char *)&vertex_need[i].tex.z, sizeof(vertex_need[i].tex.z));
		out_stream.write((char *)&vertex_need[i].tex.w, sizeof(vertex_need[i].tex.z));

		out_stream.write((char *)&vertex_need[i].tex2.x, sizeof(vertex_need[i].tex2.x));
		out_stream.write((char *)&vertex_need[i].tex2.y, sizeof(vertex_need[i].tex2.y));
		out_stream.write((char *)&vertex_need[i].tex2.z, sizeof(vertex_need[i].tex2.z));
		out_stream.write((char *)&vertex_need[i].tex2.w, sizeof(vertex_need[i].tex2.z));
		*/
		/*
		printf("%.4f %.4f %.4f ", vertex_need[i].position.x, vertex_need[i].position.y, vertex_need[i].position.z);
		printf("%.4f %.4f %.4f ", vertex_need[i].normal.x, vertex_need[i].normal.y, vertex_need[i].normal.z);
		printf("%.4f %.4f %.4f ", vertex_need[i].tangent.x, vertex_need[i].tangent.y, vertex_need[i].tangent.z);
		printf("%d %d %d %d ", vertex_need[i].tex_id.x, vertex_need[i].tex_id.y, vertex_need[i].tex_id.z, vertex_need[i].tex_id.w);
		printf("%.4f %.4f %.4f %.4f ", vertex_need[i].tex.x, vertex_need[i].tex.y, vertex_need[i].tex.z, vertex_need[i].tex.w);
		printf("%.4f %.4f %.4f %.4f ", vertex_need[i].tex2.x, vertex_need[i].tex2.y, vertex_need[i].tex2.z, vertex_need[i].tex2.w);
		*/
	}
}
void scene_test_square::change_model_texcoord(texture_combine *texture_deal, point_skincommon *vertex_need, int point_num)
{
	string lastname;
	texture_rebuild_data last_data;

	string last_normal_name;
	texture_rebuild_data last_normal_data;

	string last_metallic_name;
	texture_rebuild_data last_metallic_data;

	string last_roughness_name;
	texture_rebuild_data last_roughness_data;


	texture_rebuild_data texture_message;
	//freopen("testerror.dat","wb",stdout);
	for (int i = 0; i < point_num; ++i)
	{
		//暂时存储顶点的原始纹理坐标
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

		float tex_coord_x_rec = vertex_need[i].tex.x;
		float tex_coord_y_rec = vertex_need[i].tex.y;
		//先获取顶点对应的纹理名
		int id_pre_need = vertex_need[i].tex_id.x;
		material_list rec_need;
		mesh_model_need->get_texture(&rec_need, id_pre_need);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新漫反射纹理坐标~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
		//更新纹理坐标位置
		vertex_need[i].tex.x = tex_coord_x_rec * static_cast<float>(texture_message.pic_width) / static_cast<float>(picture_type_width);
		vertex_need[i].tex.y = tex_coord_y_rec * static_cast<float>(texture_message.pic_height) / static_cast<float>(picture_type_height);

		vertex_need[i].tex.x += static_cast<float>(texture_message.place_data.x_st) / static_cast<float>(picture_type_width);
		vertex_need[i].tex.y += static_cast<float>(texture_message.place_data.y_st) / static_cast<float>(picture_type_height);
		auto texture_message1 = texture_message;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新法线贴图纹理坐标~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		string name_normal = rec_need.texture_normal;
		if (rec_need.texture_normal_resource == NULL)
		{
			vertex_need[i].tex_id.y = -1;
		}
		else
		{
			if (name_normal == last_normal_name)
			{
				texture_message = last_normal_data;
			}
			else
			{
				//查找该纹理名对应的纹理ID
				int index_check;
				for (int i = 0; i < picture_namelist.size(); ++i)
				{
					if (picture_namelist[i] == name_normal)
					{
						index_check = i;
						last_normal_name = name_normal;
						break;
					}
				}
				//寻找其位置及ID
				texture_message = texture_deal->get_diffusetexture_data_byID(index_check);
				last_normal_data = texture_message;
			}
			vertex_need[i].tex_id.y = texture_message.now_index;
			//更新纹理坐标位置
			vertex_need[i].tex.z = tex_coord_x_rec * static_cast<float>(texture_message.pic_width) / static_cast<float>(picture_type_width);
			vertex_need[i].tex.w = tex_coord_y_rec * static_cast<float>(texture_message.pic_height) / static_cast<float>(picture_type_height);
			vertex_need[i].tex.z += static_cast<float>(texture_message.place_data.x_st) / static_cast<float>(picture_type_width);
			vertex_need[i].tex.w += static_cast<float>(texture_message.place_data.y_st) / static_cast<float>(picture_type_height);
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新金属度贴图纹理坐标~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		string name_metallic = pbr_list[id_pre_need].metallic_name;

		if (name_metallic == last_metallic_name)
		{
			texture_message = last_metallic_data;
		}
		else
		{
			//查找该纹理名对应的纹理ID
			int index_check;
			for (int i = 0; i < picture_namelist.size(); ++i)
			{
				if (picture_namelist[i] == name_metallic)
				{
					index_check = i;
					last_metallic_name = name_metallic;
					break;
				}
			}
			//寻找其位置及ID
			texture_message = texture_deal->get_diffusetexture_data_byID(index_check);
			last_metallic_data = texture_message;
		}
		vertex_need[i].tex_id.z = texture_message.now_index;
		//更新纹理坐标位置
		vertex_need[i].tex2.x = tex_coord_x_rec * static_cast<float>(texture_message.pic_width) / static_cast<float>(picture_type_width);
		vertex_need[i].tex2.y = tex_coord_y_rec * static_cast<float>(texture_message.pic_height) / static_cast<float>(picture_type_height);
		vertex_need[i].tex2.x += static_cast<float>(texture_message.place_data.x_st) / static_cast<float>(picture_type_width);
		vertex_need[i].tex2.y += static_cast<float>(texture_message.place_data.y_st) / static_cast<float>(picture_type_height);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新粗糙度贴图纹理坐标~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		string name_roughness = pbr_list[id_pre_need].roughness_name;
		if (name_roughness == last_roughness_name)
		{
			texture_message = last_roughness_data;
		}
		else
		{
			//查找该纹理名对应的纹理ID
			int index_check;
			for (int i = 0; i < picture_namelist.size(); ++i)
			{
				if (picture_namelist[i] == name_roughness)
				{
					index_check = i;
					last_roughness_name = name_roughness;
					break;
				}
			}
			//寻找其位置及ID
			texture_message = texture_deal->get_diffusetexture_data_byID(index_check);
			last_roughness_data = texture_message;
		}
		vertex_need[i].tex_id.w = texture_message.now_index;
		//更新纹理坐标位置
		vertex_need[i].tex2.z = tex_coord_x_rec * static_cast<float>(texture_message.pic_width) / static_cast<float>(picture_type_width);
		vertex_need[i].tex2.w = tex_coord_y_rec * static_cast<float>(texture_message.pic_height) / static_cast<float>(picture_type_height);
		vertex_need[i].tex2.z += static_cast<float>(texture_message.place_data.x_st) / static_cast<float>(picture_type_width);
		vertex_need[i].tex2.w += static_cast<float>(texture_message.place_data.y_st) / static_cast<float>(picture_type_height);


		out_stream.write((char *)&vertex_need[i], sizeof(vertex_need[i]));
		/*
		out_stream.write((char *)&vertex_need[i].position.x, sizeof(vertex_need[i].position.x));
		out_stream.write((char *)&vertex_need[i].position.y, sizeof(vertex_need[i].position.y));
		out_stream.write((char *)&vertex_need[i].position.z, sizeof(vertex_need[i].position.z));

		out_stream.write((char *)&vertex_need[i].normal.x, sizeof(vertex_need[i].normal.x));
		out_stream.write((char *)&vertex_need[i].normal.y, sizeof(vertex_need[i].normal.y));
		out_stream.write((char *)&vertex_need[i].normal.z, sizeof(vertex_need[i].normal.z));

		out_stream.write((char *)&vertex_need[i].tangent.x, sizeof(vertex_need[i].tangent.x));
		out_stream.write((char *)&vertex_need[i].tangent.y, sizeof(vertex_need[i].tangent.y));
		out_stream.write((char *)&vertex_need[i].tangent.z, sizeof(vertex_need[i].tangent.z));

		out_stream.write((char *)&vertex_need[i].tex_id.x, sizeof(vertex_need[i].tex_id.x));
		out_stream.write((char *)&vertex_need[i].tex_id.y, sizeof(vertex_need[i].tex_id.y));
		out_stream.write((char *)&vertex_need[i].tex_id.z, sizeof(vertex_need[i].tex_id.z));
		out_stream.write((char *)&vertex_need[i].tex_id.w, sizeof(vertex_need[i].tex_id.z));

		out_stream.write((char *)&vertex_need[i].tex.x, sizeof(vertex_need[i].tex.x));
		out_stream.write((char *)&vertex_need[i].tex.y, sizeof(vertex_need[i].tex.y));
		out_stream.write((char *)&vertex_need[i].tex.z, sizeof(vertex_need[i].tex.z));
		out_stream.write((char *)&vertex_need[i].tex.w, sizeof(vertex_need[i].tex.z));

		out_stream.write((char *)&vertex_need[i].tex2.x, sizeof(vertex_need[i].tex2.x));
		out_stream.write((char *)&vertex_need[i].tex2.y, sizeof(vertex_need[i].tex2.y));
		out_stream.write((char *)&vertex_need[i].tex2.z, sizeof(vertex_need[i].tex2.z));
		out_stream.write((char *)&vertex_need[i].tex2.w, sizeof(vertex_need[i].tex2.z));
		*/
		/*
		printf("%.4f %.4f %.4f ", vertex_need[i].position.x, vertex_need[i].position.y, vertex_need[i].position.z);
		printf("%.4f %.4f %.4f ", vertex_need[i].normal.x, vertex_need[i].normal.y, vertex_need[i].normal.z);
		printf("%.4f %.4f %.4f ", vertex_need[i].tangent.x, vertex_need[i].tangent.y, vertex_need[i].tangent.z);
		printf("%d %d %d %d ", vertex_need[i].tex_id.x, vertex_need[i].tex_id.y, vertex_need[i].tex_id.z, vertex_need[i].tex_id.w);
		printf("%.4f %.4f %.4f %.4f ", vertex_need[i].tex.x, vertex_need[i].tex.y, vertex_need[i].tex.z, vertex_need[i].tex.w);
		printf("%.4f %.4f %.4f %.4f ", vertex_need[i].tex2.x, vertex_need[i].tex2.y, vertex_need[i].tex2.z, vertex_need[i].tex2.w);
		*/
	}
}
void scene_test_square::display()
{
	//draw_brdfdata();
	if (if_have_model)
	{
		find_model_clip();
		show_model();
	}
	show_cube();
	show_sky();
	if (now_show_part != 99999)
	{
		show_pbr_metallic(pbr_list[now_show_part]);
		show_pbr_roughness(pbr_list[now_show_part]);
	}
	show_metallic_choose();
	show_roughness_choose();
	show_read_mdoel();
	show_write_mdoel();
	//show_square();
	//show_model_single();
}
void scene_test_square::draw_brdfdata()
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
void scene_test_square::show_model()
{

	D3D11_VIEWPORT viewPort;
	viewPort.Width = 800.0f;
	viewPort.Height = 600.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 200.0f;
	//d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);

	d3d_pancy_basic_singleton::GetInstance()->clear_basicrender_target(viewPort);
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_virtual_light(check_error);
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	XMFLOAT4X4 final_matrix;
	rec += 0.001f;
	trans_world = XMMatrixTranslation(0.0, 10.0, 0.0);
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
	XMFLOAT3 view_pos;
	pancy_camera::get_instance()->get_view_position(&view_pos);
	shader_need->set_view_pos(view_pos);
	shader_need->set_tex_environment(cubemap_resource);
	shader_need->set_tex_brdfluv(brdf_pic);
	for (int i = 0; i < mesh_model_need->get_meshnum(); ++i)
	{
		material_list rec_need;
		mesh_model_need->get_texture(&rec_need, i);
		shader_need->set_tex_diffuse(rec_need.tex_diffuse_resource);
		shader_need->set_tex_metallic(pbr_list[i].metallic);
		shader_need->set_tex_roughness(pbr_list[i].roughness);

		if (!if_have_bone)
		{
			shader_need->get_technique(&teque_need, "light_tech_pbr");
			mesh_model_need->get_technique(teque_need);
			mesh_model_need->draw_part(i);
		}
		else
		{
			auto skin_ptr = dynamic_cast<model_reader_skin*>(mesh_model_need);
			skin_ptr->update_animation(0.002f);

			shader_need->set_bone_matrix(skin_ptr->get_bone_matrix(), skin_ptr->get_bone_num());
			//设置顶点声明
			D3D11_INPUT_ELEMENT_DESC rec_inputdesc[] =
			{
				//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
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
			shader_need->get_technique(rec_inputdesc, num_member, &teque_need, "light_tech_pbr_withbone");
			mesh_model_need->get_technique(teque_need);
			mesh_model_need->draw_part(i);
		}

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
	shader_need->set_tex_diffuse_array(test_resource);
	model_out_test->show_mesh();
}
void scene_test_square::find_model_clip()
{
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 800.0f;
	viewPort.Height = 600.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 200.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	auto point = d3d_pancy_basic_singleton::GetInstance()->update_mouse();
	if (if_click)
	{
		if (point.x > viewPort.TopLeftX && point.x < viewPort.TopLeftX + viewPort.Width && point.y > viewPort.TopLeftY && point.y < viewPort.TopLeftY + viewPort.Height)
		{
			ID3D11RenderTargetView* renderTargets[1] = { clip_RTV };
			float clearColor[] = { 99999.0f, 0.0f, 0.0f, 1.0f };
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTargets, clip_DSV);
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(clip_RTV, clearColor);
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearDepthStencilView(clip_DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
			//contex_pancy->RSSetViewports(1, &viewPort);
			engine_basic::engine_fail_reason check_error;
			auto shader_need = shader_control::GetInstance()->get_shader_find_clip(check_error);
			XMMATRIX trans_world;
			XMMATRIX scal_world;
			XMMATRIX rotation_world;
			XMMATRIX rec_world;
			XMFLOAT4X4 world_matrix;
			XMFLOAT4X4 final_matrix;
			rec += 0.001f;
			trans_world = XMMatrixTranslation(0.0, 10.0, 0.0);
			scal_world = XMMatrixScaling(1, 1, 1);
			//XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, 800.0f / 600.0f, 0.1f, 1000.f);
			XMFLOAT4X4 view_mat;
			pancy_camera::get_instance()->count_view_matrix(&view_mat);
			XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
			rec_world = scal_world  *  trans_world * XMLoadFloat4x4(&view_mat) * proj;
			XMStoreFloat4x4(&final_matrix, rec_world);
			shader_need->set_trans_all(&final_matrix);
			ID3DX11EffectTechnique *teque_need;
			for (int i = 0; i < mesh_model_need->get_meshnum(); ++i)
			{
				shader_need->set_part_ID(XMUINT4(i, 0, 0, 0));
				shader_need->get_technique(&teque_need, "draw_clipmap");
				mesh_model_need->get_technique(teque_need);
				mesh_model_need->draw_part(i);
			}
			d3d_pancy_basic_singleton::GetInstance()->restore_render_target();

			CreateAndCopyToDebugBuf(CPU_read_buffer, clipTex0);
			D3D11_TEXTURE2D_DESC texElementDesc;
			CPU_read_buffer->GetDesc(&texElementDesc);

			unsigned int rec_answer;
			for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
			{
				D3D11_MAPPED_SUBRESOURCE mappedTex2D;
				HRESULT hr;
				hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Map(CPU_read_buffer, mipLevel, D3D11_MAP_READ, 0, &mappedTex2D);
				unsigned int* rec = static_cast<unsigned int*>(mappedTex2D.pData) + (mappedTex2D.RowPitch / 4) * point.y;
				rec_answer = rec[point.x];
				//contex_pancy->UpdateSubresource(texArray, D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels), 0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);
				d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Unmap(CPU_read_buffer, mipLevel);
			}
			now_show_part = rec_answer;
		}
	}
}
void scene_test_square::show_square(texture_combine *texture_deal)
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
	if (user_input->check_mouseDown(0))
	{
		if_button_down = true;
		if_click = false;
	}
	else if (if_button_down)
	{
		if_button_down = false;
		if_click = true;
	}
	else
	{
		if_click = false;
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
	mat_need_pbrbasic.metallic->Release();
	mat_need_pbrbasic.roughness->Release();
	for (auto data = pbr_list.begin(); data != pbr_list.end(); ++data)
	{
		if (data._Ptr->metallic_name != "basic_metallic")
		{
			data._Ptr->metallic->Release();
		}
		if (data._Ptr->roughness_name != "basic_roughness")
		{
			data._Ptr->roughness->Release();
		}
	}
	tex_floor->Release();
	if (model_out_test != NULL)
	{
		model_out_test->release();
	}
	//	if (texture_deal != NULL) 
	//	{
	//		texture_deal->releae();
	//	}
	if (test_resource != NULL)
	{
		test_resource->Release();
	}
	metallic_choose_tex->Release();
	roughness_choose_tex->Release();
	cubemap_resource->Release();
	ballmesh_need->release();
	AlphaToCoverageBS->Release();
	read_model_tex->Release();
	export_model_tex->Release();
	brdf_pic->Release();
	brdf_target->Release();
	clipTex0->Release();
	CPU_read_buffer->Release();
	clip_SRV->Release();
	clip_RTV->Release();
	clip_DSV->Release();
}
void scene_test_square::show_pbr_metallic(pbr_material mat_in)
{
	//修改视口大小
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 250;
	viewPort.Height = 250;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 850.0f;
	viewPort.TopLeftY = 120.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	engine_basic::engine_fail_reason check_error;
	auto shader_picture = shader_control::GetInstance()->get_shader_picture(check_error);
	shader_picture->set_UI_position(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	shader_picture->set_UI_scal(XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f));
	shader_picture->set_tex_color_resource(mat_in.metallic);
	ID3DX11EffectTechnique *teque_need;
	shader_picture->get_technique(&teque_need, "draw_ui");
	picture_buf->get_teque(teque_need);
	picture_buf->show_mesh();
}
void scene_test_square::show_pbr_roughness(pbr_material mat_in)
{
	//修改视口大小
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 250;
	viewPort.Height = 250;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 850.0f;
	viewPort.TopLeftY = 480.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	engine_basic::engine_fail_reason check_error;
	auto shader_picture = shader_control::GetInstance()->get_shader_picture(check_error);
	shader_picture->set_UI_position(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	shader_picture->set_UI_scal(XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f));
	shader_picture->set_tex_color_resource(mat_in.roughness);
	ID3DX11EffectTechnique *teque_need;
	shader_picture->get_technique(&teque_need, "draw_ui");
	picture_buf->get_teque(teque_need);
	picture_buf->show_mesh();
}


void scene_test_square::show_metallic_choose()
{
	//设置渲染模式
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(AlphaToCoverageBS, blendFactor, 0xffffffff);
	//修改视口大小
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 150;
	viewPort.Height = 50;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 900.0f;
	viewPort.TopLeftY = 50.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	engine_basic::engine_fail_reason check_error;
	auto shader_picture = shader_control::GetInstance()->get_shader_picture(check_error);
	auto point = d3d_pancy_basic_singleton::GetInstance()->update_mouse();
	if (point.x > viewPort.TopLeftX && point.x < viewPort.TopLeftX + viewPort.Width && point.y > viewPort.TopLeftY && point.y < viewPort.TopLeftY + viewPort.Height)
	{
		shader_picture->set_UI_position(XMFLOAT4(0.0f, 0.5f, 0.0f, 0.0f));
		if (if_click && now_show_part != 99999)
		{
			//保存当前路径
			DWORD length_currentdir;
			TCHAR current_dir_path[MAX_PATH];
			ZeroMemory(current_dir_path, sizeof(current_dir_path));
			GetCurrentDirectory(MAX_PATH, current_dir_path);
			//打开文件
			GetOpenFileName(&ofn);
			ID3D11ShaderResourceView *RSV_check;
			auto hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), szPath, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &RSV_check);
			if (FAILED(hr_need))
			{
				MessageBox(0, L"load tex error", L"error", MB_OK);
			}
			else
			{
				if (pbr_list[now_show_part].metallic_name != "basic_metallic")
				{
					pbr_list[now_show_part].metallic->Release();
				}
				DWORD dwMinSize = 0;
				LPSTR lpszStr = NULL;
				dwMinSize = WideCharToMultiByte(CP_OEMCP, NULL, szPath, -1, NULL, 0, NULL, FALSE);
				lpszStr = new char[dwMinSize];
				WideCharToMultiByte(CP_OEMCP, NULL, szPath, -1, lpszStr, dwMinSize, NULL, FALSE);
				string filename_str = lpszStr;
				pbr_list[now_show_part].metallic_name = filename_str;
				pbr_list[now_show_part].metallic = RSV_check;

			}
			//还原当前路径
			SetCurrentDirectory(current_dir_path);
		}
	}
	else
	{
		shader_picture->set_UI_position(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	}
	shader_picture->set_UI_scal(XMFLOAT4(1.0f, 0.5f, 0.0f, 0.0f));
	shader_picture->set_tex_color_resource(metallic_choose_tex);
	ID3DX11EffectTechnique *teque_need;
	shader_picture->get_technique(&teque_need, "draw_ui_move");
	picture_buf->get_teque(teque_need);
	picture_buf->show_mesh();
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(NULL, blendFactor, 0xffffffff);
}
void scene_test_square::show_roughness_choose()
{
	//设置渲染模式
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(AlphaToCoverageBS, blendFactor, 0xffffffff);
	//修改视口大小
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 150;
	viewPort.Height = 50;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 900.0f;
	viewPort.TopLeftY = 400.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	engine_basic::engine_fail_reason check_error;
	auto shader_picture = shader_control::GetInstance()->get_shader_picture(check_error);
	auto point = d3d_pancy_basic_singleton::GetInstance()->update_mouse();
	if (point.x > viewPort.TopLeftX && point.x < viewPort.TopLeftX + viewPort.Width && point.y > viewPort.TopLeftY && point.y < viewPort.TopLeftY + viewPort.Height)
	{
		shader_picture->set_UI_position(XMFLOAT4(0.0f, 0.5f, 0.0f, 0.0f));
		if (if_click && now_show_part != 99999)
		{
			//保存当前路径
			DWORD length_currentdir;
			TCHAR current_dir_path[MAX_PATH];
			ZeroMemory(current_dir_path, sizeof(current_dir_path));
			GetCurrentDirectory(MAX_PATH, current_dir_path);
			//打开文件
			GetOpenFileName(&ofn);
			ID3D11ShaderResourceView *RSV_check;
			auto hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), szPath, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &RSV_check);
			if (FAILED(hr_need))
			{
				MessageBox(0, L"load tex error", L"error", MB_OK);
			}
			else
			{
				if (pbr_list[now_show_part].roughness_name != "basic_roughness")
				{
					pbr_list[now_show_part].roughness->Release();
				}
				DWORD dwMinSize = 0;
				LPSTR lpszStr = NULL;
				dwMinSize = WideCharToMultiByte(CP_OEMCP, NULL, szPath, -1, NULL, 0, NULL, FALSE);
				lpszStr = new char[dwMinSize];
				WideCharToMultiByte(CP_OEMCP, NULL, szPath, -1, lpszStr, dwMinSize, NULL, FALSE);
				string filename_str = lpszStr;
				pbr_list[now_show_part].roughness_name = filename_str;
				pbr_list[now_show_part].roughness = RSV_check;
			}
			//还原当前路径
			SetCurrentDirectory(current_dir_path);
		}
	}
	else
	{
		shader_picture->set_UI_position(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	}
	shader_picture->set_UI_scal(XMFLOAT4(1.0f, 0.5f, 0.0f, 0.0f));
	shader_picture->set_tex_color_resource(roughness_choose_tex);
	ID3DX11EffectTechnique *teque_need;
	shader_picture->get_technique(&teque_need, "draw_ui_move");
	picture_buf->get_teque(teque_need);
	picture_buf->show_mesh();
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(NULL, blendFactor, 0xffffffff);
}
string get_file_path(string filename)
{
	int end = -1;
	string out;
	for (int i = filename.size() - 1; i >= 0; --i)
	{
		if (filename[i] == '\\')
		{
			end = i;
			break;
		}
	}
	for (int i = 0; i <= end; ++i)
	{
		out += filename[i];
	}
	return out;
}
void scene_test_square::show_read_mdoel()
{
	//设置渲染模式
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(AlphaToCoverageBS, blendFactor, 0xffffffff);
	//修改视口大小
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 150;
	viewPort.Height = 50;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 100.0f;
	viewPort.TopLeftY = 50.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	engine_basic::engine_fail_reason check_error;
	auto shader_picture = shader_control::GetInstance()->get_shader_picture(check_error);
	auto point = d3d_pancy_basic_singleton::GetInstance()->update_mouse();
	if (point.x > viewPort.TopLeftX && point.x < viewPort.TopLeftX + viewPort.Width && point.y > viewPort.TopLeftY && point.y < viewPort.TopLeftY + viewPort.Height)
	{
		shader_picture->set_UI_position(XMFLOAT4(0.0f, 0.5f, 0.0f, 0.0f));
		if (if_click)
		{
			//保存当前路径
			DWORD length_currentdir;
			TCHAR current_dir_path[MAX_PATH];
			ZeroMemory(current_dir_path, sizeof(current_dir_path));
			GetCurrentDirectory(MAX_PATH, current_dir_path);
			//打开文件
			GetOpenFileName(&omodelfn);
			DWORD dwMinSize = 0;
			LPSTR lpszStr = NULL;
			dwMinSize = WideCharToMultiByte(CP_OEMCP, NULL, szPath_file, -1, NULL, 0, NULL, FALSE);

			lpszStr = new char[dwMinSize];
			WideCharToMultiByte(CP_OEMCP, NULL, szPath_file, -1, lpszStr, dwMinSize, NULL, FALSE);

			string filename_str = lpszStr;
			auto path = get_file_path(filename_str);
			load_model(filename_str, path);
			//还原当前路径
			SetCurrentDirectory(current_dir_path);
		}
	}
	else
	{
		shader_picture->set_UI_position(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	}
	shader_picture->set_UI_scal(XMFLOAT4(1.0f, 0.5f, 0.0f, 0.0f));
	shader_picture->set_tex_color_resource(read_model_tex);
	ID3DX11EffectTechnique *teque_need;
	shader_picture->get_technique(&teque_need, "draw_ui_move");
	picture_buf->get_teque(teque_need);
	picture_buf->show_mesh();
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(NULL, blendFactor, 0xffffffff);
}
void scene_test_square::show_write_mdoel()
{
	//设置渲染模式
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(AlphaToCoverageBS, blendFactor, 0xffffffff);
	//修改视口大小
	D3D11_VIEWPORT viewPort;
	viewPort.Width = 150;
	viewPort.Height = 50;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 400.0f;
	viewPort.TopLeftY = 50.0f;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &viewPort);
	engine_basic::engine_fail_reason check_error;
	auto shader_picture = shader_control::GetInstance()->get_shader_picture(check_error);
	auto point = d3d_pancy_basic_singleton::GetInstance()->update_mouse();
	if (point.x > viewPort.TopLeftX && point.x < viewPort.TopLeftX + viewPort.Width && point.y > viewPort.TopLeftY && point.y < viewPort.TopLeftY + viewPort.Height)
	{
		shader_picture->set_UI_position(XMFLOAT4(0.0f, 0.5f, 0.0f, 0.0f));
		if (if_click)
		{
			//保存当前路径
			DWORD length_currentdir;
			TCHAR current_dir_path[MAX_PATH];
			ZeroMemory(current_dir_path, sizeof(current_dir_path));
			GetCurrentDirectory(MAX_PATH, current_dir_path);
			//保存文件
			GetSaveFileName(&ooutfn);
			DWORD dwMinSize = 0;
			LPSTR lpszStr = NULL;
			dwMinSize = WideCharToMultiByte(CP_OEMCP, NULL, szPathcurrent_save, -1, NULL, 0, NULL, FALSE);

			lpszStr = new char[dwMinSize];
			WideCharToMultiByte(CP_OEMCP, NULL, szPathcurrent_save, -1, lpszStr, dwMinSize, NULL, FALSE);

			string filename_str = lpszStr;
			if (filename_str != "")
			{
				auto path = get_file_path(filename_str);
				export_model(path, filename_str);
			}
			//还原当前路径
			SetCurrentDirectory(current_dir_path);
		}
	}
	else
	{
		shader_picture->set_UI_position(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	}
	shader_picture->set_UI_scal(XMFLOAT4(1.0f, 0.5f, 0.0f, 0.0f));
	shader_picture->set_tex_color_resource(export_model_tex);
	ID3DX11EffectTechnique *teque_need;
	shader_picture->get_technique(&teque_need, "draw_ui_move");
	picture_buf->get_teque(teque_need);
	picture_buf->show_mesh();
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(NULL, blendFactor, 0xffffffff);
}
void scene_test_square::CreateAndCopyToDebugBuf(ID3D11Resource *dest_res, ID3D11Resource *source_res)
{
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->CopyResource(dest_res, source_res);
}
HRESULT scene_test_square::CreateCPUaccessBuf(D3D11_TEXTURE2D_DESC texDesc, ID3D11Texture2D **resource_out)
{
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.BindFlags = 0;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	texDesc.MiscFlags = 0;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, NULL, resource_out);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
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
	for (auto data = scene_list.begin(); data != scene_list.end(); ++data)
	{
		(*data._Ptr)->release();
	}
	scene_list.clear();
}


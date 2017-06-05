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
	picture_type_width = 1024;
	picture_type_height = 1024;
	rec = 0.0f;
	mesh_need = new mesh_cube(false);
	picture_buf = new mesh_square(false);
	mesh_model_need = new model_reader_assimp<point_common>("square\\square.obj", "square\\");
}
engine_basic::engine_fail_reason scene_test_square::read_texture_from_file(std::vector<string> file_name_list)
{
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
	texture_deal = new texture_combine(rec_texture_packmap.size(), width_list, height_list, 1024, 1024);
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

	/*
	ifstream in_stream;
	in_stream.open("outmodel.pancymesh", ios::binary);
	int vnum_rec, inum_rec, tnum_rec;
	in_stream.read(reinterpret_cast<char*>(&vnum_rec), sizeof(vnum_rec));
	in_stream.read(reinterpret_cast<char*>(&inum_rec), sizeof(inum_rec));
	in_stream.read(reinterpret_cast<char*>(&tnum_rec), sizeof(tnum_rec));
	in_stream.read(reinterpret_cast<char*>(data_point_need), vnum_rec * sizeof(data_point_need[0]));
	in_stream.read(reinterpret_cast<char*>(data_index_need), inum_rec * sizeof(data_index_need[0]));
	*/
	
	mesh_model_need->get_model_pack_data(data_point_need, data_index_need);
	//ifstream in_stream;
	out_stream.open("outmodel.pancymesh", ios::binary);
	out_stream.write((char *)&vertex_num, sizeof(vertex_num));
	out_stream.write((char *)&index_num, sizeof(index_num));
	int texture_num = texture_deal->get_texture_num();
	out_stream.write((char *)&texture_num, sizeof(texture_num));
	change_model_texcoord(data_point_need, vertex_num);
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

	show_square();
	auto rendertarget = texture_deal->get_SRV_texarray();
	ID3D11Resource *resource_rec;
	rendertarget->GetResource(&resource_rec);
	string data_tail[] = { "a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z" };
	std::vector<string> file_name_saving;

	/*
	LPITEMIDLIST pil = NULL;
	INITCOMMONCONTROLSEX InitCtrls = { 0 };
	TCHAR szBuf[4096] = { 0 };
	BROWSEINFO bi = { 0 };
	bi.hwndOwner = NULL;
	bi.iImage = 0;
	bi.lParam = NULL;
	bi.lpfn = NULL;
	bi.lpszTitle = _T("请选择文件路径");
	bi.pszDisplayName = szBuf;
	bi.ulFlags = BIF_BROWSEINCLUDEFILES;

	InitCommonControlsEx(&InitCtrls);//在调用函数SHBrowseForFolder之前需要调用该函数初始化相关环境  
	pil = SHBrowseForFolder(&bi);
	if (NULL != pil)//若函数执行成功，并且用户选择问件路径并点击确定  
	{
		SHGetPathFromIDList(pil, szBuf);//获取用户选择的文件路径  
		wprintf_s(_T("%s"), szBuf);
	}
	*/

	out_stream.open("outmodel.pancymat");
	for (int i = 0; i < texture_deal->get_texture_num(); ++i)
	{
		string file_name_rec = "tex_pack_" + data_tail[i] + ".dds";
		file_name_saving.push_back(file_name_rec);
		d3d_pancy_basic_singleton::GetInstance()->save_texture(resource_rec, file_name_rec, i);
		out_stream.write(file_name_rec.c_str(), file_name_rec.size() * sizeof(char));
		out_stream.write("\n", sizeof(char));
	}
	out_stream.close();
	resource_rec->Release();

	read_texture_from_file(file_name_saving);




	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void scene_test_square::change_model_texcoord(point_common *vertex_need, int point_num)
{
	string lastname;
	texture_rebuild_data last_data;

	string last_normal_name;
	texture_rebuild_data last_normal_data;


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
	shader_need->set_tex_diffuse_array(test_resource);
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
	test_resource->Release();
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


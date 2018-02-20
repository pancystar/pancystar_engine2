#include"pancy_terrain.h"
//地形数据
pancy_terrain_part::pancy_terrain_part(
	float terrain_width_in, 
	int terrain_divide_in, 
	float Terrain_ColorTexScal_in, 
	float Terrain_HeightScal_in, 
	XMFLOAT2 terrain_offset_in, 
	string file_name
	)
{
	terrain_width = terrain_width_in;
	terrain_divide = terrain_divide_in;
	Terrain_HeightScal = Terrain_HeightScal_in;
	terrain_offset = terrain_offset_in;

	terrain_file_name = file_name;
	Terrain_ColorTexScal = Terrain_ColorTexScal_in;

}
engine_basic::engine_fail_reason pancy_terrain_part::create()
{
	engine_basic::engine_fail_reason check_error;
	check_error = load_terrain_file();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	terrain_renderbuffer = new mesh_terrain_tessellation(false, terrain_divide, terrain_width, Terrain_ColorTexScal);
	check_error = terrain_renderbuffer->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = load_terrain_height();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = load_terrain_blend();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = load_terrain_normal();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = load_terrain_tangent();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = load_terrain_color();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void pancy_terrain_part::release()
{
	terrain_renderbuffer->release();
	delete terrain_renderbuffer;
	terrain_height_tex->Release();
	terrain_blend_tex->Release();
	terrain_tangent_tex->Release();
	terrain_normal_tex->Release();
	terrain_color_albe_tex->Release();
	terrain_color_norm_tex->Release();
	terrain_height_data.clear();
}
engine_basic::engine_fail_reason pancy_terrain_part::load_terrain_height()
{
	struct _stat info;
	_stat(terrain_file.height_rawdata_name.c_str(), &info);
	int size_file = info.st_size;

	std::ifstream inFile;
	inFile.open(terrain_file.height_rawdata_name.c_str(), std::ios_base::binary);
	TexHeight_width = static_cast<int>(sqrt(static_cast<double>(size_file / 2)));

	char *data_pre = new char[size_file];
	float *data_need = new float[TexHeight_width*TexHeight_width];
	//DirectX::PackedVector::HALF *data_need = new DirectX::PackedVector::HALF[TexHeight_width*TexHeight_width];
	inFile.read(data_pre, size_file * sizeof(char));
	inFile.close();
	for (int i = 0; i < TexHeight_width*TexHeight_width; ++i)
	{
		unsigned char first_byte = data_pre[2 * i];
		unsigned char second_byte = data_pre[2 * i + 1];
		double data = static_cast<double>(first_byte + second_byte * 256) / 65535.0;
		data_need[i] = static_cast<float>(data);
		unsigned short data_unsigned = first_byte + second_byte * 256;
		short data_signed = static_cast<short>(first_byte + second_byte * 256);
		terrain_height_data.push_back(data_signed);
		//float rec_height = static_cast<float>(data_pre[2 * i] + data_pre[2 * i + 1] * 256) / 65535.0f;
		//data_need[i] = DirectX::PackedVector::XMConvertFloatToHalf(rec_height);
	}
	delete[] data_pre;
	/*
	std::ifstream inFile;
	inFile.open(terrain_file.height_rawdata_name.c_str(), std::ios_base::binary);
	while (!inFile.eof())
	{
	char now_byte_first, now_byte_second;
	inFile.read(&now_byte_first, sizeof(char));
	inFile.read(&now_byte_second, sizeof(char));
	float rec_height = static_cast<float>(now_byte_first + now_byte_second * 256) / 65535.0f;
	terrain_height_data.push_back(rec_height);
	}
	TexHeight_width = static_cast<int>(sqrt(static_cast<double>(terrain_height_data.size())));
	std::vector<DirectX::PackedVector::HALF> hmap(terrain_height_data.size());
	std::transform(terrain_height_data.begin(), terrain_height_data.end(), hmap.begin(), DirectX::PackedVector::XMConvertFloatToHalf);
	*/
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = TexHeight_width;
	texDesc.Height = TexHeight_width;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;



	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = data_need;
	//data.SysMemPitch = TexHeight_width*sizeof(DirectX::PackedVector::HALF);
	data.SysMemPitch = TexHeight_width*sizeof(float);
	data.SysMemSlicePitch = 0;

	ID3D11Texture2D* hmapTex = 0;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, &data, &hmapTex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "build terrain height map texdata error");
		return error_message;
	}
	delete[] data_need;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(hmapTex, &srvDesc, &terrain_height_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "build terrain height map texdata error");
		return error_message;
	}
	hmapTex->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_terrain_part::load_terrain_normal()
{
	size_t length_need = strlen(terrain_file.normal_texdata_name.c_str()) + 1;
	size_t converted = 0;
	wchar_t *data_output;
	data_output = (wchar_t*)malloc(length_need*sizeof(wchar_t));
	mbstowcs_s(&converted, data_output, length_need, terrain_file.normal_texdata_name.c_str(), _TRUNCATE);
	HRESULT hr = CreateDDSTextureFromFile(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), data_output, NULL, &terrain_normal_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "load terrain texture" + terrain_file.normal_texdata_name + "error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_terrain_part::load_terrain_tangent()
{
	size_t length_need = strlen(terrain_file.tangent_texdata_name.c_str()) + 1;
	size_t converted = 0;
	wchar_t *data_output;
	data_output = (wchar_t*)malloc(length_need*sizeof(wchar_t));
	mbstowcs_s(&converted, data_output, length_need, terrain_file.tangent_texdata_name.c_str(), _TRUNCATE);
	HRESULT hr = CreateDDSTextureFromFile(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), data_output, NULL, &terrain_tangent_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "load terrain texture" + terrain_file.tangent_texdata_name + "error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_terrain_part::load_terrain_blend()
{
	size_t length_need = strlen(terrain_file.blend_texdata_name.c_str()) + 1;
	size_t converted = 0;
	wchar_t *data_output;
	data_output = (wchar_t*)malloc(length_need*sizeof(wchar_t));
	mbstowcs_s(&converted, data_output, length_need, terrain_file.blend_texdata_name.c_str(), _TRUNCATE);
	HRESULT hr = CreateDDSTextureFromFile(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), data_output, NULL, &terrain_blend_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "load terrain texture" + terrain_file.blend_texdata_name + "error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_terrain_part::load_terrain_color()
{
	engine_basic::engine_fail_reason check_error;
	check_error = load_tex_array(terrain_file.color_albe_texdata_name, &terrain_color_albe_tex);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = load_tex_array(terrain_file.color_norm_texdata_name, &terrain_color_norm_tex);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_terrain_part::load_tex_array(string texdata_name[4], ID3D11ShaderResourceView **tex_array)
{
	std::vector<ID3D11Texture2D*> srcTex(4);
	for (int i = 0; i < 4; ++i)
	{
		size_t length_need = strlen(texdata_name[i].c_str()) + 1;
		size_t converted = 0;
		wchar_t *data_output;
		data_output = (wchar_t*)malloc(length_need*sizeof(wchar_t));
		mbstowcs_s(&converted, data_output, length_need, texdata_name[i].c_str(), _TRUNCATE);
		HRESULT hr = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), data_output, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ, 0, false, (ID3D11Resource**)&srcTex[i], 0, 0);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "load terrain texture" + texdata_name[i] + "error");
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
	texArrayDesc.ArraySize = 4;
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
	for (UINT texElement = 0; texElement < 4; ++texElement)
	{
		// for each mipmap level...
		for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
		{
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			HRESULT hr;
			hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D);
			if (FAILED(hr))
			{
				engine_basic::engine_fail_reason error_message(hr, "build terrain texture" + texdata_name[0] + "error");
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
	viewDesc.Texture2DArray.ArraySize = 4;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(texArray, &viewDesc, tex_array);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "load model texture" + texdata_name[0] + "error");
		return error_message;
	}
	texArray->Release();
	for (UINT i = 0; i < 4; ++i)
	{
		srcTex[i]->Release();
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
string pancy_terrain_part::find_path(string input)
{
	string check_out;
	int tail = -1;
	for (int i = input.size(); i >= 0; --i)
	{
		if (input[i] == '\\') 
		{
			tail = i;
			break;
		}
	}
	for (int i = 0; i <= tail; ++i)
	{
		check_out += input[i];
	}
	return check_out;
}
string pancy_terrain_part::move_string_space(string input)
{
	string check_out;
	int start_copy = input.size();
	for (int i = 0; i < input.size() - 1; ++i)
	{
		if (input[i] == ' ' && input[i + 1] != ' ') 
		{
			start_copy = i + 1;
		}
	}
	for (int i = start_copy; i < input.size(); ++i) 
	{
		check_out += input[i];
	}
	return check_out;
}
engine_basic::engine_fail_reason pancy_terrain_part::load_terrain_file()
{
	std::ifstream load_file;
	load_file.open(terrain_file_name);
	if (!load_file.is_open()) 
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "load terrain file " + terrain_file_name + " error");
		return error_message;
	}
	string path_terrain = find_path(terrain_file_name);
	char data[100];
	int length = 0;
	load_file.getline(data, 100);
	//地形纹理信息
	load_file.getline(data, 100);
	terrain_file.height_rawdata_name = path_terrain + move_string_space(data);
	load_file.getline(data, 100);
	terrain_file.normal_texdata_name = path_terrain + move_string_space(data);
	load_file.getline(data, 100);
	terrain_file.tangent_texdata_name = path_terrain + move_string_space(data);
	load_file.getline(data, 100);
	terrain_file.blend_texdata_name = path_terrain + move_string_space(data);
	//颜色纹理信息
	string tex_albedo_name[4];
	string tex_normal_name[4];
	for (int i = 0; i < 4; ++i)
	{
		load_file.getline(data, 100);
		load_file.getline(data, 100);
		terrain_file.color_albe_texdata_name[i] = path_terrain + data;
		load_file.getline(data, 100);
		terrain_file.color_norm_texdata_name[i] = path_terrain + data;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void pancy_terrain_part::render_terrain(XMFLOAT3 view_pos, XMFLOAT4X4 view_mat, XMFLOAT4X4 proj_mat)
{
	engine_basic::engine_fail_reason check_error;
	auto shader_terrain = shader_control::GetInstance()->get_shader_terrain_test(check_error);
	XMFLOAT4X4 world_mat, final_mat;
	XMStoreFloat4x4(&world_mat, XMMatrixTranslation(terrain_offset.x - (terrain_width / 2.0f), 0, terrain_offset.y - (terrain_width / 2.0f)));
	XMStoreFloat4x4(&final_mat, XMMatrixTranslation(terrain_offset.x - (terrain_width / 2.0f), 0, terrain_offset.y - (terrain_width / 2.0f)) * XMLoadFloat4x4(&view_mat) * XMLoadFloat4x4(&proj_mat));
	shader_terrain->set_trans_world(&world_mat);
	shader_terrain->set_trans_all(&final_mat);
	shader_terrain->set_terrain_size(terrain_width, TexHeight_width, Terrain_HeightScal);
	shader_terrain->set_view_pos(view_pos);
	shader_terrain->set_texture_height(terrain_height_tex);
	shader_terrain->set_texture_normal(terrain_normal_tex);
	shader_terrain->set_texture_tangnt(terrain_tangent_tex);
	shader_terrain->set_texture_blend(terrain_blend_tex);
	shader_terrain->set_texture_color(terrain_color_albe_tex, terrain_color_norm_tex);

	ID3DX11EffectTechnique* tech_need;
	shader_terrain->get_technique(&tech_need, "LightTerrain");
	terrain_renderbuffer->get_teque(tech_need);
	terrain_renderbuffer->show_mesh();
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->HSSetShader(NULL, 0, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->DSSetShader(NULL, 0, 0);
}
//地形组织资源
terrain_part_resource::terrain_part_resource(
	int self_ID_in,
	float terrain_width_in,
	int terrain_divide_in,
	float Terrain_ColorTexScal_in,
	float Terrain_HeightScal_in,
	XMFLOAT2 terrain_offset_in,
	string file_name_in
	)
{
	self_ID = self_ID_in;
	terrain_width = terrain_width_in;
	terrain_divide = terrain_divide_in;
	Terrain_ColorTexScal = Terrain_ColorTexScal_in;
	Terrain_HeightScal = Terrain_HeightScal_in;
	terrain_offset = terrain_offset_in;
	file_name = file_name_in;
	now_terrain = NULL;
	if_loaded = false;
	neighbour_up_ID = -1;
	neighbour_down_ID = -1;
	neighbour_left_ID = -1;
	neighbour_right_ID = -1;
	neighbour_upleft_ID = -1;
	neighbour_upright_ID = -1;
	neighbour_downleft_ID = -1;
	neighbour_downright_ID = -1;
	if_loaded_physics = false;
	if_wakeup_physics = false;
}
void terrain_part_resource::get_all_neighbour(int neighbour_ID[9])
{
	neighbour_ID[0] = neighbour_downleft_ID;
	neighbour_ID[1] = neighbour_left_ID;
	neighbour_ID[2] = neighbour_upleft_ID;
	neighbour_ID[3] = neighbour_down_ID;
	neighbour_ID[4] = self_ID;
	neighbour_ID[5] = neighbour_up_ID;
	neighbour_ID[6] = neighbour_downright_ID;
	neighbour_ID[7] = neighbour_right_ID;
	neighbour_ID[8] = neighbour_upright_ID;

}
engine_basic::engine_fail_reason terrain_part_resource::build_resource(pancy_physx_scene *physic_scene)
{
	now_terrain = new pancy_terrain_part(terrain_width, terrain_divide, Terrain_ColorTexScal, Terrain_HeightScal, terrain_offset, file_name);
	engine_basic::engine_fail_reason check_error = now_terrain->create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	if_loaded = true;
	if (!if_loaded_physics) 
	{
		build_physic(physic_scene);
	}
	else 
	{
		physic_scene->wakeup_a_terrain(terrain_physx_ID);
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason terrain_part_resource::build_physic(pancy_physx_scene *physic_scene)
{
	physx::PxHeightFieldSample* samples = new physx::PxHeightFieldSample[now_terrain->get_terrain_height_width() * now_terrain->get_terrain_height_width()];
	physx::PxHeightFieldDesc hfDesc;
	hfDesc.format = physx::PxHeightFieldFormat::eS16_TM;
	hfDesc.nbColumns = now_terrain->get_terrain_height_width();
	hfDesc.nbRows = now_terrain->get_terrain_height_width();
	auto height_list = now_terrain->get_terrain_height_data();
	int data_num = 0;
	/*
	for (int i = 0; i < now_terrain->get_terrain_height_width(); ++i) 
	{
		for (int j = 0; j < now_terrain->get_terrain_height_width(); ++j)
		{
			samples[j*now_terrain->get_terrain_height_width() + i].height = height_list[i*now_terrain->get_terrain_height_width() + j];
		}
	}
	*/
	float height_scal = Terrain_HeightScal / 65535.0f;
	for (auto data_vector = height_list.begin(); data_vector != height_list.end(); ++data_vector) 
	{
		int i = data_num / now_terrain->get_terrain_height_width();
		int j = data_num % now_terrain->get_terrain_height_width();
		samples[j*now_terrain->get_terrain_height_width() + i].height = *data_vector._Ptr;
		data_num += 1;
	}
	hfDesc.samples.data = samples;
	hfDesc.samples.stride = sizeof(physx::PxHeightFieldSample);
	float terrain_widht_divide = (1.0 / static_cast<float>(now_terrain->get_terrain_height_width())) * terrain_width;
	physx::PxVec3 rec_scal(Terrain_HeightScal / 65535.0f, terrain_widht_divide, terrain_widht_divide);
	engine_basic::engine_fail_reason check_error = physic_scene->add_a_terrain_object(hfDesc, rec_scal, physx::PxTransform(physx::PxVec3(terrain_offset.x - (terrain_width / 2.0f), 0.0f, terrain_offset.y - (terrain_width / 2.0f))), terrain_physx_ID);
	delete[] samples;
	if (!check_error.check_if_failed()) 
	{
		return check_error;
	}
	if_loaded_physics = true;
	if_wakeup_physics = true;
	engine_basic::engine_fail_reason succeed;
	return succeed;
	//hr = physic_scene->create_terrain(hfDesc, rec_scal, physx::PxTransform(physx::PxVec3(-1500.0f, 0.0f, -1500.0f)), terrain_mat_force);
}
void terrain_part_resource::release_resource(pancy_physx_scene *physic_scene)
{
	//卸载模型信息
	now_terrain->release();
	delete now_terrain;
	now_terrain = NULL;
	if_loaded = false;
	//移除物理信息
	physic_scene->delete_a_terrain(terrain_physx_ID);
	if_loaded_physics = false;
	//physic_scene->sleep_a_terrain(terrain_physx_ID);
	if_wakeup_physics = false;
}
void terrain_part_resource::display(XMFLOAT3 view_pos, XMFLOAT4X4 view_mat, XMFLOAT4X4 proj_mat)
{
	now_terrain->render_terrain(view_pos, view_mat, proj_mat);
}
//地形管理器
pancy_terrain_control::pancy_terrain_control(
	pancy_physx_scene *physc_in,
	string terrain_list,
	float terrain_width_in,
	int terrain_divide_in,
	float Terrain_ColorTexScal_in,
	float Terrain_HeightScal_in,
	float rebuild_dis
	)
{
	physic_scene = physc_in;
	terrain_list_file = terrain_list;
	terrain_width = terrain_width_in;
	terrain_divide = terrain_divide_in;
	Terrain_ColorTexScal = Terrain_ColorTexScal_in;
	Terrain_HeightScal = Terrain_HeightScal_in;
	rebuild_distance_quality = rebuild_dis;
	if_created = false;
}
string pancy_terrain_control::move_string_space(string input)
{
	string check_out;
	int start_copy = input.size();
	for (int i = 0; i < input.size() - 1; ++i)
	{
		if (input[i] == ' ' && input[i + 1] != ' ')
		{
			start_copy = i + 1;
		}
	}
	for (int i = start_copy; i < input.size(); ++i)
	{
		check_out += input[i];
	}
	return check_out;
}
string pancy_terrain_control::find_path(string input)
{
	string check_out;
	int tail = -1;
	for (int i = input.size(); i >= 0; --i)
	{
		if (input[i] == '\\')
		{
			tail = i;
			break;
		}
	}
	for (int i = 0; i <= tail; ++i)
	{
		check_out += input[i];
	}
	return check_out;
}
engine_basic::engine_fail_reason pancy_terrain_control::build_neighbour(int node_ID)
{
	auto data_center = &terrain_data_list.find(node_ID)->second;
	if (data_center->get_neighbour_up_ID() == -1 || data_center->get_neighbour_down_ID() == -1 || data_center->get_neighbour_left_ID() == -1 || data_center->get_neighbour_right_ID() == -1)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "the tree havent been completely build");
		return error_message;
	}
	if (data_center->get_neighbour_upleft_ID() == -1 || data_center->get_neighbour_upright_ID() == -1 || data_center->get_neighbour_downleft_ID() == -1 || data_center->get_neighbour_downright_ID() == -1)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "the tree havent been completely build");
		return error_message;
	}
	//八个区域子节点
	auto data_up = &terrain_data_list.find(data_center->get_neighbour_up_ID())->second;
	auto data_down = &terrain_data_list.find(data_center->get_neighbour_down_ID())->second;
	auto data_left = &terrain_data_list.find(data_center->get_neighbour_left_ID())->second;
	auto data_right = &terrain_data_list.find(data_center->get_neighbour_right_ID())->second;
	auto data_upleft = &terrain_data_list.find(data_center->get_neighbour_upleft_ID())->second;
	auto data_upright = &terrain_data_list.find(data_center->get_neighbour_upright_ID())->second;
	auto data_downleft = &terrain_data_list.find(data_center->get_neighbour_downleft_ID())->second;
	auto data_downright = &terrain_data_list.find(data_center->get_neighbour_downright_ID())->second;
	//建立上区域子节点
	if (data_up->get_neighbour_down_ID() == -1)
	{
		data_up->set_neighbour_down_ID(data_center->get_self_ID());
	}
	else if (data_up->get_neighbour_down_ID() != data_center->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the up_node's down node");
		return error_message;
	}
	if (data_up->get_neighbour_left_ID() == -1)
	{
		data_up->set_neighbour_left_ID(data_upleft->get_self_ID());
	}
	else if (data_up->get_neighbour_left_ID() != data_upleft->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the up_node's left node");
		return error_message;
	}
	if (data_up->get_neighbour_right_ID() == -1)
	{
		data_up->set_neighbour_right_ID(data_upright->get_self_ID());
	}
	else if (data_up->get_neighbour_right_ID() != data_upright->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the up_node's right node");
		return error_message;
	}
	if (data_up->get_neighbour_downleft_ID() == -1)
	{
		data_up->set_neighbour_downleft_ID(data_left->get_self_ID());
	}
	else if (data_up->get_neighbour_downleft_ID() != data_left->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the up_node's downleft node");
		return error_message;
	}
	if (data_up->get_neighbour_downright_ID() == -1)
	{
		data_up->set_neighbour_downright_ID(data_right->get_self_ID());
	}
	else if (data_up->get_neighbour_downright_ID() != data_right->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the up_node's downright node");
		return error_message;
	}
	//建立下区域子节点
	if (data_down->get_neighbour_up_ID() == -1)
	{
		data_down->set_neighbour_up_ID(data_center->get_self_ID());
	}
	else if (data_down->get_neighbour_up_ID() != data_center->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the down_node's up node");
		return error_message;
	}
	if (data_down->get_neighbour_left_ID() == -1)
	{
		data_down->set_neighbour_left_ID(data_downleft->get_self_ID());
	}
	else if (data_down->get_neighbour_left_ID() != data_downleft->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the down_node's left node");
		return error_message;
	}
	if (data_down->get_neighbour_right_ID() == -1)
	{
		data_down->set_neighbour_right_ID(data_downright->get_self_ID());
	}
	else if (data_down->get_neighbour_right_ID() != data_downright->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the down_node's right node");
		return error_message;
	}
	if (data_down->get_neighbour_upleft_ID() == -1)
	{
		data_down->set_neighbour_upleft_ID(data_left->get_self_ID());
	}
	else if (data_down->get_neighbour_upleft_ID() != data_left->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the down_node's upleft node");
		return error_message;
	}
	if (data_down->get_neighbour_upright_ID() == -1)
	{
		data_down->set_neighbour_upright_ID(data_right->get_self_ID());
	}
	else if (data_down->get_neighbour_upright_ID() != data_right->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the down_node's upright node");
		return error_message;
	}
	//建立左区域子节点
	if (data_left->get_neighbour_up_ID() == -1)
	{
		data_left->set_neighbour_up_ID(data_upleft->get_self_ID());
	}
	else if (data_left->get_neighbour_up_ID() != data_upleft->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the left_node's up node");
		return error_message;
	}
	if (data_left->get_neighbour_down_ID() == -1)
	{
		data_left->set_neighbour_down_ID(data_downleft->get_self_ID());
	}
	else if (data_left->get_neighbour_down_ID() != data_downleft->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the left_node's down node");
		return error_message;
	}
	if (data_left->get_neighbour_right_ID() == -1)
	{
		data_left->set_neighbour_right_ID(data_center->get_self_ID());
	}
	else if (data_left->get_neighbour_right_ID() != data_center->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the left_node's right node");
		return error_message;
	}
	if (data_left->get_neighbour_upright_ID() == -1)
	{
		data_left->set_neighbour_upright_ID(data_up->get_self_ID());
	}
	else if (data_left->get_neighbour_upright_ID() != data_up->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the left_node's upright node");
		return error_message;
	}
	if (data_left->get_neighbour_downright_ID() == -1)
	{
		data_left->set_neighbour_downright_ID(data_down->get_self_ID());
	}
	else if (data_left->get_neighbour_downright_ID() != data_down->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the left_node's downright node");
		return error_message;
	}
	//建立右区域子节点
	if (data_right->get_neighbour_up_ID() == -1)
	{
		data_right->set_neighbour_up_ID(data_upright->get_self_ID());
	}
	else if (data_right->get_neighbour_up_ID() != data_upright->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the right_node's up node");
		return error_message;
	}
	if (data_right->get_neighbour_down_ID() == -1)
	{
		data_right->set_neighbour_down_ID(data_downright->get_self_ID());
	}
	else if (data_right->get_neighbour_down_ID() != data_downright->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the right_node's down node");
		return error_message;
	}
	if (data_right->get_neighbour_left_ID() == -1)
	{
		data_right->set_neighbour_left_ID(data_center->get_self_ID());
	}
	else if (data_right->get_neighbour_left_ID() != data_center->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the right_node's left node");
		return error_message;
	}
	if (data_right->get_neighbour_upleft_ID() == -1)
	{
		data_right->set_neighbour_upleft_ID(data_up->get_self_ID());
	}
	else if (data_right->get_neighbour_upleft_ID() != data_up->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the right_node's upleft node");
		return error_message;
	}
	if (data_right->get_neighbour_downleft_ID() == -1)
	{
		data_right->set_neighbour_downleft_ID(data_down->get_self_ID());
	}
	else if (data_right->get_neighbour_downleft_ID() != data_down->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the right_node's downleft node");
		return error_message;
	}
	//建立左上区域子节点
	if (data_upleft->get_neighbour_right_ID() == -1)
	{
		data_upleft->set_neighbour_right_ID(data_up->get_self_ID());
	}
	else if (data_upleft->get_neighbour_right_ID() != data_up->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the upleft_node's right node");
		return error_message;
	}
	if (data_upleft->get_neighbour_down_ID() == -1)
	{
		data_upleft->set_neighbour_down_ID(data_left->get_self_ID());
	}
	else if (data_upleft->get_neighbour_down_ID() != data_left->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the upleft_node's down node");
		return error_message;
	}
	if (data_upleft->get_neighbour_downright_ID() == -1)
	{
		data_upleft->set_neighbour_downright_ID(data_center->get_self_ID());
	}
	else if (data_upleft->get_neighbour_downright_ID() != data_center->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the upleft_node's downright node");
		return error_message;
	}
	//建立右上区域子节点
	if (data_upright->get_neighbour_left_ID() == -1)
	{
		data_upright->set_neighbour_left_ID(data_up->get_self_ID());
	}
	else if (data_upright->get_neighbour_left_ID() != data_up->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the upright_node's left node");
		return error_message;
	}
	if (data_upright->get_neighbour_down_ID() == -1)
	{
		data_upright->set_neighbour_down_ID(data_right->get_self_ID());
	}
	else if (data_upright->get_neighbour_down_ID() != data_right->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the upright_node's down node");
		return error_message;
	}
	if (data_upright->get_neighbour_downleft_ID() == -1)
	{
		data_upright->set_neighbour_downleft_ID(data_center->get_self_ID());
	}
	else if (data_upright->get_neighbour_downleft_ID() != data_center->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the upright_node's downleft node");
		return error_message;
	}
	//建立左下区域子节点
	if (data_downleft->get_neighbour_right_ID() == -1)
	{
		data_downleft->set_neighbour_right_ID(data_down->get_self_ID());
	}
	else if (data_downleft->get_neighbour_right_ID() != data_down->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the downleft_node's right node");
		return error_message;
	}
	if (data_downleft->get_neighbour_up_ID() == -1)
	{
		data_downleft->set_neighbour_up_ID(data_left->get_self_ID());
	}
	else if (data_downleft->get_neighbour_up_ID() != data_left->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the downleft_node's up node");
		return error_message;
	}
	if (data_downleft->get_neighbour_upright_ID() == -1)
	{
		data_downleft->set_neighbour_upright_ID(data_center->get_self_ID());
	}
	else if (data_downleft->get_neighbour_upright_ID() != data_center->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the downleft_node's upright node");
		return error_message;
	}
	//建立右下区域子节点
	if (data_downright->get_neighbour_left_ID() == -1)
	{
		data_downright->set_neighbour_left_ID(data_down->get_self_ID());
	}
	else if (data_downright->get_neighbour_left_ID() != data_down->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the downright_node's left node");
		return error_message;
	}
	if (data_downright->get_neighbour_up_ID() == -1)
	{
		data_downright->set_neighbour_up_ID(data_right->get_self_ID());
	}
	else if (data_downright->get_neighbour_up_ID() != data_right->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the downright_node's up node");
		return error_message;
	}
	if (data_downright->get_neighbour_upleft_ID() == -1)
	{
		data_downright->set_neighbour_upleft_ID(data_center->get_self_ID());
	}
	else if (data_downright->get_neighbour_upleft_ID() != data_center->get_self_ID())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list build error when find the downright_node's upleft node");
		return error_message;
	}

	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_terrain_control::build_terrain_tree()
{
	int now_queue_tail = 1;
	int now_build_headtree = 0;//当前正在建造的地形树的顶节点
	while (true)
	{
		//获取当前的根节点
		auto data_center = &terrain_data_list.find(now_build_headtree)->second;
		//链接根节点的子节点
		if (now_queue_tail >= terrain_data_list.size())
		{
			break;
		}
		if (data_center->get_neighbour_up_ID() == -1)
		{
			data_center->set_neighbour_up_ID(now_queue_tail);
			terrain_data_list.find(now_queue_tail)->second.set_neighbour_down_ID(data_center->get_self_ID());
			XMFLOAT2 up_position;
			up_position.x = data_center->get_offset().x;
			up_position.y = data_center->get_offset().y + terrain_width;
			terrain_data_list.find(now_queue_tail)->second.set_offset(up_position);
			now_queue_tail += 1;
		}
		else if (data_center->get_self_ID() != terrain_data_list.find(data_center->get_neighbour_up_ID())->second.get_neighbour_down_ID())
		{
			engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list headnode build error when find the up node");
			return error_message;
		}

		if (now_queue_tail >= terrain_data_list.size())
		{
			break;
		}
		if (data_center->get_neighbour_down_ID() == -1)
		{
			data_center->set_neighbour_down_ID(now_queue_tail);
			terrain_data_list.find(now_queue_tail)->second.set_neighbour_up_ID(data_center->get_self_ID());
			XMFLOAT2 down_position;
			down_position.x = data_center->get_offset().x;
			down_position.y = data_center->get_offset().y - terrain_width;
			terrain_data_list.find(now_queue_tail)->second.set_offset(down_position);

			now_queue_tail += 1;
		}
		else if (data_center->get_self_ID() != terrain_data_list.find(data_center->get_neighbour_down_ID())->second.get_neighbour_up_ID())
		{
			engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list headnode build error when find the down node");
			return error_message;
		}

		if (now_queue_tail >= terrain_data_list.size())
		{
			break;
		}
		if (data_center->get_neighbour_left_ID() == -1)
		{
			data_center->set_neighbour_left_ID(now_queue_tail);
			terrain_data_list.find(now_queue_tail)->second.set_neighbour_right_ID(data_center->get_self_ID());
			XMFLOAT2 left_position;
			left_position.x = data_center->get_offset().x - terrain_width;
			left_position.y = data_center->get_offset().y;
			terrain_data_list.find(now_queue_tail)->second.set_offset(left_position);

			now_queue_tail += 1;
		}
		else if (data_center->get_self_ID() != terrain_data_list.find(data_center->get_neighbour_left_ID())->second.get_neighbour_right_ID())
		{
			engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list headnode build error when find the left node");
			return error_message;
		}

		if (now_queue_tail >= terrain_data_list.size())
		{
			break;
		}
		if (data_center->get_neighbour_right_ID() == -1)
		{
			data_center->set_neighbour_right_ID(now_queue_tail);
			terrain_data_list.find(now_queue_tail)->second.set_neighbour_left_ID(data_center->get_self_ID());
			XMFLOAT2 right_position;
			right_position.x = data_center->get_offset().x + terrain_width;
			right_position.y = data_center->get_offset().y;
			terrain_data_list.find(now_queue_tail)->second.set_offset(right_position);
			now_queue_tail += 1;
		}
		else if (data_center->get_self_ID() != terrain_data_list.find(data_center->get_neighbour_right_ID())->second.get_neighbour_left_ID())
		{
			engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list headnode build error when find the right node");
			return error_message;
		}


		if (now_queue_tail >= terrain_data_list.size())
		{
			break;
		}
		if (data_center->get_neighbour_upleft_ID() == -1)
		{
			data_center->set_neighbour_upleft_ID(now_queue_tail);
			terrain_data_list.find(now_queue_tail)->second.set_neighbour_downright_ID(data_center->get_self_ID());
			XMFLOAT2 upleft_position;
			upleft_position.x = data_center->get_offset().x - terrain_width;
			upleft_position.y = data_center->get_offset().y + terrain_width;
			terrain_data_list.find(now_queue_tail)->second.set_offset(upleft_position);
			now_queue_tail += 1;
		}
		else if (data_center->get_self_ID() != terrain_data_list.find(data_center->get_neighbour_upleft_ID())->second.get_neighbour_downright_ID())
		{
			engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list headnode build error when find the upleft node");
			return error_message;
		}

		if (now_queue_tail >= terrain_data_list.size())
		{
			break;
		}
		if (data_center->get_neighbour_upright_ID() == -1)
		{
			data_center->set_neighbour_upright_ID(now_queue_tail);
			terrain_data_list.find(now_queue_tail)->second.set_neighbour_downleft_ID(data_center->get_self_ID());
			XMFLOAT2 upright_position;
			upright_position.x = data_center->get_offset().x + terrain_width;
			upright_position.y = data_center->get_offset().y + terrain_width;
			terrain_data_list.find(now_queue_tail)->second.set_offset(upright_position);
			now_queue_tail += 1;
		}
		else if (data_center->get_self_ID() != terrain_data_list.find(data_center->get_neighbour_upright_ID())->second.get_neighbour_downleft_ID())
		{
			engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list headnode build error when find the upright node");
			return error_message;
		}

		if (now_queue_tail >= terrain_data_list.size())
		{
			break;
		}
		if (data_center->get_neighbour_downleft_ID() == -1)
		{
			data_center->set_neighbour_downleft_ID(now_queue_tail);
			terrain_data_list.find(now_queue_tail)->second.set_neighbour_upright_ID(data_center->get_self_ID());
			XMFLOAT2 downleft_position;
			downleft_position.x = data_center->get_offset().x - terrain_width;
			downleft_position.y = data_center->get_offset().y - terrain_width;
			terrain_data_list.find(now_queue_tail)->second.set_offset(downleft_position);
			now_queue_tail += 1;
		}
		else if (data_center->get_self_ID() != terrain_data_list.find(data_center->get_neighbour_downleft_ID())->second.get_neighbour_upright_ID())
		{
			engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list headnode build error when find the downleft node");
			return error_message;
		}

		if (now_queue_tail >= terrain_data_list.size())
		{
			break;
		}
		if (data_center->get_neighbour_downright_ID() == -1)
		{
			data_center->set_neighbour_downright_ID(now_queue_tail);
			terrain_data_list.find(now_queue_tail)->second.set_neighbour_upleft_ID(data_center->get_self_ID());
			XMFLOAT2 downright_position;
			downright_position.x = data_center->get_offset().x + terrain_width;
			downright_position.y = data_center->get_offset().y - terrain_width;
			terrain_data_list.find(now_queue_tail)->second.set_offset(downright_position);
			now_queue_tail += 1;
		}
		else if (data_center->get_self_ID() != terrain_data_list.find(data_center->get_neighbour_downright_ID())->second.get_neighbour_upleft_ID())
		{
			engine_basic::engine_fail_reason error_message(E_FAIL, "terrain list headnode build error when find the downright node");
			return error_message;
		}

		//子节点互相连接
		build_neighbour(now_build_headtree);
		//切换根节点
		now_build_headtree += 1;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason pancy_terrain_control::load_a_terrain(int node_ID)
{
	auto data_center = &terrain_data_list.find(node_ID)->second;
	if (data_center->check_if_loaded() == true)
	{
		//资源在之前已经被加载创建
		engine_basic::engine_fail_reason error_message(E_FAIL, "the resource have been build, do not rebuild resource");
	}
	engine_basic::engine_fail_reason check_error = data_center->build_resource(physic_scene);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_terrain_control::unload_a_terrain(int node_ID)
{
	auto data_center = &terrain_data_list.find(node_ID)->second;
	if (data_center->check_if_loaded() == false)
	{
		//资源尚未创建，无法释放
		engine_basic::engine_fail_reason error_message(E_FAIL, "the resource haven't been build, could not release resource");
	}
	data_center->release_resource(physic_scene);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason pancy_terrain_control::create()
{
	std::ifstream load_file;
	load_file.open(terrain_list_file);
	if (!load_file.is_open())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "load terrain file " + terrain_list_file + " error");
		return error_message;
	}
	char data[100];
	int length = 0;
	load_file.getline(data, 100);
	string file_path = find_path(terrain_list_file);
	while (!load_file.eof())
	{
		load_file.getline(data, 100);
		string now_terrain_data = file_path + move_string_space(data);
		terrain_part_resource *new_terrain_data = new terrain_part_resource(length, terrain_width, terrain_divide, Terrain_ColorTexScal, Terrain_HeightScal, XMFLOAT2(0, 0), now_terrain_data);
		std::pair<int, terrain_part_resource> new_terrain(length, *new_terrain_data);
		terrain_data_list.insert(new_terrain);
		length += 1;
		delete new_terrain_data;
	}
	engine_basic::engine_fail_reason check_error = build_terrain_tree();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void pancy_terrain_control::update(XMFLOAT3 view_pos,XMFLOAT4X4 view_matrix_in,XMFLOAT4X4 proj_matrix_in)
{
	now_view_pos = view_pos;
	view_matrix = view_matrix_in;//取景变换
	proj_matrix = proj_matrix_in;//投影变换
	auto data_center = &terrain_data_list.find(now_center_terrain)->second;
	XMFLOAT2 view_pos_2d = XMFLOAT2(view_pos.x, view_pos.z);
	XMFLOAT2 now_center_pos = data_center->get_offset();
	float dir_x = abs(now_center_pos.x - view_pos_2d.x);
	float dir_y = abs(now_center_pos.y - view_pos_2d.y);
	float rebuild_distance_need = rebuild_distance_quality * terrain_width + terrain_width / 2.0f;
	if (dir_x > rebuild_distance_need || dir_y > rebuild_distance_need || !if_created)
	{
		//消去第一次创建的标志
		if_created = true;
		//摄像机越界，重新构造九宫格
		int neighbour_ID[9];
		data_center->get_all_neighbour(neighbour_ID);
		//计算当前摄像机所在地形的归属
		int x_dis = static_cast<int>(view_pos_2d.x - now_center_pos.x);
		int y_dis = static_cast<int>(view_pos_2d.y - now_center_pos.y);
		int x_check = 0, y_check = 0;
		if (x_dis > terrain_width / 2.0f)
		{
			//新的地形中心位于原地形的右边
			x_check = 1;
		}
		if (x_dis < -terrain_width / 2.0f)
		{
			//新的地形中心位于原地形的左边
			x_check = -1;
		}
		if (y_dis > terrain_width / 2.0f)
		{
			//新的地形中心位于原地形的上边
			y_check = 1;
		}
		if (y_dis < -terrain_width / 2.0f)
		{
			//新的地形中心位于原地形的下边
			y_check = -1;
		}
		//三进制索引
		x_check += 1;
		y_check += 1;
		int index_neighbour = x_check * 3 + y_check;
		//terrain_part_resource *exchange_terrain = NULL;
		if (neighbour_ID[index_neighbour] == -1)
		{
			//对应的边缘地形不存在
			return;
		}
		auto exchange_terrain = &terrain_data_list.find(neighbour_ID[index_neighbour])->second;
		//XMFLOAT2 new_center_pos = terrain_data_list.find(neighbour_ID[index_neighbour])->second.get_offset();
		XMFLOAT2 new_center_pos = exchange_terrain->get_offset();
		//释放与新的中心地形不邻接的地形
		for (int i = 0; i < 9; ++i)
		{
			if (neighbour_ID[i] != -1)
			{
				auto data_neighbour = terrain_data_list.find(neighbour_ID[i]);
				if (data_neighbour != terrain_data_list.end())
				{
					XMFLOAT2 off_pos_2d = data_neighbour->second.get_offset();
					float min_distance = 1.5f*terrain_width;
					float distance_x = abs(new_center_pos.x - off_pos_2d.x);
					float distance_y = abs(new_center_pos.y - off_pos_2d.y);
					if (distance_x > min_distance || distance_y > min_distance)
					{
						if (data_neighbour->second.check_if_loaded()) 
						{
							data_neighbour->second.release_resource(physic_scene);
						}
					}
				}

			}
		}
		//为新的中心点拓展边界地形
		int all_new_neighbour[9];
		exchange_terrain->get_all_neighbour(all_new_neighbour);
		for (int i = 0; i < 9; ++i)
		{
			if (all_new_neighbour[i] != -1)
			{
				auto now_new_neighbour = terrain_data_list.find(all_new_neighbour[i]);
				if (now_new_neighbour->second.check_if_loaded() == false)
				{
					now_new_neighbour->second.build_resource(physic_scene);
				}
			}
		}
		now_center_terrain = neighbour_ID[index_neighbour];
	}

}
void pancy_terrain_control::release() 
{
	auto data_center = &terrain_data_list.find(now_center_terrain)->second;
	int data_all[9];
	data_center->get_all_neighbour(data_all);
	for (int i = 0; i < 9; ++i) 
	{
		if (data_all[i] != -1) 
		{
			auto data_delete = terrain_data_list.find(data_all[i]);
			if (data_delete != terrain_data_list.end() && data_delete->second.check_if_loaded())
			{
				data_delete->second.release_resource(physic_scene);
			}
		}
	}
}
void pancy_terrain_control::display() 
{
	auto data_center = &terrain_data_list.find(now_center_terrain)->second;
	int data_all[9];
	data_center->get_all_neighbour(data_all);
	for (int i = 0; i < 9; ++i)
	{
		if (data_all[i] != -1)
		{
			auto data_render = terrain_data_list.find(data_all[i]);
			if (data_render != terrain_data_list.end() && data_render->second.check_if_loaded())
			{
				XMFLOAT2 offset_pos = data_render->second.get_offset();
				XMFLOAT2 distance;
				distance.x = abs(offset_pos.x - now_view_pos.x);
				distance.y = abs(offset_pos.y - now_view_pos.z);
				if (distance.x < (terrain_width / 2.0f + terrain_width / 3.0f) && distance.y < (terrain_width / 2.0f + terrain_width / 3.0f)) 
				{
					data_render->second.display(now_view_pos,view_matrix,proj_matrix);
				}
			}
		}
	}
}
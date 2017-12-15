#include"pancy_terrain.h"

pancy_terrain_part::pancy_terrain_part(float terrain_width_in, int terrain_divide_in, float Terrain_ColorTexScal_in, float Terrain_HeightScal_in, XMFLOAT2 terrain_offset_in, terrain_file_path file_name)
{
	terrain_width = terrain_width_in;
	terrain_divide = terrain_divide_in;
	Terrain_HeightScal = Terrain_HeightScal_in;
	terrain_offset = terrain_offset_in;

	terrain_file = file_name;
	Terrain_ColorTexScal = Terrain_ColorTexScal_in;

}
engine_basic::engine_fail_reason pancy_terrain_part::create()
{
	engine_basic::engine_fail_reason check_error;
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
	terrain_height_tex->Release();
	terrain_blend_tex->Release();
	terrain_tangent_tex->Release();
	terrain_normal_tex->Release();
	terrain_color_albe_tex->Release();
	terrain_color_norm_tex->Release();
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
	for (int i = 0; i < TexHeight_width*TexHeight_width; ++i)
	{
		unsigned char first_byte = data_pre[2 * i];
		unsigned char second_byte = data_pre[2 * i + 1];
		double data = static_cast<double>(first_byte + second_byte * 256) / 65535.0;
		data_need[i] = static_cast<float>(data);
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
void pancy_terrain_part::render_terrain(XMFLOAT3 view_pos, XMFLOAT4X4 view_mat, XMFLOAT4X4 proj_mat)
{
	engine_basic::engine_fail_reason check_error;
	auto shader_terrain = shader_control::GetInstance()->get_shader_terrain_test(check_error);
	XMFLOAT4X4 world_mat, final_mat;
	XMStoreFloat4x4(&world_mat, XMMatrixTranslation(terrain_offset.x, 0, terrain_offset.y));
	XMStoreFloat4x4(&final_mat, XMMatrixTranslation(terrain_offset.x, 0, terrain_offset.y) * XMLoadFloat4x4(&view_mat) * XMLoadFloat4x4(&proj_mat));
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

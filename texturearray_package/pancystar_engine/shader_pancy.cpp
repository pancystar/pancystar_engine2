#include"shader_pancy.h"
bool shader_basic::WCharToMByte(LPCWSTR lpcwszStr, std::string &str)
{
	DWORD dwMinSize = 0;
	LPSTR lpszStr = NULL;
	dwMinSize = WideCharToMultiByte(CP_OEMCP, NULL, lpcwszStr, -1, NULL, 0, NULL, FALSE);
	if (0 == dwMinSize)
	{
		return FALSE;
	}
	lpszStr = new char[dwMinSize];
	WideCharToMultiByte(CP_OEMCP, NULL, lpcwszStr, -1, lpszStr, dwMinSize, NULL, FALSE);
	str = lpszStr;
	delete[] lpszStr;
	lpszStr = NULL;
	return TRUE;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~��������ɫ�����벿��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_basic::shader_basic(LPCWSTR filename)
{
	fx_need = NULL;
	shader_filename = filename;
	WCharToMByte(filename, shader_file_string);
}
engine_basic::engine_fail_reason shader_basic::shder_create()
{
	engine_basic::engine_fail_reason check_error = combile_shader(shader_filename);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	init_handle();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_basic::release_basic()
{
	safe_release(fx_need);
}
engine_basic::engine_fail_reason shader_basic::get_technique(ID3DX11EffectTechnique** tech_need, LPCSTR tech_name)
{
	D3D11_INPUT_ELEMENT_DESC member_point[30];
	UINT num_member;
	set_inputpoint_desc(member_point, &num_member);
	*tech_need = fx_need->GetTechniqueByName(tech_name);
	D3DX11_PASS_DESC pass_shade;
	HRESULT hr;
	hr = (*tech_need)->GetPassByIndex(0)->GetDesc(&pass_shade);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_message("get technique" + std::string(tech_name) + "error in" + shader_file_string);
		return failed_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateInputLayout(member_point, num_member, pass_shade.pIAInputSignature, pass_shade.IAInputSignatureSize, &input_need);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetInputLayout(input_need);
	input_need->Release();
	input_need = NULL;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_basic::get_technique(D3D11_INPUT_ELEMENT_DESC member_point[], UINT num_member, ID3DX11EffectTechnique** tech_need, LPCSTR tech_name)
{
	*tech_need = fx_need->GetTechniqueByName(tech_name);
	D3DX11_PASS_DESC pass_shade;
	HRESULT hr2;
	hr2 = (*tech_need)->GetPassByIndex(0)->GetDesc(&pass_shade);
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateInputLayout(member_point, num_member, pass_shade.pIAInputSignature, pass_shade.IAInputSignatureSize, &input_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_message( "get technique" + std::string(tech_name) + "error in"+ shader_file_string);
		return failed_message;
	}
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetInputLayout(input_need);
	input_need->Release();
	input_need = NULL;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_basic::combile_shader(LPCWSTR filename)
{
	//����shader
	UINT flag_need(0);
	flag_need |= D3D10_SHADER_SKIP_OPTIMIZATION;
#if defined(DEBUG) || defined(_DEBUG)
	flag_need |= D3D10_SHADER_DEBUG;
#endif
	//����ID3D10Blob������ű���õ�shader��������Ϣ
	ID3D10Blob	*shader(NULL);
	ID3D10Blob	*errMsg(NULL);
	//����effect
	std::ifstream fin(filename, std::ios::binary);
	if (fin.fail())
	{
		engine_basic::engine_fail_reason failed_message("open shader file"+ shader_file_string + "error");
		return failed_message;
	}
	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);
	fin.read(&compiledShader[0], size);
	fin.close();
	HRESULT hr = D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), &fx_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_message("Create Effect From" + shader_file_string + "error");
		return failed_message;
	}
	safe_release(shader);
	//�������붥���ʽ
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_basic::set_matrix(ID3DX11EffectMatrixVariable *mat_handle, XMFLOAT4X4 *mat_need)
{
	XMMATRIX rec_mat = XMLoadFloat4x4(mat_need);
	HRESULT hr;
	hr = mat_handle->SetMatrix(reinterpret_cast<float*>(&rec_mat));
	if (FAILED(hr)) 
	{
		engine_basic::engine_fail_reason failed_message("set matrix error in" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~���δ���~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
terrain_shader_basic::terrain_shader_basic(LPCWSTR filename) : shader_basic(filename)
{
}
engine_basic::engine_fail_reason terrain_shader_basic::set_texture_height(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr = tex_height_handle->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting terrain height map");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason terrain_shader_basic::set_texture_normal(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr = tex_normalt_handle->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting terrain normal map");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason terrain_shader_basic::set_texture_blend(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr = tex_blend_handle->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting terrain tangent map");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason terrain_shader_basic::set_texture_tangnt(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr = tex_tangnt_handle->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting terrain blend map");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason terrain_shader_basic::set_terrain_size(float world_size, float texture_size, float height_scal)
{
	XMFLOAT4 terrain_size_data(world_size, texture_size, height_scal, 0.0f);
	HRESULT hr = terrain_size->SetRawValue((void*)&terrain_size_data, 0, sizeof(terrain_size_data));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set terrain size error in terrain shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason terrain_shader_basic::set_view_pos(XMFLOAT3 eye_pos)
{
	HRESULT hr = view_pos_handle->SetRawValue((void*)&eye_pos, 0, sizeof(eye_pos));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "terrain draw error when setting view position");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason terrain_shader_basic::set_texture_color(terrain_color_resource tex_color_in[4])
{
	for (int i = 0; i < 4; ++i)
	{
		tex_MaterialArray_handle[i].terrain_color_albedo_handle->SetResource(tex_color_in[i].terrain_color_albedo_tex);
		tex_MaterialArray_handle[i].terrain_color_normal_handle->SetResource(tex_color_in[i].terrain_color_normal_tex);
		tex_MaterialArray_handle[i].terrain_color_metallic_handle->SetResource(tex_color_in[i].terrain_color_metallic_tex);
		tex_MaterialArray_handle[i].terrain_color_roughness_handle->SetResource(tex_color_in[i].terrain_color_roughness_tex);
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void terrain_shader_basic::init_handle_terrain()
{
	terrain_size = fx_need->GetVariableByName("terrain_size");
	view_pos_handle = fx_need->GetVariableByName("eye_pos");
	tex_height_handle = fx_need->GetVariableByName("terrain_height")->AsShaderResource();
	tex_normalt_handle = fx_need->GetVariableByName("terrain_normal")->AsShaderResource();
	tex_tangnt_handle = fx_need->GetVariableByName("terrain_tangent")->AsShaderResource();
	tex_blend_handle = fx_need->GetVariableByName("terrain_blend")->AsShaderResource();
	for (int i = 0; i < 4; ++i)
	{
		std::string now_tex_turn = "_";
		now_tex_turn += '0' + i;
		tex_MaterialArray_handle[i].terrain_color_albedo_handle = fx_need->GetVariableByName(("ColorTexture_pack_albedo" + now_tex_turn).c_str())->AsShaderResource();
		tex_MaterialArray_handle[i].terrain_color_normal_handle = fx_need->GetVariableByName(("ColorTexture_pack_normal" + now_tex_turn).c_str())->AsShaderResource();
		tex_MaterialArray_handle[i].terrain_color_metallic_handle = fx_need->GetVariableByName(("ColorTexture_pack_metallic" + now_tex_turn).c_str())->AsShaderResource();
		tex_MaterialArray_handle[i].terrain_color_roughness_handle = fx_need->GetVariableByName(("ColorTexture_pack_roughness" + now_tex_turn).c_str())->AsShaderResource();
	}
	//tex_ColorArray_handle = fx_need->GetVariableByName("ColorTexture_pack_albedo")->AsShaderResource();
	//tex_NormalArray_handle = fx_need->GetVariableByName("ColorTexture_pack_normal")->AsShaderResource();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ֲ������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
plant_shader_basic::plant_shader_basic(LPCWSTR filename) : shader_basic(filename)
{
}
engine_basic::engine_fail_reason plant_shader_basic::set_animation_buffer(ID3D11ShaderResourceView* buffer_in)
{
	HRESULT hr = animation_buffer->SetResource(buffer_in);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason failed_message(hr, "set animation_buffer error in" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason plant_shader_basic::set_animation_offset(XMUINT4 offset_data)
{
	HRESULT hr = point_offset_handle->SetRawValue((void*)&offset_data, 0, sizeof(offset_data));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason failed_message(hr, "set point_offset_handle error in" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason plant_shader_basic::set_animation_offset_array(XMUINT4 *offset_data, int array_num)
{
	HRESULT hr = point_offset_handle->SetRawValue((void*)offset_data, 0, array_num * sizeof(offset_data));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason failed_message(hr, "set point_offset_array_handle error in" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void plant_shader_basic::init_handle_plant()
{
	animation_buffer = fx_need->GetVariableByName("input_point")->AsShaderResource();
	point_offset_handle = fx_need->GetVariableByName("offset_num");
	point_offset_array_handle = fx_need->GetVariableByName("offset_num_list");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~��ɫ����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
color_shader::color_shader(LPCWSTR filename) : shader_basic(filename)
{
}
void color_shader::init_handle()
{
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();
}
engine_basic::engine_fail_reason color_shader::set_trans_all(XMFLOAT4X4 *mat_final)
{
	engine_basic::engine_fail_reason check_fail = set_matrix(project_matrix_handle, mat_final);
	if (!check_fail.check_if_failed())
	{
		engine_basic::engine_fail_reason failed_message("set final matrix error in" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void color_shader::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT" ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
void color_shader::release()
{
	release_basic();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~��α����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
virtual_light_shader::virtual_light_shader(LPCWSTR filename) : shader_basic(filename)
{
}
void virtual_light_shader::init_handle()
{
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();
	normal_matrix_handle = fx_need->GetVariableByName("normal_matrix")->AsMatrix();
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();
	ssao_matrix_handle = fx_need->GetVariableByName("ssao_matrix")->AsMatrix();             //ssao����任���

	texture_diffuse_handle = fx_need->GetVariableByName("texture_diffuse")->AsShaderResource();
	texture_normal_handle = fx_need->GetVariableByName("texture_specular")->AsShaderResource();
	texture_specular_handle = fx_need->GetVariableByName("texturet_normal")->AsShaderResource();
	texture_ssao_handle = fx_need->GetVariableByName("texture_ssao")->AsShaderResource();        //��������ͼ���

	texture_diffusearray_handle = fx_need->GetVariableByName("texture_pack_diffuse")->AsShaderResource();
}
engine_basic::engine_fail_reason virtual_light_shader::set_tex_diffuse_array(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_diffusearray_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason failed_message(hr, "set diffuse texarray error in" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason virtual_light_shader::set_trans_world(XMFLOAT4X4 *mat_world)
{
	//����任
	engine_basic::engine_fail_reason check_fail = set_matrix(world_matrix_handle, mat_world);
	if (!check_fail.check_if_failed())
	{
		engine_basic::engine_fail_reason failed_message("set world matrix error in" + shader_file_string);
		return failed_message;
	}
	//���߱任
	XMMATRIX rec_mat = XMLoadFloat4x4(mat_world);
	XMVECTOR x_delta;
	XMMATRIX check = rec_mat;
	//���߱任
	XMMATRIX normal_need = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&x_delta, check));
	XMFLOAT4X4 mat_normal;
	XMStoreFloat4x4(&mat_normal, normal_need);
	mat_normal._41 = 0.0f;
	mat_normal._42 = 0.0f;
	mat_normal._43 = 0.0f;
	mat_normal._44 = 1.0f;
	check_fail = set_matrix(normal_matrix_handle, &mat_normal);
	if (!check_fail.check_if_failed())
	{
		engine_basic::engine_fail_reason failed_message("set normal matrix error in" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason virtual_light_shader::set_trans_all(XMFLOAT4X4 *mat_final)
{
	engine_basic::engine_fail_reason check_fail = set_matrix(project_matrix_handle, mat_final);
	if (!check_fail.check_if_failed())
	{
		engine_basic::engine_fail_reason failed_message("set final matrix error in" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason virtual_light_shader::set_trans_ssao(XMFLOAT4X4 *mat_need)
{
	engine_basic::engine_fail_reason check_error = set_matrix(ssao_matrix_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason virtual_light_shader::set_ssaotex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_ssao_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason failed_message("set ssao texture error in" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason virtual_light_shader::set_tex_diffuse(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_diffuse_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason failed_message(hr,"set diffuse tex error in" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason virtual_light_shader::set_tex_normal(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_normal_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason failed_message(hr, "set diffuse tex error in" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason virtual_light_shader::set_tex_specular(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_specular_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason failed_message(hr, "set diffuse tex error in" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void virtual_light_shader::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT" ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXDIFFNORM",0  ,DXGI_FORMAT_R32G32B32A32_FLOAT      ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXOTHER",0  ,DXGI_FORMAT_R32G32B32A32_FLOAT      ,0    ,68 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
void virtual_light_shader::release()
{
	release_basic();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~��ʾͼ��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
picture_show_shader::picture_show_shader(LPCWSTR filename) : shader_basic(filename)
{
}
void picture_show_shader::release()
{
	release_basic();
}
void picture_show_shader::init_handle()
{
	UI_scal_handle = fx_need->GetVariableByName("UI_scal");
	UI_position_handle = fx_need->GetVariableByName("UI_pos");
	tex_color_input = fx_need->GetVariableByName("texture_need")->AsShaderResource();
}
void picture_show_shader::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
engine_basic::engine_fail_reason picture_show_shader::set_tex_color_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_color_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_message(hr, "set UI_comman color texture error" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason picture_show_shader::set_UI_scal(XMFLOAT4 range)
{
	HRESULT hr;
	hr = UI_scal_handle->SetRawValue((void*)&range, 0, sizeof(range));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_message(hr, "set UI scal handle error" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason picture_show_shader::set_UI_position(XMFLOAT4 range)
{
	HRESULT hr;
	hr = UI_position_handle->SetRawValue((void*)&range, 0, sizeof(range));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_message(hr, "set UI position handle error" + shader_file_string);
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~gbuffer��¼~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_pretreat_gbuffer::shader_pretreat_gbuffer(LPCWSTR filename) :shader_basic(filename), plant_shader_basic(filename),terrain_shader_basic(filename)
{
}
void shader_pretreat_gbuffer::init_handle()
{
	init_handle_terrain();
	init_handle_plant();
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();
	normal_matrix_handle = fx_need->GetVariableByName("normal_matrix")->AsMatrix();
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();
	texture_packarray_handle = fx_need->GetVariableByName("texture_pack_array")->AsShaderResource();
	world_matrix_array_handle = fx_need->GetVariableByName("world_matrix_array")->AsMatrix();//����任�����
	normal_matrix_array_handle = fx_need->GetVariableByName("normal_matrix_array")->AsMatrix();//���߱任�����
	proj_matrix_handle = fx_need->GetVariableByName("proj_matrix")->AsMatrix();   //ȡ��*ͶӰ�任����
	view_matrix_handle = fx_need->GetVariableByName("view_matrix")->AsMatrix();   //ȡ���任����
	BoneTransforms = fx_need->GetVariableByName("gBoneTransforms")->AsMatrix();
	bone_matrix_buffer = fx_need->GetVariableByName("input_buffer")->AsShaderResource();
	bone_num_handle = fx_need->GetVariableByName("bone_num");
}
engine_basic::engine_fail_reason shader_pretreat_gbuffer::set_trans_world(XMFLOAT4X4 *mat_world, XMFLOAT4X4 *mat_view)
{
	HRESULT hr;
	auto check_error = set_matrix(view_matrix_handle, mat_view);
	if (!check_error.check_if_failed()) 
	{
		return check_error;
	}
	
	XMVECTOR x_delta;
	XMMATRIX world_need = XMLoadFloat4x4(mat_world);
	XMMATRIX view_need = XMLoadFloat4x4(mat_view);
	XMMATRIX normal_need = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&x_delta, world_need));
	normal_need.r[0].m128_f32[3] = 0;
	normal_need.r[1].m128_f32[3] = 0;
	normal_need.r[2].m128_f32[3] = 0;
	normal_need.r[3].m128_f32[3] = 1;
	//hr = world_matrix_handle->SetMatrix(reinterpret_cast<float*>(&(world_need*view_need)));
	hr = world_matrix_handle->SetMatrix(reinterpret_cast<float*>(&(world_need)));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set world matrix error in gbuffer depthnormal part");
		return error_message;
	}
	//hr = normal_matrix_handle->SetMatrix(reinterpret_cast<float*>(&(normal_need*view_need)));
	hr = normal_matrix_handle->SetMatrix(reinterpret_cast<float*>(&(normal_need)));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set normal matrix error in gbuffer depthnormal part");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_pretreat_gbuffer::set_trans_all(XMFLOAT4X4 *mat_final)
{
	engine_basic::engine_fail_reason check_error = set_matrix(project_matrix_handle, mat_final);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_pretreat_gbuffer::set_trans_proj(XMFLOAT4X4 *mat_need)
{
	engine_basic::engine_fail_reason check_error = set_matrix(proj_matrix_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_pretreat_gbuffer::set_texturepack_array(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr;
	hr = texture_packarray_handle->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_pack error in gbuffer depthnormal part");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_pretreat_gbuffer::set_world_matrix_array(const XMFLOAT4X4* M, XMFLOAT4X4 mat_view,int cnt)
{
	XMFLOAT4X4 *world_mat = new XMFLOAT4X4[cnt];
	XMFLOAT4X4 *normal_mat = new XMFLOAT4X4[cnt];
	auto check_error = set_matrix(view_matrix_handle, &mat_view);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	for (int i = 0; i < cnt; ++i) 
	{
		XMVECTOR x_delta;
		XMMATRIX world_need = XMLoadFloat4x4(&M[i]);
		XMMATRIX view_need = XMLoadFloat4x4(&mat_view);
		XMMATRIX normal_need = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&x_delta, world_need));
		normal_need.r[0].m128_f32[3] = 0;
		normal_need.r[1].m128_f32[3] = 0;
		normal_need.r[2].m128_f32[3] = 0;
		normal_need.r[3].m128_f32[3] = 1;

		//XMStoreFloat4x4(&world_mat[i], world_need * view_need);
		//XMStoreFloat4x4(&normal_mat[i], normal_need * view_need);
		XMStoreFloat4x4(&world_mat[i], world_need);
		XMStoreFloat4x4(&normal_mat[i], normal_need);
	}
	
	HRESULT hr = world_matrix_array_handle->SetMatrixArray(reinterpret_cast<const float*>(world_mat), 0, cnt);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set world_matrix_array error in gbuffer depthnormal part");
		return error_message;
	}
	hr = normal_matrix_array_handle->SetMatrixArray(reinterpret_cast<const float*>(normal_mat), 0, cnt);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set normal_matrix_array error in gbuffer depthnormal part");
		return error_message;
	}
	delete[] world_mat;
	delete[] normal_mat;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_pretreat_gbuffer::set_bone_matrix(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when setting bone_matrix");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_pretreat_gbuffer::set_bonemat_buffer(ID3D11ShaderResourceView *buffer_in)
{
	HRESULT hr;
	hr = bone_matrix_buffer->SetResource(buffer_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set bone_matrix_buffer error in gbuffer");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_pretreat_gbuffer::set_bone_num(UINT bone_num)
{
	HRESULT hr = bone_num_handle->SetRawValue((void*)&bone_num, 0, sizeof(bone_num));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "gbuffer error when setting bone_num_handle");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_pretreat_gbuffer::release()
{
	release_basic();
}
void shader_pretreat_gbuffer::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION"   ,0  ,DXGI_FORMAT_R32G32B32_FLOAT     ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"     ,0  ,DXGI_FORMAT_R32G32B32_FLOAT     ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT"    ,0  ,DXGI_FORMAT_R32G32B32_FLOAT     ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT   ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXDIFFNORM",0  ,DXGI_FORMAT_R32G32B32A32_FLOAT  ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXOTHER"   ,0  ,DXGI_FORMAT_R32G32B32A32_FLOAT  ,0    ,68 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~����ز���MSAA->����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_resolvedepth::shader_resolvedepth(LPCWSTR filename) :shader_basic(filename)
{
}
engine_basic::engine_fail_reason shader_resolvedepth::set_texture_MSAA(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_MSAA->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set texture_MSAA error in resolve MSAA depth texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_resolvedepth::set_projmessage(XMFLOAT3 proj_message)
{
	HRESULT hr = projmessage_handle->SetRawValue((void*)&proj_message, 0, sizeof(proj_message));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "set set_projmessage error in resolve MSAA depth texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_resolvedepth::set_window_size(float width, float height) 
{
	XMFLOAT4 window_size_data = XMFLOAT4(width, height,0.0f,0.0f);
	HRESULT hr = window_size->SetRawValue((void*)&window_size_data, 0, sizeof(window_size_data));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "set window size error in resolve MSAA depth texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_resolvedepth::release()
{
	release_basic();
}
void shader_resolvedepth::init_handle()
{
	//������Ϣ���
	texture_MSAA = fx_need->GetVariableByName("gdepth_map")->AsShaderResource();
	//���α任��Ϣ���
	projmessage_handle = fx_need->GetVariableByName("proj_desc");
	//�������ڴ�С
	window_size = fx_need->GetVariableByName("window_size");
}
void shader_resolvedepth::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao �ڱ���Ⱦ����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_ssaomap::shader_ssaomap(LPCWSTR filename) :shader_basic(filename)
{
}
void shader_ssaomap::init_handle()
{
	ViewToTexSpace = fx_need->GetVariableByName("gViewToTexSpace")->AsMatrix();
	OffsetVectors = fx_need->GetVariableByName("gOffsetVectors")->AsVector();
	FrustumCorners = fx_need->GetVariableByName("gFrustumCorners")->AsVector();

	NormalDepthMap = fx_need->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	DepthMap = fx_need->GetVariableByName("gdepth_map")->AsShaderResource();
	RandomVecMap = fx_need->GetVariableByName("gRandomVecMap")->AsShaderResource();
}
void shader_ssaomap::release()
{
	release_basic();
}
engine_basic::engine_fail_reason shader_ssaomap::set_ViewToTexSpace(XMFLOAT4X4 *mat)
{
	engine_basic::engine_fail_reason check_error = set_matrix(ViewToTexSpace, mat);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ssaomap::set_FrustumCorners(const XMFLOAT4 v[4])
{
	HRESULT hr = FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set FrustumCorners error in ssao draw part");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ssaomap::set_OffsetVectors(const XMFLOAT4 v[14])
{
	HRESULT hr = OffsetVectors->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 14);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set OffsetVectors error in ssao draw part");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ssaomap::set_NormalDepthtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = NormalDepthMap->SetResource(srv);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set Normalspectex error in ssao draw part");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ssaomap::set_Depthtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = DepthMap->SetResource(srv);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set depthmap error in ssao draw part");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ssaomap::set_randomtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = RandomVecMap->SetResource(srv);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set randomtex error in ssao draw part");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_ssaomap::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao aoͼģ��������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_ssaoblur::shader_ssaoblur(LPCWSTR filename) :shader_basic(filename)
{
}
void shader_ssaoblur::release()
{
	release_basic();
}
void shader_ssaoblur::init_handle()
{
	TexelWidth = fx_need->GetVariableByName("gTexelWidth")->AsScalar();
	TexelHeight = fx_need->GetVariableByName("gTexelHeight")->AsScalar();

	NormalDepthMap = fx_need->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	DepthMap = fx_need->GetVariableByName("gdepth_map")->AsShaderResource();
	InputImage = fx_need->GetVariableByName("gInputImage")->AsShaderResource();
}
engine_basic::engine_fail_reason shader_ssaoblur::set_image_size(float width, float height)
{
	HRESULT hr;
	hr = TexelWidth->SetFloat(width);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set image_size error in ssao blur part");
		return error_message;
	}
	hr = TexelHeight->SetFloat(height);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set image_size error in ssao blur part");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ssaoblur::set_tex_resource(ID3D11ShaderResourceView* tex_normaldepth, ID3D11ShaderResourceView* tex_aomap)
{
	HRESULT hr;
	hr = NormalDepthMap->SetResource(tex_normaldepth);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_normaldepth error in ssao blur part");
		return error_message;
	}
	hr = InputImage->SetResource(tex_aomap);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_aomap error in ssao blur part");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ssaoblur::set_Depthtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = DepthMap->SetResource(srv);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set Depthtex error in ssao draw part");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_ssaoblur::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~shadow map����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
light_shadow::light_shadow(LPCWSTR filename) : shader_basic(filename)
{
}
engine_basic::engine_fail_reason light_shadow::set_trans_all(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(project_matrix_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_shadow::set_trans_viewproj(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(viewproj_matrix_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_shadow::set_world_matrix_array(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = world_matrix_array_handle->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set instance world matrix array error in shadow map part");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_shadow::set_texturepack_array(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr;
	hr = texture_need->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_normaldepth error in shadow map part");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_shadow::set_bone_matrix(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when setting bone_matrix");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_shadow::set_bonemat_buffer(ID3D11ShaderResourceView *buffer_in)
{
	HRESULT hr;
	hr = bone_matrix_buffer->SetResource(buffer_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set bone_matrix_buffer error in gbuffer");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_shadow::set_bone_num(UINT bone_num)
{
	HRESULT hr = bone_num_handle->SetRawValue((void*)&bone_num, 0, sizeof(bone_num));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "gbuffer error when setting bone_num_handle");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void light_shadow::init_handle()
{
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();         //ȫ�׼��α任���
	texture_need = fx_need->GetVariableByName("texture_pack_array")->AsShaderResource();
	world_matrix_array_handle = fx_need->GetVariableByName("world_matrix_array")->AsMatrix();//����任�����
	viewproj_matrix_handle = fx_need->GetVariableByName("view_proj_matrix")->AsMatrix();   //ȡ��*ͶӰ�任����
	BoneTransforms = fx_need->GetVariableByName("gBoneTransforms")->AsMatrix();
	bone_matrix_buffer = fx_need->GetVariableByName("input_buffer")->AsShaderResource();
	bone_num_handle = fx_need->GetVariableByName("bone_num");
}
void light_shadow::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT" ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXDIFFNORM",0  ,DXGI_FORMAT_R32G32B32A32_FLOAT      ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXOTHER",0  ,DXGI_FORMAT_R32G32B32A32_FLOAT      ,0    ,68 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
void light_shadow::release()
{
	release_basic();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~�ӳٹ����㷨���ջ�����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
light_defered_lightbuffer::light_defered_lightbuffer(LPCWSTR filename) :shader_basic(filename)
{
}

engine_basic::engine_fail_reason light_defered_lightbuffer::set_sunlight(pancy_light_basic light_need)
{
	HRESULT hr = light_sun->SetRawValue(&light_need, 0, sizeof(light_need));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when setting sunlight");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_sunshadow_matrix(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = sunshadow_matrix_handle->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when sunshadow_matrix");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_sunlight_num(XMUINT3 all_light_num)
{
	HRESULT hr = sunlight_num->SetRawValue((void*)&all_light_num, 0, sizeof(all_light_num));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when setting light num");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_sunshadow_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = suntexture_shadow->SetResource(tex_in);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when setting sunshadow tex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_depth_devide(XMFLOAT4 v)
{
	HRESULT hr = depth_devide->SetRawValue((void*)&v, 0, sizeof(v));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when setting depth divide");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason light_defered_lightbuffer::set_light(pancy_light_basic light_need, int light_num)
{
	HRESULT hr = light_list->SetRawValue(&light_need, light_num * sizeof(light_need), sizeof(light_need));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when setting light");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_FrustumCorners(const XMFLOAT4 v[4])
{
	HRESULT hr = FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when setting FrustumCorners");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_projmessage(XMFLOAT3 proj_message)
{
	HRESULT hr = projmessage_handle->SetRawValue((void*)&proj_message, 0, sizeof(proj_message));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "set set_projmessage error in lightbuffer shadere");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_shadow_matrix(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = shadow_matrix_handle->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when setting shadow_matrix");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_view_matrix(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(view_matrix_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_invview_matrix(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(invview_matrix_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason light_defered_lightbuffer::set_light_num(XMUINT3 all_light_num)
{
	HRESULT hr = light_num_handle->SetRawValue((void*)&all_light_num, 0, sizeof(all_light_num));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when setting light num");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_shadow_num(XMUINT3 all_light_num)
{
	HRESULT hr = shadow_num_handle->SetRawValue((void*)&all_light_num, 0, sizeof(all_light_num));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when setting shadow num");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_Normalspec_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = NormalspecMap->SetResource(tex_in);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when setting Normalspec_tex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_SpecRoughness_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = SpecRoughnessMap->SetResource(tex_in);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when setting SpecRoughness_tex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_DepthMap_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = DepthMap->SetResource(tex_in);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when setting DepthMap_tex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_shadow_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_shadow->SetResource(tex_in);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "lightbuffer shader error when setting shadow_tex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

//��������
engine_basic::engine_fail_reason light_defered_lightbuffer::set_tex_transmittance(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = transmittance_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set transmittance texture error in lbuffer atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_tex_scattering(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = scattering_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set scattering texture error in lbuffer atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_tex_single_mie_scattering(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = single_mie_scattering_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set single_mie_scattering texture error in lbuffer atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_tex_irradiance(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = irradiance_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set irradiance texture error in lbuffer atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_tex_mask(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = mask_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set mask texture error in lbuffer atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}


//��������
engine_basic::engine_fail_reason light_defered_lightbuffer::set_view_from_clip(XMFLOAT4X4 view_from_clip_in)
{
	auto check_error = set_matrix(view_from_clip, &view_from_clip_in);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
//������������
engine_basic::engine_fail_reason light_defered_lightbuffer::set_white_point(XMFLOAT3 white_point_in)
{
	HRESULT hr = white_point->SetRawValue((void*)&white_point_in, 0, sizeof(white_point_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set white_point_in error in lbuffer atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_earth_center(XMFLOAT3 earth_center_in)
{
	HRESULT hr = earth_center->SetRawValue((void*)&earth_center_in, 0, sizeof(earth_center_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set earth_center_in error in lbuffer atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_sun_size(XMFLOAT2 sun_size_in)
{
	HRESULT hr = sun_size->SetRawValue((void*)&sun_size_in, 0, sizeof(sun_size_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set sun_size error in lbuffer atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_camera(XMFLOAT3 camera_in)
{
	HRESULT hr = camera->SetRawValue((void*)&camera_in, 0, sizeof(camera_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set camera error in lbuffer atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_lightbuffer::set_exposure(float exposure_in)
{
	HRESULT hr = exposure->SetRawValue((void*)&exposure_in, 0, sizeof(exposure_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set exposure error in lbuffer atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

void light_defered_lightbuffer::release()
{
	release_basic();
}
void light_defered_lightbuffer::init_handle()
{
	//����ɢ����Ϣ
	white_point = fx_need->GetVariableByName("white_point_in");//��ƽ�����
	earth_center = fx_need->GetVariableByName("earth_center");//3d�����
	sun_size = fx_need->GetVariableByName("sun_size");//3d�����
	camera = fx_need->GetVariableByName("camera");//�����λ��
	exposure = fx_need->GetVariableByName("exposure");//��ƽ���ع����
	view_from_clip = fx_need->GetVariableByName("view_from_clip")->AsMatrix();//��դ����->ȡ���ռ�任
	transmittance_texture = fx_need->GetVariableByName("transmittance_texture")->AsShaderResource();//͸��������
	scattering_texture = fx_need->GetVariableByName("scattering_texture")->AsShaderResource();//����ɢ��
	single_mie_scattering_texture = fx_need->GetVariableByName("single_mie_scattering_texture")->AsShaderResource();//��������ɢ��
	irradiance_texture = fx_need->GetVariableByName("irradiance_texture")->AsShaderResource();//��������
	mask_texture = fx_need->GetVariableByName("render_mask")->AsShaderResource();
	//̫��������
	light_sun = fx_need->GetVariableByName("sun_light");                  //̫����
	sunlight_num = fx_need->GetVariableByName("sun_light_num");               //̫����ּ�����
	depth_devide = fx_need->GetVariableByName("depth_devide");               //ÿһ�������
	suntexture_shadow = fx_need->GetVariableByName("texture_sunshadow")->AsShaderResource();          //̫������Ӱ������Դ���
	sunshadow_matrix_handle = fx_need->GetVariableByName("sunlight_shadowmat")->AsMatrix();    //̫������Ӱͼ�任
																							   //��ͨ��Դ����
	shadow_matrix_handle = fx_need->GetVariableByName("shadowmap_matrix")->AsMatrix();//��Ӱ�任���
	view_matrix_handle = fx_need->GetVariableByName("view_matrix")->AsMatrix();       //ȡ���任���	
	invview_matrix_handle = fx_need->GetVariableByName("invview_matrix")->AsMatrix(); //ȡ���任��任���
	light_list = fx_need->GetVariableByName("light_need");                            //���վ��
	light_num_handle = fx_need->GetVariableByName("light_num");                       //��Դ�������
	shadow_num_handle = fx_need->GetVariableByName("shadow_num");                     //��Ӱ�������
	FrustumCorners = fx_need->GetVariableByName("gFrustumCorners")->AsVector();       //3D��ԭ�ǵ���
																					  //���α任��Ϣ���
	projmessage_handle = fx_need->GetVariableByName("proj_desc");
	NormalspecMap = fx_need->GetVariableByName("gNormalspecMap")->AsShaderResource();  //shader�е�������Դ���
	SpecRoughnessMap = fx_need->GetVariableByName("gSpecRoughnessMap")->AsShaderResource();  //shader�е�������Դ���
	DepthMap = fx_need->GetVariableByName("gdepth_map")->AsShaderResource();  //shader�е�������Դ���
	texture_shadow = fx_need->GetVariableByName("texture_shadow")->AsShaderResource();  //shader�е�������Դ���
	
}
void light_defered_lightbuffer::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~�ӳٹ����㷨������Ⱦ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
light_defered_draw::light_defered_draw(LPCWSTR filename) : shader_basic(filename), plant_shader_basic(filename), terrain_shader_basic(filename)
{
}
/*
engine_basic::engine_fail_reason light_defered_draw::set_view_pos(XMFLOAT3 eye_pos)
{
	HRESULT hr = view_pos_handle->SetRawValue((void*)&eye_pos, 0, sizeof(eye_pos));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when setting view position");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
*/
engine_basic::engine_fail_reason light_defered_draw::set_trans_world(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(world_matrix_handle, mat_need);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_trans_view(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(view_matrix_handle, mat_need);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_trans_invview(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(invview_matrix_handle, mat_need);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_trans_ssao(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(ssao_matrix_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_trans_all(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(final_matrix_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_trans_viewproj(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(viewproj_matrix_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_bone_matrix(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when setting bone_matrix");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_world_matrix_array(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = world_matrix_array_handle->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when setting world_matrix");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_material(pancy_material material_in)
{
	HRESULT hr = material_need->SetRawValue(&material_in, 0, sizeof(material_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when set material");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_ssaotex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_ssao_handle->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when set ssaotex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_tex_diffuse_array(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_diffuse_handle->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when set diffusetex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_diffuse_light_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = tex_light_diffuse_handle->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when set diffuse light tex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_specular_light_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = tex_light_specular_handle->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when set specular light tex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_normal_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_normal_handle->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when set normal tex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_tex_specroughness_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_specroughness->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_specroughness input error in globel reflect final shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_tex_brdflist_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_brdf_list->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_brdf_list input error in globel reflect final shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_IBL_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_ibl_handle->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when set IBL tex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_IBL_diffuse_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_ibl_diffuse_handle->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when set IBL diffuse tex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_tex_atmosphere_occlusion(ID3D11ShaderResourceView* tex_in)
{
	HRESULT hr;
	hr = atomosphere_occlusion->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "cubemap shader error when setting atomosphere texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_tex_atmosphere_fog(ID3D11ShaderResourceView* tex_in)
{
	HRESULT hr;
	hr = atomosphere_fog->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "cubemap shader error when setting atomosphere fog texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason light_defered_draw::set_bonemat_buffer(ID3D11ShaderResourceView *buffer_in)
{
	HRESULT hr;
	hr = bone_matrix_buffer->SetResource(buffer_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set bone_matrix_buffer error in deffered drawing");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_defered_draw::set_bone_num(UINT bone_num)
{
	HRESULT hr = bone_num_handle->SetRawValue((void*)&bone_num, 0, sizeof(bone_num));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "deffered light draw error when setting bone_num_handle");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void light_defered_draw::release()
{
	release_basic();
}
void light_defered_draw::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT" ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXDIFFNORM",0  ,DXGI_FORMAT_R32G32B32A32_FLOAT      ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXOTHER",0  ,DXGI_FORMAT_R32G32B32A32_FLOAT      ,0    ,68 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
void light_defered_draw::init_handle()
{
	init_handle_terrain();
	init_handle_plant();
	atomosphere_fog = fx_need->GetVariableByName("fog_color_tex")->AsShaderResource();
	atomosphere_occlusion = fx_need->GetVariableByName("atmosphere_occlusion")->AsShaderResource();
	texture_ibl_handle = fx_need->GetVariableByName("IBL_cube")->AsShaderResource();
	texture_ibl_diffuse_handle = fx_need->GetVariableByName("IBL_diffuse")->AsShaderResource();
	tex_specroughness = fx_need->GetVariableByName("gInputspecular_roughness")->AsShaderResource();
	tex_brdf_list = fx_need->GetVariableByName("gInputbrdf")->AsShaderResource();
	texture_normal_handle = fx_need->GetVariableByName("gNormalspecMap")->AsShaderResource();
	texture_diffuse_handle = fx_need->GetVariableByName("texture_pack_array")->AsShaderResource();
	tex_light_diffuse_handle = fx_need->GetVariableByName("texture_light_diffuse")->AsShaderResource();
	tex_light_specular_handle = fx_need->GetVariableByName("texture_light_specular")->AsShaderResource();
	texture_ssao_handle = fx_need->GetVariableByName("texture_ssao")->AsShaderResource();
	//���α任���
	//view_pos_handle = fx_need->GetVariableByName("position_view");
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();
	view_matrix_handle = fx_need->GetVariableByName("view_matrix")->AsMatrix();
	invview_matrix_handle = fx_need->GetVariableByName("invview_matrix")->AsMatrix();
	final_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();
	ssao_matrix_handle = fx_need->GetVariableByName("ssao_matrix")->AsMatrix();
	world_matrix_array_handle = fx_need->GetVariableByName("world_matrix_array")->AsMatrix();
	viewproj_matrix_handle = fx_need->GetVariableByName("view_proj_matrix")->AsMatrix();
	
	material_need = fx_need->GetVariableByName("material_need");

	BoneTransforms = fx_need->GetVariableByName("gBoneTransforms")->AsMatrix();
	bone_matrix_buffer = fx_need->GetVariableByName("input_buffer")->AsShaderResource();
	bone_num_handle = fx_need->GetVariableByName("bone_num");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~��¼cubemap����alpha����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_reflect_save_depth::shader_reflect_save_depth(LPCWSTR filename) :shader_basic(filename)
{
}
engine_basic::engine_fail_reason shader_reflect_save_depth::set_depthtex_input(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = depth_input->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set cube depth error in reflect depth cube save shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_reflect_save_depth::release()
{
	release_basic();
}
void shader_reflect_save_depth::init_handle()
{
	//������Ϣ���
	depth_input = fx_need->GetVariableByName("depth_input")->AsShaderResource();
	cube_count_handle = fx_need->GetVariableByName("cube_count");
}
engine_basic::engine_fail_reason shader_reflect_save_depth::set_cube_count(XMFLOAT3 cube_count)
{
	HRESULT hr = cube_count_handle->SetRawValue((void*)&cube_count, 0, sizeof(cube_count));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "set cube count error in reflect depth cube save shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_reflect_save_depth::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ʵʱȫ�ַ���~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
rtgr_reflect::rtgr_reflect(LPCWSTR filename) :shader_basic(filename)
{
}
void rtgr_reflect::init_handle()
{
	projmessage_handle = fx_need->GetVariableByName("proj_desc");
	view_pos_handle = fx_need->GetVariableByName("view_position");
	view_matrix_handle = fx_need->GetVariableByName("view_matrix")->AsMatrix();       //ȡ���任���	
	ViewToTexSpace = fx_need->GetVariableByName("gViewToTexSpace")->AsMatrix();
	FrustumCorners = fx_need->GetVariableByName("gFrustumCorners")->AsVector();
	invview_matrix_handle = fx_need->GetVariableByName("invview_matrix")->AsMatrix(); //ȡ���任��任���
	cubeview_matrix_handle = fx_need->GetVariableByName("view_matrix_cube")->AsMatrix();
	NormalDepthMap = fx_need->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	DepthMap = fx_need->GetVariableByName("gdepth_map")->AsShaderResource();
	texture_diffuse_handle = fx_need->GetVariableByName("gcolorMap")->AsShaderResource();
	texture_cube_handle = fx_need->GetVariableByName("texture_cube")->AsShaderResource();
	//texture_depthcube_handle = fx_need->GetVariableByName("depth_cube")->AsShaderResource();
	texture_stencilcube_handle = fx_need->GetVariableByName("stencil_cube")->AsShaderResource();
	texture_color_mask = fx_need->GetVariableByName("mask_input")->AsShaderResource();;
	texture_color_ssr = fx_need->GetVariableByName("ssrcolor_input")->AsShaderResource();;
	camera_positions = fx_need->GetVariableByName("center_position")->AsVector();
}
void rtgr_reflect::release()
{
	release_basic();
}
engine_basic::engine_fail_reason rtgr_reflect::set_projmessage(XMFLOAT3 proj_message)
{
	HRESULT hr = projmessage_handle->SetRawValue((void*)&proj_message, 0, sizeof(proj_message));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "set set_projmessage error in rtgr reflect");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_ViewToTexSpace(XMFLOAT4X4 *mat)
{
	auto check_error = set_matrix(ViewToTexSpace, mat);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_view_matrix(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(view_matrix_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_view_pos(XMFLOAT3 eye_pos)
{
	HRESULT hr = view_pos_handle->SetRawValue((void*)&eye_pos, 0, sizeof(eye_pos));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "rtgr reflect error when setting view position");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_FrustumCorners(const XMFLOAT4 v[4])
{
	HRESULT hr = FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "rtgr reflect error when setting FrustumCorners");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_camera_positions(XMFLOAT3 v)
{
	HRESULT hr = camera_positions->SetRawValue((void*)&v, 0, sizeof(v));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "rtgr reflect error when setting view position");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_NormalDepthtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = NormalDepthMap->SetResource(srv);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "rtgr reflect error when setting NormalDepthtex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_Depthtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = DepthMap->SetResource(srv);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "rtgr reflect error when setting Depthtex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_diffusetex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_diffuse_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "rtgr reflect error when setting diffuse texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_enviroment_tex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = texture_cube_handle->SetResource(srv);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "rtgr reflect error when setting cube texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_color_mask_tex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = texture_color_mask->SetResource(srv);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "rtgr reflect error when setting color_mask_tex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_color_ssr_tex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = texture_color_ssr->SetResource(srv);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "rtgr reflect error when setting color_ssr_tex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_enviroment_stencil(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = texture_stencilcube_handle->SetResource(srv);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "rtgr reflect error when setting enviroment_stencil tex");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_invview_matrix(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(invview_matrix_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect::set_cubeview_matrix(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = cubeview_matrix_handle->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "rtgr reflect error when setting cubeview_matrix");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void rtgr_reflect::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~����Ч��ģ��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
rtgr_reflect_blur::rtgr_reflect_blur(LPCWSTR filename) : shader_basic(filename)
{
}
engine_basic::engine_fail_reason rtgr_reflect_blur::set_tex_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set reflect blur texture error in globel reflect blur shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect_blur::set_tex_resource_array(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_input_array->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set reflect blur texture array error in globel reflect blur shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect_blur::set_image_size(XMFLOAT4 range)
{
	HRESULT hr;
	hr = Texelrange->SetRawValue((void*)&range, 0, sizeof(range));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set image_size error in globel reflect blur shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void rtgr_reflect_blur::release()
{
	release_basic();
}
void rtgr_reflect_blur::init_handle()
{
	sample_level_handle = fx_need->GetVariableByName("blur_sample_level");
	Texelrange = fx_need->GetVariableByName("tex_range_color_normal");
	tex_input = fx_need->GetVariableByName("gInputImage")->AsShaderResource();
	tex_input_array = fx_need->GetVariableByName("gInputImagearray")->AsShaderResource();
	tex_normal_input = fx_need->GetVariableByName("normal_tex")->AsShaderResource();
	tex_depth_input = fx_need->GetVariableByName("depth_tex")->AsShaderResource();
	tex_mask_input = fx_need->GetVariableByName("gInputMask")->AsShaderResource();
}
void rtgr_reflect_blur::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
engine_basic::engine_fail_reason rtgr_reflect_blur::set_tex_normal_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_normal_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_normal_resource error in globel reflect blur shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect_blur::set_tex_depth_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_depth_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_depth_resource error in globel reflect blur shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect_blur::set_tex_mask_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_mask_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_mask error in globel reflect blur shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect_blur::set_sample_level(XMUINT4 sample_level)
{
	HRESULT hr;
	hr = sample_level_handle->SetRawValue((void*)&sample_level, 0, sizeof(sample_level));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set sample_level error in globel reflect blur shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~���շ������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
rtgr_reflect_final::rtgr_reflect_final(LPCWSTR filename) : shader_basic(filename)
{
}
void rtgr_reflect_final::release()
{
	release_basic();
}
void rtgr_reflect_final::init_handle()
{
	Texelrange = fx_need->GetVariableByName("tex_range_color_normal");
	tex_color_input = fx_need->GetVariableByName("gInputImage")->AsShaderResource();
	tex_reflect_input = fx_need->GetVariableByName("gInputReflect")->AsShaderResource();

	tex_color_ao = fx_need->GetVariableByName("gInputAO")->AsShaderResource();
	tex_metallic = fx_need->GetVariableByName("gInputnormal_metallic")->AsShaderResource();
	tex_specroughness = fx_need->GetVariableByName("gInputspecular_roughness")->AsShaderResource();
	tex_brdf_list = fx_need->GetVariableByName("gInputbrdf")->AsShaderResource();
	tex_albedo_nov = fx_need->GetVariableByName("gInput_albedo_nov")->AsShaderResource();
}
void rtgr_reflect_final::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
engine_basic::engine_fail_reason rtgr_reflect_final::set_tex_color_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_color_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set color input error in globel reflect final shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect_final::set_tex_reflect_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_reflect_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_reflect_resource input error in globel reflect final shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect_final::set_tex_ao_resource(ID3D11ShaderResourceView *buffer_input) 
{
	HRESULT hr;
	hr = tex_color_ao->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_color_ao input error in globel reflect final shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect_final::set_tex_metallic_resource(ID3D11ShaderResourceView *buffer_input) 
{
	HRESULT hr;
	hr = tex_metallic->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_metallic input error in globel reflect final shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect_final::set_tex_specroughness_resource(ID3D11ShaderResourceView *buffer_input) 
{
	HRESULT hr;
	hr = tex_specroughness->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_specroughness input error in globel reflect final shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect_final::set_tex_brdflist_resource(ID3D11ShaderResourceView *buffer_input) 
{
	HRESULT hr;
	hr = tex_brdf_list->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_brdf_list input error in globel reflect final shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect_final::set_tex_tex_albedonov_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_albedo_nov->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set tex_albedo_nov input error in globel reflect final shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason rtgr_reflect_final::set_image_size(XMFLOAT4 range)
{
	HRESULT hr;
	hr = Texelrange->SetRawValue((void*)&range, 0, sizeof(range));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set image_size input error in globel reflect final shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~���������ӳ��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_skycube::shader_skycube(LPCWSTR filename) : shader_basic(filename)
{
}
engine_basic::engine_fail_reason shader_skycube::set_view_pos(XMFLOAT3 eye_pos)
{
	HRESULT hr = view_pos_handle->SetRawValue((void*)&eye_pos, 0, sizeof(eye_pos));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "cubemap shader error when setting view position");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_skycube::set_trans_world(XMFLOAT4X4 *mat_need)
{
	XMMATRIX rec_mat = XMLoadFloat4x4(mat_need);
	XMVECTOR x_delta;
	XMMATRIX check = rec_mat;
	//���߱任
	XMMATRIX normal_need = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&x_delta, check));
	normal_need.r[0].m128_f32[3] = 0.0f;
	normal_need.r[1].m128_f32[3] = 0.0f;
	normal_need.r[2].m128_f32[3] = 0.0f;
	normal_need.r[3].m128_f32[3] = 1.0f;
	auto check_error = set_matrix(world_matrix_handle, mat_need);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	HRESULT hr = normal_matrix_handle->SetMatrix(reinterpret_cast<float*>(&normal_need));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "cubemap shader error when setting normal matrix");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_skycube::set_trans_all(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(project_matrix_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_skycube::set_trans_texproj(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(texproj_matrix_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_skycube::set_tex_resource(ID3D11ShaderResourceView* tex_cube)
{
	HRESULT hr;
	hr = cubemap_texture->SetResource(tex_cube);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "cubemap shader error when setting cube texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_skycube::set_tex_atmosphere(ID3D11ShaderResourceView* tex_in)
{
	HRESULT hr;
	hr = atomosphere_texture->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "cubemap shader error when setting atomosphere texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_skycube::set_tex_atmosphere_occlusion(ID3D11ShaderResourceView* tex_in)
{
	HRESULT hr;
	hr = atomosphere_occlusion->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "cubemap shader error when setting atomosphere texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_skycube::init_handle()
{
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();         //ȫ�׼��α任���
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();           //����任���
	normal_matrix_handle = fx_need->GetVariableByName("normal_matrix")->AsMatrix();         //���߱任���
	texproj_matrix_handle = fx_need->GetVariableByName("textureproj_matrix")->AsMatrix();         //���߱任���
	view_pos_handle = fx_need->GetVariableByName("position_view");
	cubemap_texture = fx_need->GetVariableByName("texture_cube")->AsShaderResource();  //shader�е�������Դ���
	atomosphere_texture = fx_need->GetVariableByName("atmosphere_mask")->AsShaderResource();
	atomosphere_occlusion = fx_need->GetVariableByName("atmosphere_occlusion")->AsShaderResource();
}
void shader_skycube::release()
{
	release_basic();
}
void shader_skycube::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION"   ,0  ,DXGI_FORMAT_R32G32B32_FLOAT     ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"     ,0  ,DXGI_FORMAT_R32G32B32_FLOAT     ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT"    ,0  ,DXGI_FORMAT_R32G32B32_FLOAT     ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT   ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXDIFFNORM",0  ,DXGI_FORMAT_R32G32B32A32_FLOAT  ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXOTHER"   ,0  ,DXGI_FORMAT_R32G32B32A32_FLOAT  ,0    ,68 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~brdf����ǿ��Ԥ����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
brdf_envpre_shader::brdf_envpre_shader(LPCWSTR filename) : shader_basic(filename)
{
}
void brdf_envpre_shader::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
void brdf_envpre_shader::release()
{
	release_basic();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR����ƽ��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
compute_averagelight::compute_averagelight(LPCWSTR filename) :shader_basic(filename)
{
}
engine_basic::engine_fail_reason compute_averagelight::set_compute_tex(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr = texture_input->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting HDR texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason compute_averagelight::set_compute_buffer(ID3D11UnorderedAccessView *buffer_input_need, ID3D11UnorderedAccessView *buffer_output_need)
{
	HRESULT hr;
	hr = buffer_input->SetUnorderedAccessView(buffer_input_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set UAV buffer error when setting HDR texture");
		return error_message;
	}
	hr = buffer_output->SetUnorderedAccessView(buffer_output_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set UAV buffer error when setting HDR texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason compute_averagelight::set_piccturerange(int width_need, int height_need, int buffer_num, int bytewidth)
{
	XMUINT4 rec_float = XMUINT4(static_cast<unsigned int>(width_need), static_cast<unsigned int>(height_need), static_cast<unsigned int>(buffer_num), static_cast<unsigned int>(bytewidth));
	HRESULT hr = texture_range->SetRawValue((void*)&rec_float, 0, sizeof(rec_float));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set_piccturerange error when setting HDR texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void compute_averagelight::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
}
void compute_averagelight::release()
{
	release_basic();
}
void compute_averagelight::init_handle()
{
	texture_input = fx_need->GetVariableByName("input_tex")->AsShaderResource();
	buffer_input = fx_need->GetVariableByName("input_buffer")->AsUnorderedAccessView();
	buffer_output = fx_need->GetVariableByName("output_buffer")->AsUnorderedAccessView();
	texture_range = fx_need->GetVariableByName("input_range");
}
void compute_averagelight::dispatch(int width_need, int height_need, int final_need, int map_need)
{
	ID3DX11EffectTechnique* tech_need;
	tech_need = fx_need->GetTechniqueByName("HDR_average_pass");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech_need->GetDesc(&techDesc);
	//�ַ��߳�
	for (UINT i = 0; i<techDesc.Passes; ++i)
	{
		tech_need->GetPassByIndex(i)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
		if (i == 0)
		{
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Dispatch(width_need, height_need, 1);
		}
		else if (i == 1)
		{
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Dispatch(final_need, 1, 1);
		}
		else
		{
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Dispatch(map_need, 1, 1);
		}
	}
	//��ԭ��Ⱦ״̬
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->CSSetShaderResources(0, 1, nullSRV);
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->CSSetShader(0, 0, 0);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_�߹���ȡ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_HDRpreblur::shader_HDRpreblur(LPCWSTR filename) : shader_basic(filename)
{
}
engine_basic::engine_fail_reason shader_HDRpreblur::set_buffer_input(ID3D11ShaderResourceView *buffer_need, ID3D11ShaderResourceView *tex_need)
{
	HRESULT hr = tex_input->SetResource(tex_need);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "set_buffer_input error when setting HDR texture");
		return error_message;
	}

	hr = buffer_input->SetResource(buffer_need);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "set_buffer_input error when setting HDR texture");
		return error_message;
	}
	XMFLOAT4X4 yuv_rgb = XMFLOAT4X4
		(1.0f, 1.0f, 1.0f, 0.0f,
			0.0f, -0.3441f, 1.7720f, 0.0f,
			1.4020f, -0.7141f, 0.0f, 0.0f,
			-0.7010f, 0.5291f, -0.8860f, 1.0f);
	engine_basic::engine_fail_reason check_error = set_matrix(matrix_YUV2RGB, &yuv_rgb);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	XMFLOAT4X4 rgb_yuv = XMFLOAT4X4(0.2990f, -0.1687f, 0.5f, 0.0f,
		0.5870, -0.3313f, -0.4187f, 0.0f,
		0.1140, 0.5f, -0.0813f, 0.0f,
		0.0f, 0.5f, 0.5f, 1.0f);
	check_error = set_matrix(matrix_RGB2YUV, &rgb_yuv);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_HDRpreblur::set_lum_message(float average_lum, float HighLight_divide, float HightLight_max, float key_tonemapping)
{
	XMFLOAT4 rec_float = XMFLOAT4(average_lum, HighLight_divide, HightLight_max, key_tonemapping);
	HRESULT hr = lum_message->SetRawValue((void*)&rec_float, 0, sizeof(rec_float));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "setting HDR lum_message error when setting HDR texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_HDRpreblur::set_piccturerange(int width_need, int height_need, int buffer_num, int bytewidth)
{
	XMUINT4 rec_float = XMUINT4(static_cast<unsigned int>(width_need), static_cast<unsigned int>(height_need), static_cast<unsigned int>(buffer_num), static_cast<unsigned int>(bytewidth));
	HRESULT hr = texture_range->SetRawValue((void*)&rec_float, 0, sizeof(rec_float));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "setting set_piccturerange error when setting HDR texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_HDRpreblur::init_handle()
{
	matrix_YUV2RGB = fx_need->GetVariableByName("YUV2RGB")->AsMatrix();
	matrix_RGB2YUV = fx_need->GetVariableByName("RGB2YUV")->AsMatrix();
	tex_input = fx_need->GetVariableByName("input_tex")->AsShaderResource();
	lum_message = fx_need->GetVariableByName("light_average");
	texture_range = fx_need->GetVariableByName("input_range");
	buffer_input = fx_need->GetVariableByName("input_buffer")->AsShaderResource();
}
void shader_HDRpreblur::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
void shader_HDRpreblur::release()
{
	release_basic();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_�߹�ģ��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_HDRblur::shader_HDRblur(LPCWSTR filename) : shader_basic(filename)
{
}
engine_basic::engine_fail_reason shader_HDRblur::set_tex_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "setting set_tex_resource error when setting HDR blur shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_HDRblur::set_image_size(float width, float height)
{
	HRESULT hr;
	hr = TexelWidth->SetFloat(width);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set_image_size error when setting HDR blur shader");
		return error_message;
	}
	hr = TexelHeight->SetFloat(height);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set_image_size error when setting HDR blur shader");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_HDRblur::release()
{
	release_basic();
}
void shader_HDRblur::init_handle()
{
	TexelWidth = fx_need->GetVariableByName("gTexelWidth")->AsScalar();
	TexelHeight = fx_need->GetVariableByName("gTexelHeight")->AsScalar();
	tex_input = fx_need->GetVariableByName("gInputImage")->AsShaderResource();
}
void shader_HDRblur::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_������Ⱦ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_HDRfinal::shader_HDRfinal(LPCWSTR filename) : shader_basic(filename)
{
}
engine_basic::engine_fail_reason shader_HDRfinal::set_tex_resource(ID3D11ShaderResourceView *tex_input_need, ID3D11ShaderResourceView *tex_bloom_need, ID3D11ShaderResourceView *buffer_need)
{
	HRESULT hr;
	hr = tex_input->SetResource(tex_input_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set HDR blur texture error in HDR final mapping");
		return error_message;
	}
	hr = tex_bloom->SetResource(tex_bloom_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set HDR bloom texture error in HDR final mapping");
		return error_message;
	}
	hr = buffer_input->SetResource(buffer_need);
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "set HDR average buffer error in HDR final mapping");
		return error_message;
	}
	XMFLOAT4X4 yuv_rgb = XMFLOAT4X4
		(1.0f, 1.0f, 1.0f, 0.0f,
			0.0f, -0.3441f, 1.7720f, 0.0f,
			1.4020f, -0.7141f, 0.0f, 0.0f,
			-0.7010f, 0.5291f, -0.8860f, 1.0f);
	engine_basic::engine_fail_reason check_error = set_matrix(matrix_YUV2RGB, &yuv_rgb);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	XMFLOAT4X4 rgb_yuv = XMFLOAT4X4(0.2990f, -0.1687f, 0.5f, 0.0f,
		0.5870, -0.3313f, -0.4187f, 0.0f,
		0.1140, 0.5f, -0.0813f, 0.0f,
		0.0f, 0.5f, 0.5f, 1.0f);
	check_error = set_matrix(matrix_RGB2YUV, &rgb_yuv);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_HDRfinal::set_lum_message(float average_lum, float HighLight_divide, float HightLight_max, float key_tonemapping)
{
	XMFLOAT4 rec_float = XMFLOAT4(average_lum, HighLight_divide, HightLight_max, key_tonemapping);
	HRESULT hr = lum_message->SetRawValue((void*)&rec_float, 0, sizeof(rec_float));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set HDR lum_message error in HDR final mapping");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_HDRfinal::set_piccturerange(int width_need, int height_need, int buffer_num, int bytewidth)
{
	XMUINT4 rec_float = XMUINT4(static_cast<unsigned int>(width_need), static_cast<unsigned int>(height_need), static_cast<unsigned int>(buffer_num), static_cast<unsigned int>(bytewidth));
	HRESULT hr = texture_range->SetRawValue((void*)&rec_float, 0, sizeof(rec_float));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set HDR piccturerange error in HDR final mapping");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_HDRfinal::release()
{
	release_basic();
}
void shader_HDRfinal::init_handle()
{
	lum_message = fx_need->GetVariableByName("light_average");
	matrix_YUV2RGB = fx_need->GetVariableByName("YUV2RGB")->AsMatrix();
	matrix_RGB2YUV = fx_need->GetVariableByName("RGB2YUV")->AsMatrix();
	tex_input = fx_need->GetVariableByName("input_tex")->AsShaderResource();
	tex_bloom = fx_need->GetVariableByName("input_bloom")->AsShaderResource();
	texture_range = fx_need->GetVariableByName("input_range");
	buffer_input = fx_need->GetVariableByName("input_buffer")->AsShaderResource();
}
void shader_HDRfinal::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~����ɢ��Ԥ����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_atmosphere_pretreat::shader_atmosphere_pretreat(LPCWSTR filename) : shader_basic(filename)
{
}
engine_basic::engine_fail_reason shader_atmosphere_pretreat::set_tex_transmittance(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = transmittance_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set transmittance texture error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_pretreat::set_tex_single_rayleigh_scattering(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = single_rayleigh_scattering_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set single_rayleigh_scattering texture error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_pretreat::set_tex_single_mie_scattering(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = single_mie_scattering_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set single_mie_scattering texture error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_pretreat::set_tex_multiple_scattering(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = multiple_scattering_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set multiple_scattering texture error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_pretreat::set_tex_irradiance(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = irradiance_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set irradiance texture error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_pretreat::set_tex_scattering_density(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = scattering_density_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set scattering_density texture error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason shader_atmosphere_pretreat::set_scattering_order(unsigned int scattering_order_in)
{
	HRESULT hr = scattering_order->SetRawValue((void*)&scattering_order_in, 0, sizeof(scattering_order_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set scattering_order error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_pretreat::set_layer(unsigned int layer_in)
{
	HRESULT hr = layer->SetRawValue((void*)&layer_in, 0, sizeof(layer_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set layer error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_pretreat::set_luminance_from_radiance(XMFLOAT4X4 luminance_from_radiance_in)
{
	auto check_error = set_matrix(luminance_from_radiance, &luminance_from_radiance_in);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_atmosphere_pretreat::release()
{
	release_basic();
}
void shader_atmosphere_pretreat::init_handle()
{

	scattering_order = fx_need->GetVariableByName("scattering_order");                        //ɢ���
	layer = fx_need->GetVariableByName("layer");                                   //3d�����
	luminance_from_radiance = fx_need->GetVariableByName("luminance_from_radiance")->AsMatrix();                 //�����

	transmittance_texture = fx_need->GetVariableByName("transmittance_texture")->AsShaderResource();                   //͸��������
	single_rayleigh_scattering_texture = fx_need->GetVariableByName("single_rayleigh_scattering_texture")->AsShaderResource();      //��������ɢ��
	single_mie_scattering_texture = fx_need->GetVariableByName("single_mie_scattering_texture")->AsShaderResource();           //��������ɢ��
	multiple_scattering_texture = fx_need->GetVariableByName("multiple_scattering_texture")->AsShaderResource();             //���ɢ��
	irradiance_texture = fx_need->GetVariableByName("irradiance_texture")->AsShaderResource();                      //��������
	scattering_density_texture = fx_need->GetVariableByName("scattering_density_texture")->AsShaderResource();              //����ɢ��Ŀ��
}
void shader_atmosphere_pretreat::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~����ɢ����ɫ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_atmosphere_render::shader_atmosphere_render(LPCWSTR filename) : shader_basic(filename)
{
}
//��������
engine_basic::engine_fail_reason shader_atmosphere_render::set_tex_transmittance(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = transmittance_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set transmittance texture error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_render::set_tex_scattering(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = scattering_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set scattering texture error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_render::set_tex_single_mie_scattering(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = single_mie_scattering_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set single_mie_scattering texture error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_render::set_tex_irradiance(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = irradiance_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set irradiance texture error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_render::set_tex_mask(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = mask_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set mask texture error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
//��������
engine_basic::engine_fail_reason shader_atmosphere_render::set_view_from_clip(XMFLOAT4X4 view_from_clip_in)
{
	auto check_error = set_matrix(view_from_clip, &view_from_clip_in);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_render::set_model_from_view(XMFLOAT4X4 view_model_from_view)
{
	auto check_error = set_matrix(model_from_view, &view_model_from_view);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
//������������
engine_basic::engine_fail_reason shader_atmosphere_render::set_white_point(XMFLOAT3 white_point_in)
{
	HRESULT hr = white_point->SetRawValue((void*)&white_point_in, 0, sizeof(white_point_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set white_point_in error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_render::set_earth_center(XMFLOAT3 earth_center_in)
{
	HRESULT hr = earth_center->SetRawValue((void*)&earth_center_in, 0, sizeof(earth_center_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set earth_center_in error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_render::set_sun_size(XMFLOAT2 sun_size_in)
{
	HRESULT hr = sun_size->SetRawValue((void*)&sun_size_in, 0, sizeof(sun_size_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set sun_size error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_render::set_camera(XMFLOAT3 camera_in)
{
	HRESULT hr = camera->SetRawValue((void*)&camera_in, 0, sizeof(camera_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set camera error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_render::set_exposure(float exposure_in)
{
	HRESULT hr = exposure->SetRawValue((void*)&exposure_in, 0, sizeof(exposure_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set exposure error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_render::set_sun_direction(XMFLOAT3 sun_direction_in)
{
	HRESULT hr = sun_direction->SetRawValue((void*)&sun_direction_in, 0, sizeof(sun_direction_in));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set sun_direction error in atmosphere_pretreat");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_render::set_tex_depth(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = depth_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set depth_texture error in atmosphere_render");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_render::set_tex_normal(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr;
	hr = normal_texture->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set normal_texture error in atmosphere_render");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_atmosphere_render::set_FrustumCorners(const XMFLOAT4 v[4])
{
	HRESULT hr = FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set FrustumCorners error in atmosphere_render");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason set_tex_normal(ID3D11ShaderResourceView *tex_input);
void shader_atmosphere_render::release()
{
	release_basic();
}
void shader_atmosphere_render::init_handle()
{

	white_point = fx_need->GetVariableByName("white_point_in");//��ƽ�����
	earth_center = fx_need->GetVariableByName("earth_center");//3d�����
	sun_size = fx_need->GetVariableByName("sun_size");//3d�����
	
	camera = fx_need->GetVariableByName("camera");//�����λ��
	exposure = fx_need->GetVariableByName("exposure");//��ƽ���ع����
	sun_direction = fx_need->GetVariableByName("sun_direction");//̫���ⷽ��

	view_from_clip = fx_need->GetVariableByName("view_from_clip")->AsMatrix();//��դ����->ȡ���ռ�任
	model_from_view = fx_need->GetVariableByName("model_from_view")->AsMatrix();//ȡ���ռ�->����ռ�任

	transmittance_texture = fx_need->GetVariableByName("transmittance_texture")->AsShaderResource();//͸��������
	scattering_texture = fx_need->GetVariableByName("scattering_texture")->AsShaderResource();//����ɢ��
	single_mie_scattering_texture = fx_need->GetVariableByName("single_mie_scattering_texture")->AsShaderResource();//��������ɢ��
	irradiance_texture = fx_need->GetVariableByName("irradiance_texture")->AsShaderResource();//��������

	depth_texture = fx_need->GetVariableByName("gdepth_map")->AsShaderResource();
	normal_texture = fx_need->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	mask_texture = fx_need->GetVariableByName("render_mask")->AsShaderResource();
	FrustumCorners = fx_need->GetVariableByName("gFrustumCorners")->AsVector();
}
void shader_atmosphere_render::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~���ٸ���Ҷ�任~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
compute_FFT::compute_FFT(LPCWSTR filename) :shader_basic(filename)
{
}
engine_basic::engine_fail_reason compute_FFT::set_shader_resource(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_input->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting FFT SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason compute_FFT::set_compute_UAV(ID3D11UnorderedAccessView *buffer_input_need) 
{
	HRESULT hr;
	hr = UAV_output->SetUnorderedAccessView(buffer_input_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set UAV buffer error when setting FFT texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason compute_FFT::set_Constant_Buffer(ID3D11Buffer *buffer_input) 
{
	HRESULT hr;
	hr = constent_buffer->SetConstantBuffer(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set constant buffer error when setting FFT texture");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void compute_FFT::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
}
void compute_FFT::release()
{
	release_basic();
}
void compute_FFT::init_handle()
{
	SRV_input = fx_need->GetVariableByName("g_SrcData")->AsShaderResource();
	UAV_output = fx_need->GetVariableByName("g_DstData")->AsUnorderedAccessView();
	constent_buffer = fx_need->GetConstantBufferByName("cbChangePerCall");
	//constent_buffer = fx_need->GetVariableByName("cbChangePerCall")->AsConstantBuffer();
}
void compute_FFT::dispatch(int grad,LPCSTR name)
{
	ID3DX11EffectTechnique* tech_need;
	tech_need = fx_need->GetTechniqueByName(name);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech_need->GetDesc(&techDesc);
	//�ַ��߳�
	for (UINT i = 0; i<techDesc.Passes; ++i)
	{
		tech_need->GetPassByIndex(i)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Dispatch(grad, 1, 1);
	}
	//��ԭ��Ⱦ״̬
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->CSSetShaderResources(0, 1, nullSRV);
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->CSSetShader(0, 0, 0);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ˮ����ȾԤ����(���ڸ���Ҷ�任)~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_ocean_simulateCS::shader_ocean_simulateCS(LPCWSTR filename) :shader_basic(filename)
{
}
engine_basic::engine_fail_reason shader_ocean_simulateCS::set_shader_resource_h0(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_input_h0->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean simulate CS SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_simulateCS::set_shader_resource_omega(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_input_omega->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean simulate CS SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_simulateCS::set_compute_UAV(ID3D11UnorderedAccessView *buffer_input_need)
{
	HRESULT hr;
	hr = UAV_output->SetUnorderedAccessView(buffer_input_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set UAV buffer error when setting ocean simulate CS");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_simulateCS::set_Constant_Buffer_Immutable(ID3D11Buffer *buffer_input)
{
	HRESULT hr;
	hr = constent_buffer_Immutable->SetConstantBuffer(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set constent_buffer_Immutable error when setting ocean simulate CS");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_simulateCS::set_Constant_Buffer_ChangePerFrame(ID3D11Buffer *buffer_input)
{
	HRESULT hr;
	hr = constent_buffer_ChangePerFrame->SetConstantBuffer(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set constent_buffer_ChangePerFrame error when setting ocean simulate CS");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_ocean_simulateCS::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
}
void shader_ocean_simulateCS::release()
{
	release_basic();
}
void shader_ocean_simulateCS::init_handle()
{
	SRV_input_h0 = fx_need->GetVariableByName("g_InputH0")->AsShaderResource();
	SRV_input_omega = fx_need->GetVariableByName("g_InputOmega")->AsShaderResource();
	UAV_output = fx_need->GetVariableByName("g_OutputHt")->AsUnorderedAccessView();
	constent_buffer_Immutable = fx_need->GetConstantBufferByName("cbImmutable");
	constent_buffer_ChangePerFrame = fx_need->GetConstantBufferByName("cbChangePerFrame");
	//constent_buffer_Immutable = fx_need->GetVariableByName("cbImmutable")->AsConstantBuffer();
	//constent_buffer_ChangePerFrame = fx_need->GetVariableByName("cbChangePerFrame")->AsConstantBuffer();
}
void shader_ocean_simulateCS::dispatch(int grad_x,int grad_y)
{
	ID3DX11EffectTechnique* tech_need;
	tech_need = fx_need->GetTechniqueByName("common_simulatepre");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech_need->GetDesc(&techDesc);
	//�ַ��߳�
	for (UINT i = 0; i<techDesc.Passes; ++i)
	{
		tech_need->GetPassByIndex(i)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Dispatch(grad_x, grad_y, 1);
	}
	//��ԭ��Ⱦ״̬
	ID3D11ShaderResourceView* nullSRV[2] = { 0 ,0};
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->CSSetShaderResources(0, 2, nullSRV);
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->CSSetShader(0, 0, 0);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ˮ��Ԥ��Ⱦ(���ڸ���Ҷ�任)~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_ocean_simulateVPS::shader_ocean_simulateVPS(LPCWSTR filename) :shader_basic(filename)
{
}
engine_basic::engine_fail_reason shader_ocean_simulateVPS::set_shader_resource_texture(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_input_tex->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean simulate VSPS texture SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_simulateVPS::set_shader_resource_buffer(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_input_buffer->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean simulate VSPS buffer SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_simulateVPS::set_Constant_Buffer_Immutable(ID3D11Buffer *buffer_input)
{
	HRESULT hr;
	hr = constent_buffer_Immutable->SetConstantBuffer(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set constent_buffer_Immutable error when setting ocean simulate VSPS");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_simulateVPS::set_Constant_Buffer_ChangePerFrame(ID3D11Buffer *buffer_input)
{
	HRESULT hr;
	hr = constent_buffer_ChangePerFrame->SetConstantBuffer(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set constent_buffer_ChangePerFrame error when setting ocean simulate VSPS");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_ocean_simulateVPS::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
void shader_ocean_simulateVPS::release()
{
	release_basic();
}
void shader_ocean_simulateVPS::init_handle()
{
	SRV_input_buffer = fx_need->GetVariableByName("g_InputDxyz")->AsShaderResource();
	SRV_input_tex = fx_need->GetVariableByName("g_samplerDisplacementMap")->AsShaderResource();
	constent_buffer_Immutable = fx_need->GetConstantBufferByName("cbImmutable");
	constent_buffer_ChangePerFrame = fx_need->GetConstantBufferByName("cbChangePerFrame");
	//constent_buffer_Immutable = fx_need->GetVariableByName("cbImmutable")->AsConstantBuffer();
	//constent_buffer_ChangePerFrame = fx_need->GetVariableByName("cbChangePerFrame")->AsConstantBuffer();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ˮ����ʽ��Ⱦ(������ϸ��)~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_ocean_render::shader_ocean_render(LPCWSTR filename) :shader_basic(filename)
{
}
engine_basic::engine_fail_reason shader_ocean_render::set_texture_displayment(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_tex_displayment->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean render displayment SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_render::set_texture_Perlin(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_tex_Perlin->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean render Perlin SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_render::set_texture_gradient(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_tex_gradient->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean render gradient SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_render::set_texture_Fresnel(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_tex_Fresnel->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean render Fresnel SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_render::set_texture_ReflectCube(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_tex_ReflectCube->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean render ReflectCube SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason shader_ocean_render::set_Constant_Buffer_Shading(ID3D11Buffer *buffer_input)
{
	HRESULT hr;
	hr = constent_buffer_Shading->SetConstantBuffer(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set constent_buffer_Shading error when setting ocean render VSPS");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_render::set_Constant_Buffer_PerCall(ID3D11Buffer *buffer_input)
{
	HRESULT hr;
	hr = constent_buffer_PerCall->SetConstantBuffer(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set constent_buffer_PerCall error when setting ocean render VSPS");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_ocean_render::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
void shader_ocean_render::release()
{
	release_basic();
}
void shader_ocean_render::init_handle()
{
	SRV_tex_displayment = fx_need->GetVariableByName("g_texDisplacement")->AsShaderResource();
	SRV_tex_Perlin = fx_need->GetVariableByName("g_texPerlin")->AsShaderResource();
	SRV_tex_gradient = fx_need->GetVariableByName("g_texGradient")->AsShaderResource();
	SRV_tex_Fresnel = fx_need->GetVariableByName("g_texFresnel")->AsShaderResource();
	SRV_tex_ReflectCube = fx_need->GetVariableByName("g_texReflectCube")->AsShaderResource();

	constent_buffer_Shading = fx_need->GetConstantBufferByName("cbShading");
	constent_buffer_PerCall = fx_need->GetConstantBufferByName("cbChangePerCall");
	//constent_buffer_Shading = fx_need->GetVariableByName("cbShading")->AsConstantBuffer();
	//constent_buffer_PerCall = fx_need->GetVariableByName("cbChangePerCall")->AsConstantBuffer();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ˮ����ʽ��Ⱦ(����ϸ��)~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_ocean_draw::shader_ocean_draw(LPCWSTR filename) :shader_basic(filename)
{
}
engine_basic::engine_fail_reason shader_ocean_draw::set_view_pos(XMFLOAT3 eye_pos)
{
	HRESULT hr = view_pos_handle->SetRawValue((void*)&eye_pos, 0, sizeof(eye_pos));
	if (hr != S_OK)
	{
		engine_basic::engine_fail_reason error_message(hr, "ocean draw error when setting view position");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_draw::set_texture_displayment(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_tex_displayment->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean render displayment SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_draw::set_texture_Perlin(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_tex_Perlin->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean render Perlin SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_draw::set_texture_gradient(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_tex_gradient->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean render gradient SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_draw::set_texture_Fresnel(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_tex_Fresnel->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean render Fresnel SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_draw::set_texture_ReflectCube(ID3D11ShaderResourceView *data_input)
{
	HRESULT hr = SRV_tex_ReflectCube->SetResource(data_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting ocean render ReflectCube SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_draw::set_trans_all(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(final_mat_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_draw::set_trans_scal(XMFLOAT4X4 *mat_need)
{
	auto check_error = set_matrix(scal_mat_handle, mat_need);;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_draw::set_trans_world(XMFLOAT4X4 *mat_need)
{
	XMVECTOR x_delta;
	XMMATRIX world_need = XMLoadFloat4x4(mat_need);
	XMMATRIX coordtrans = XMLoadFloat4x4(&XMFLOAT4X4(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1));
	XMMATRIX normal_need = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&x_delta, world_need));
	normal_need.r[0].m128_f32[3] = 0;
	normal_need.r[1].m128_f32[3] = 0;
	normal_need.r[2].m128_f32[3] = 0;
	normal_need.r[3].m128_f32[3] = 1;
	HRESULT hr;
	hr = world_mat_handle->SetMatrix(reinterpret_cast<float*>(&(world_need*coordtrans)));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set world matrix error in ocean draw part");
		return error_message;
	}
	hr = normal_mat_handle->SetMatrix(reinterpret_cast<float*>(&(normal_need*coordtrans)));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set normal matrix error in ocean draw part");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_draw::set_Constant_Buffer_Shading(ID3D11Buffer *buffer_input)
{
	HRESULT hr;
	hr = constent_buffer_Shading->SetConstantBuffer(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set constent_buffer_Shading error when setting ocean render VSPS");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_ocean_draw::set_Constant_Buffer_PerCall(ID3D11Buffer *buffer_input)
{
	HRESULT hr;
	hr = constent_buffer_PerCall->SetConstantBuffer(buffer_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set constent_buffer_PerCall error when setting ocean render VSPS");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_ocean_draw::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,0 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
void shader_ocean_draw::release()
{
	release_basic();
}
void shader_ocean_draw::init_handle()
{
	view_pos_handle = fx_need->GetVariableByName("g_LocalEye");
	scal_mat_handle = fx_need->GetVariableByName("scal_matrix")->AsMatrix();
	final_mat_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();
	world_mat_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();
	normal_mat_handle = fx_need->GetVariableByName("normal_matrix")->AsMatrix();

	SRV_tex_displayment = fx_need->GetVariableByName("g_texDisplacement")->AsShaderResource();
	SRV_tex_Perlin = fx_need->GetVariableByName("g_texPerlin")->AsShaderResource();
	SRV_tex_gradient = fx_need->GetVariableByName("g_texGradient")->AsShaderResource();
	SRV_tex_Fresnel = fx_need->GetVariableByName("g_texFresnel")->AsShaderResource();
	SRV_tex_ReflectCube = fx_need->GetVariableByName("g_texReflectCube")->AsShaderResource();

	constent_buffer_Shading = fx_need->GetConstantBufferByName("cbShading");
	constent_buffer_PerCall = fx_need->GetConstantBufferByName("cbChangePerCall");
	//constent_buffer_Shading = fx_need->GetVariableByName("cbShading")->AsConstantBuffer();
	//constent_buffer_PerCall = fx_need->GetVariableByName("cbChangePerCall")->AsConstantBuffer();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~IBL���淴��決~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_IBL_specular::shader_IBL_specular(LPCWSTR filename) : shader_basic(filename)
{
}
engine_basic::engine_fail_reason shader_IBL_specular::set_input_CubeTex(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr = tex_cube_handle->SetResource(tex_input);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "an error when setting IBL_specular Cube SRV");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_IBL_specular::set_input_message(XMFLOAT2 HalfPixel, float face_cube, float mip_level)
{
	HRESULT hr = HalfPixel_handle->SetRawValue((void*)&HalfPixel, 0, sizeof(HalfPixel));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set input_message error in IBL_specular");
		return error_message;
	}
	hr = Face_handle->SetRawValue((void*)&face_cube, 0, sizeof(face_cube));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set input_message error in IBL_specular");
		return error_message;
	}
	hr = MipIndex_handle->SetRawValue((void*)&mip_level, 0, sizeof(mip_level));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set input_message error in IBL_specular");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_IBL_specular::release()
{
	release_basic();
}
void shader_IBL_specular::init_handle()
{
	HalfPixel_handle = fx_need->GetVariableByName("HalfPixel");
	Face_handle = fx_need->GetVariableByName("Face");
	MipIndex_handle = fx_need->GetVariableByName("MipIndex");
	tex_cube_handle = fx_need->GetVariableByName("Cubemap")->AsShaderResource();
}
void shader_IBL_specular::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~������Ⱦ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_terrain_render::shader_terrain_render(LPCWSTR filename) : terrain_shader_basic(filename),shader_basic(filename)
{
}
engine_basic::engine_fail_reason shader_terrain_render::set_trans_world(XMFLOAT4X4 *mat_world)
{
	XMVECTOR x_delta;
	XMMATRIX world_need = XMLoadFloat4x4(mat_world);
	XMMATRIX normal_need = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&x_delta, world_need));
	normal_need.r[0].m128_f32[3] = 0;
	normal_need.r[1].m128_f32[3] = 0;
	normal_need.r[2].m128_f32[3] = 0;
	normal_need.r[3].m128_f32[3] = 1;
	XMFLOAT4X4 mat_normal;
	XMStoreFloat4x4(&mat_normal, normal_need);
	engine_basic::engine_fail_reason check_error = set_matrix(world_matrix_handle, mat_world);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = set_matrix(normal_matrix_handle, &mat_normal);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_terrain_render::set_trans_all(XMFLOAT4X4 *mat_final)
{
	engine_basic::engine_fail_reason check_error = set_matrix(final_matrix_handle, mat_final);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_terrain_render::release()
{
	release_basic();
}
void shader_terrain_render::init_handle()
{
	init_handle_terrain();
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();
	normal_matrix_handle = fx_need->GetVariableByName("normal_matrix")->AsMatrix();
	final_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();
}
void shader_terrain_render::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXHEIGHT",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXDIFFUSE",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,20 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~������ɫ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_particle::shader_particle(LPCWSTR filename) :shader_basic(filename)
{

}
void shader_particle::init_handle()
{
	//������Ϣ���
	texture_handle = fx_need->GetVariableByName("texture_first")->AsShaderResource();
	RandomTex_handle = fx_need->GetVariableByName("texture_random")->AsShaderResource();
	//���α任��Ϣ���
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();
	view_pos_handle = fx_need->GetVariableByName("position_view");
	//���Ӳ�����Ϣ
	start_position_handle = fx_need->GetVariableByName("position_start");
	//����ʱ��
	time_game_handle = fx_need->GetVariableByName("game_time")->AsScalar();
	time_delta_handle = fx_need->GetVariableByName("delta_time")->AsScalar();
}
void shader_particle::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "AGE",      0, DXGI_FORMAT_R32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TYPE",     0, DXGI_FORMAT_R32_UINT,        0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
engine_basic::engine_fail_reason shader_particle::set_viewposition(XMFLOAT3 eye_pos)
{
	HRESULT hr = view_pos_handle->SetRawValue((void*)&eye_pos, 0, sizeof(eye_pos));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set particle view position error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_particle::set_startposition(XMFLOAT3 start_pos)
{
	HRESULT hr = start_position_handle->SetRawValue((void*)&start_pos, 0, sizeof(start_pos));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set particle start position error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_particle::set_frametime(float game_time, float delta_time)
{
	HRESULT hr = time_game_handle->SetFloat(game_time);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set particle rand time error");
		return error_message;
	}
	hr = time_delta_handle->SetFloat(delta_time);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set particle delta time error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_particle::set_randomtex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = RandomTex_handle->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set particle random tex error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_particle::set_trans_all(XMFLOAT4X4 *mat_need)
{
	engine_basic::engine_fail_reason check_error = set_matrix(project_matrix_handle, mat_need);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_particle::set_texture(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_handle->SetResource(tex_in);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "set particle color tex error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void shader_particle::release()
{
	release_basic();
}

//shader������
shader_control *shader_control::shadercontrol_pInstance = NULL;
shader_control::shader_control() 
{
}
engine_basic::engine_fail_reason shader_control::add_a_new_shader(std::type_index class_type, std::shared_ptr<shader_basic> shader_in)
{
	if (!shader_in)
	{
		engine_basic::engine_fail_reason failed_message(std::string("the shader") + class_type.name() + "instance is NULL");
		return failed_message;
	}
	std::pair<std::string, std::shared_ptr<shader_basic>> data_needB(class_type.name(), shader_in);
	auto check_iferror = shader_list.insert(data_needB);
	if (!check_iferror.second)
	{
		engine_basic::engine_fail_reason failed_message(std::string("a repeat shader") + class_type.name() + "is already init");
		return failed_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason shader_control::init()
{
	engine_basic::engine_fail_reason check_error = init_basic();
	if (check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
std::wstring shader_control::get_path_name(std::string name_char)
{
	std::wstring rec_data;
	int nLen = (int)name_char.length();
	rec_data.resize(nLen, L' ');
	int nResult = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)name_char.c_str(), nLen, (LPWSTR)rec_data.c_str(), nLen);
	return rec_data;
}
engine_basic::engine_fail_reason shader_control::init_basic()
{
	std::string shader_path;
#ifdef _DEBUG
	shader_path = "F:\\Microsoft Visual Studio\\pancystar_engine2.0\\pancystar_engine2\\texturearray_package\\Debug\\";
#else
	shader_path = "F:\\Microsoft Visual Studio\\pancystar_engine2.0\\pancystar_engine2\\texturearray_package\\Release\\";
#endif
	std::shared_ptr<color_shader> shader_color_test = std::make_shared<color_shader>(get_path_name(shader_path +"color_test.cso").c_str());
	engine_basic::engine_fail_reason check_error = shader_color_test->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(color_shader)), shader_color_test);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}


	std::shared_ptr<virtual_light_shader> shader_vlight_test = std::make_shared<virtual_light_shader>(get_path_name(shader_path +"virtual_light.cso").c_str());
	check_error = shader_vlight_test->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(virtual_light_shader)), shader_vlight_test);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<picture_show_shader> shader_picture_test = std::make_shared<picture_show_shader>(get_path_name(shader_path +"show_pic.cso").c_str());
	check_error = shader_picture_test->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(picture_show_shader)), shader_picture_test);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_pretreat_gbuffer> shader_gbuffer = std::make_shared<shader_pretreat_gbuffer>(get_path_name(shader_path +"gbuffer_pretreat.cso").c_str());
	check_error = shader_gbuffer->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_pretreat_gbuffer)), shader_gbuffer);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_resolvedepth> shader_resolvemassdepth = std::make_shared<shader_resolvedepth>(get_path_name(shader_path +"ResolveMSAAdepthstencil.cso").c_str());
	check_error = shader_resolvemassdepth->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_resolvedepth)), shader_resolvemassdepth);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_ssaomap> shader_ssaomap_rec= std::make_shared<shader_ssaomap>(get_path_name(shader_path +"ssao_draw_aomap.cso").c_str());
	check_error = shader_ssaomap_rec->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_ssaomap)), shader_ssaomap_rec);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_ssaoblur> shader_ssaoblur_rec = std::make_shared<shader_ssaoblur>(get_path_name(shader_path +"ssao_blur_map.cso").c_str());
	check_error = shader_ssaoblur_rec->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_ssaoblur)), shader_ssaoblur_rec);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<light_shadow> shader_shadowmap = std::make_shared<light_shadow>(get_path_name(shader_path +"shadowmap.cso").c_str());
	check_error = shader_shadowmap->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(light_shadow)), shader_shadowmap);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<light_defered_lightbuffer> shader_defered_lightbuffer = std::make_shared<light_defered_lightbuffer>(get_path_name(shader_path +"light_buffer_pretreat.cso").c_str());
	check_error = shader_defered_lightbuffer->shder_create();
	if (!check_error.check_if_failed())
	{
	return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(light_defered_lightbuffer)), shader_defered_lightbuffer);
	if (!check_error.check_if_failed())
	{
	return check_error;
	}

	std::shared_ptr<light_defered_draw> shader_defered_draw = std::make_shared<light_defered_draw>(get_path_name(shader_path +"light_deffered.cso").c_str());
	check_error = shader_defered_draw->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(light_defered_draw)), shader_defered_draw);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_reflect_save_depth> shader_reflect_depthsave = std::make_shared<shader_reflect_save_depth>(get_path_name(shader_path +"save_cube_depthstencil.cso").c_str());
	check_error = shader_reflect_depthsave->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_reflect_save_depth)), shader_reflect_depthsave);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<rtgr_reflect> shader_rtgr_reflect = std::make_shared<rtgr_reflect>(get_path_name(shader_path +"RTGR.cso").c_str());
	check_error = shader_rtgr_reflect->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(rtgr_reflect)), shader_rtgr_reflect);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<rtgr_reflect_blur> shader_rtgr_reflect_blur = std::make_shared<rtgr_reflect_blur>(get_path_name(shader_path +"reflect_blur.cso").c_str());
	check_error = shader_rtgr_reflect_blur->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(rtgr_reflect_blur)), shader_rtgr_reflect_blur);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<rtgr_reflect_final> shader_rtgr_reflect_final = std::make_shared<rtgr_reflect_final>(get_path_name(shader_path +"reflect_final.cso").c_str());
	check_error = shader_rtgr_reflect_final->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(rtgr_reflect_final)), shader_rtgr_reflect_final);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_skycube> shader_sky_draw = std::make_shared<shader_skycube>(get_path_name(shader_path +"skycube.cso").c_str());
	check_error = shader_sky_draw->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_skycube)), shader_sky_draw);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<brdf_envpre_shader> shader_brdf_draw = std::make_shared<brdf_envpre_shader>(get_path_name(shader_path +"render_brdf_list.cso").c_str());
	check_error = shader_brdf_draw->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(brdf_envpre_shader)), shader_brdf_draw);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<compute_averagelight> shader_compute_averagelight = std::make_shared<compute_averagelight>(get_path_name(shader_path +"HDR_average_pass.cso").c_str());
	check_error = shader_compute_averagelight->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(compute_averagelight)), shader_compute_averagelight);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_HDRpreblur> shader_hdr_preblur = std::make_shared<shader_HDRpreblur>(get_path_name(shader_path +"HDR_preblur_pass.cso").c_str());
	check_error = shader_hdr_preblur->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_HDRpreblur)), shader_hdr_preblur);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_HDRblur> shader_hdr_blur = std::make_shared<shader_HDRblur>(get_path_name(shader_path +"HDR_blur_pass.cso").c_str());
	check_error = shader_hdr_blur->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_HDRblur)), shader_hdr_blur);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_HDRfinal> shader_hdr_final = std::make_shared<shader_HDRfinal>(get_path_name(shader_path +"HDR_final.cso").c_str());
	check_error = shader_hdr_final->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_HDRfinal)), shader_hdr_final);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_atmosphere_pretreat> shader_atmospherepretreat = std::make_shared<shader_atmosphere_pretreat>(get_path_name(shader_path +"atmosphere_precompute.cso").c_str());
	check_error = shader_atmospherepretreat->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_atmosphere_pretreat)), shader_atmospherepretreat);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_atmosphere_render> shader_atmosphererender = std::make_shared<shader_atmosphere_render>(get_path_name(shader_path +"atmosphere_render.cso").c_str());
	check_error = shader_atmosphererender->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_atmosphere_render)), shader_atmosphererender);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<compute_FFT> shader_compute_FFT = std::make_shared<compute_FFT>(get_path_name(shader_path +"fft_512x512_c2c.cso").c_str());
	check_error = shader_compute_FFT->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(compute_FFT)), shader_compute_FFT);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_ocean_simulateCS> shader_oceanpre_cs = std::make_shared<shader_ocean_simulateCS>(get_path_name(shader_path +"ocean_simulator_cs.cso").c_str());
	check_error = shader_oceanpre_cs->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_ocean_simulateCS)), shader_oceanpre_cs);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_ocean_simulateVPS> shader_oceanpre_vps = std::make_shared<shader_ocean_simulateVPS>(get_path_name(shader_path +"ocean_simulator_vs_ps.cso").c_str());
	check_error = shader_oceanpre_vps->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_ocean_simulateVPS)), shader_oceanpre_vps);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_ocean_render> shader_oceanrender_vps = std::make_shared<shader_ocean_render>(get_path_name(shader_path +"ocean_shading.cso").c_str());
	check_error = shader_oceanrender_vps->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_ocean_render)), shader_oceanrender_vps);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_ocean_draw> shader_oceandraw_tess = std::make_shared<shader_ocean_draw>(get_path_name(shader_path +"ocean_draw.cso").c_str());
	check_error = shader_oceandraw_tess->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_ocean_draw)), shader_oceandraw_tess);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_IBL_specular> shader_IBL_spec = std::make_shared<shader_IBL_specular>(get_path_name(shader_path +"IBL_gen.cso").c_str());
	check_error = shader_IBL_spec->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_IBL_specular)), shader_IBL_spec);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_terrain_render> shader_terrain_test = std::make_shared<shader_terrain_render>(get_path_name(shader_path +"terrain_tess.cso").c_str());
	check_error = shader_terrain_test->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_terrain_render)), shader_terrain_test);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	std::shared_ptr<shader_particle> shader_particle_test = std::make_shared<shader_particle>(get_path_name(shader_path +"basic_particle.cso").c_str());
	check_error = shader_particle_test->shder_create();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = add_a_new_shader(std::type_index(typeid(shader_particle)), shader_particle_test);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	engine_basic::engine_fail_reason succeed;
	return succeed;
}
std::shared_ptr<color_shader> shader_control::get_shader_color(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(color_shader)).name();
	auto shader_color = get_shader_by_type(std::type_index(typeid(color_shader)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<color_shader>();
	}
	auto out_pointer = std::dynamic_pointer_cast<color_shader>(shader_color);
	return out_pointer;
}
std::shared_ptr<virtual_light_shader> shader_control::get_shader_virtual_light(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(virtual_light_shader)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(virtual_light_shader)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<virtual_light_shader>();
	}
	auto out_pointer = std::dynamic_pointer_cast<virtual_light_shader>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<picture_show_shader> shader_control::get_shader_picture(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(picture_show_shader)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(picture_show_shader)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<picture_show_shader>();
	}
	auto out_pointer = std::dynamic_pointer_cast<picture_show_shader>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_pretreat_gbuffer> shader_control::get_shader_gbuffer(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_pretreat_gbuffer)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_pretreat_gbuffer)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_pretreat_gbuffer>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_pretreat_gbuffer>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_resolvedepth> shader_control::get_shader_resolvedepth(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_resolvedepth)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_resolvedepth)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_resolvedepth>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_resolvedepth>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_ssaomap> shader_control::get_shader_ssaodraw(engine_basic::engine_fail_reason &if_succeed) 
{
	std::string name_need = std::type_index(typeid(shader_ssaomap)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_ssaomap)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_ssaomap>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_ssaomap>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_ssaoblur> shader_control::get_shader_ssaoblur(engine_basic::engine_fail_reason &if_succeed) 
{
	std::string name_need = std::type_index(typeid(shader_ssaoblur)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_ssaoblur)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_ssaoblur>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_ssaoblur>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<light_shadow> shader_control::get_shader_shadowmap(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(light_shadow)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(light_shadow)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<light_shadow>();
	}
	auto out_pointer = std::dynamic_pointer_cast<light_shadow>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<light_defered_lightbuffer> shader_control::get_shader_lightbuffer(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(light_defered_lightbuffer)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(light_defered_lightbuffer)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<light_defered_lightbuffer>();
	}
	auto out_pointer = std::dynamic_pointer_cast<light_defered_lightbuffer>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<light_defered_draw> shader_control::get_shader_lightdeffered(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(light_defered_draw)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(light_defered_draw)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<light_defered_draw>();
	}
	auto out_pointer = std::dynamic_pointer_cast<light_defered_draw>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_reflect_save_depth> shader_control::get_shader_reflect_savedepth(engine_basic::engine_fail_reason &if_succeed) 
{
	std::string name_need = std::type_index(typeid(shader_reflect_save_depth)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_reflect_save_depth)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_reflect_save_depth>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_reflect_save_depth>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<rtgr_reflect> shader_control::get_shader_reflect_draw(engine_basic::engine_fail_reason &if_succeed) 
{
	std::string name_need = std::type_index(typeid(rtgr_reflect)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(rtgr_reflect)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<rtgr_reflect>();
	}
	auto out_pointer = std::dynamic_pointer_cast<rtgr_reflect>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<rtgr_reflect_blur> shader_control::get_shader_reflect_blur(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(rtgr_reflect_blur)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(rtgr_reflect_blur)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<rtgr_reflect_blur>();
	}
	auto out_pointer = std::dynamic_pointer_cast<rtgr_reflect_blur>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<rtgr_reflect_final> shader_control::get_shader_reflect_final(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(rtgr_reflect_final)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(rtgr_reflect_final)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<rtgr_reflect_final>();
	}
	auto out_pointer = std::dynamic_pointer_cast<rtgr_reflect_final>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_skycube> shader_control::get_shader_sky_draw(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_skycube)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_skycube)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_skycube>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_skycube>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<brdf_envpre_shader> shader_control::get_shader_brdf_pre(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(brdf_envpre_shader)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(brdf_envpre_shader)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<brdf_envpre_shader>();
	}
	auto out_pointer = std::dynamic_pointer_cast<brdf_envpre_shader>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<compute_averagelight> shader_control::get_shader_hdr_averagelight(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(compute_averagelight)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(compute_averagelight)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<compute_averagelight>();
	}
	auto out_pointer = std::dynamic_pointer_cast<compute_averagelight>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_HDRpreblur> shader_control::get_shader_hdr_preblur(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_HDRpreblur)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_HDRpreblur)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_HDRpreblur>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_HDRpreblur>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_HDRblur> shader_control::get_shader_hdr_blur(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_HDRblur)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_HDRblur)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_HDRblur>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_HDRblur>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_HDRfinal> shader_control::get_shader_hdr_final(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_HDRfinal)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_HDRfinal)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_HDRfinal>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_HDRfinal>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_atmosphere_pretreat> shader_control::get_shader_atmosphere_pretreat(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_atmosphere_pretreat)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_atmosphere_pretreat)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_atmosphere_pretreat>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_atmosphere_pretreat>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_atmosphere_render> shader_control::get_shader_atmosphere_render(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_atmosphere_render)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_atmosphere_render)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_atmosphere_render>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_atmosphere_render>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<compute_FFT> shader_control::get_shader_compute_fft(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(compute_FFT)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(compute_FFT)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<compute_FFT>();
	}
	auto out_pointer = std::dynamic_pointer_cast<compute_FFT>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_ocean_simulateCS> shader_control::get_shader_oceansimulate_cs(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_ocean_simulateCS)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_ocean_simulateCS)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_ocean_simulateCS>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_ocean_simulateCS>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_ocean_simulateVPS> shader_control::get_shader_oceansimulate_vps(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_ocean_simulateVPS)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_ocean_simulateVPS)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_ocean_simulateVPS>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_ocean_simulateVPS>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_ocean_render> shader_control::get_shader_oceanrender_vps(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_ocean_render)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_ocean_render)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_ocean_render>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_ocean_render>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_ocean_draw> shader_control::get_shader_oceandraw_tess(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_ocean_draw)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_ocean_draw)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_ocean_draw>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_ocean_draw>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_IBL_specular> shader_control::get_shader_IBL_specular(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_IBL_specular)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_IBL_specular)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_IBL_specular>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_IBL_specular>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_terrain_render> shader_control::get_shader_terrain_test(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_terrain_render)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_terrain_render)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_terrain_render>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_terrain_render>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_particle> shader_control::get_shader_particle_basic(engine_basic::engine_fail_reason &if_succeed)
{
	std::string name_need = std::type_index(typeid(shader_particle)).name();
	auto shader_vlight = get_shader_by_type(std::type_index(typeid(shader_particle)).name(), if_succeed);
	if (!if_succeed.check_if_failed())
	{
		return std::shared_ptr<shader_particle>();
	}
	auto out_pointer = std::dynamic_pointer_cast<shader_particle>(shader_vlight);
	return out_pointer;
}
std::shared_ptr<shader_basic> shader_control::get_shader_by_type(std::string type_name, engine_basic::engine_fail_reason &if_succeed)
{
	auto shader_out = shader_list.find(type_name)->second;
	if (!shader_out)
	{
		engine_basic::engine_fail_reason failed_message(std::string("could not find shader") + type_name);
		if_succeed = failed_message;
		return 0;
	}
	engine_basic::engine_fail_reason succeed;
	if_succeed = succeed;
	return shader_out;
}
void shader_control::release() 
{
	for (auto shader_data = shader_list.begin(); shader_data != shader_list.end(); ++shader_data) 
	{
		shader_data->second->release();
	}
	shader_list.clear();
}
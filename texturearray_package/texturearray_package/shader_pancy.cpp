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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~基础的着色器编译部分~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
	HRESULT hr2;
	hr2 = (*tech_need)->GetPassByIndex(0)->GetDesc(&pass_shade);
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateInputLayout(member_point, num_member, pass_shade.pIAInputSignature, pass_shade.IAInputSignatureSize, &input_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_message("get technique" + std::string(tech_name) + "error in" + shader_file_string);
		return failed_message;
	}
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
	//创建shader
	UINT flag_need(0);
	flag_need |= D3D10_SHADER_SKIP_OPTIMIZATION;
#if defined(DEBUG) || defined(_DEBUG)
	flag_need |= D3D10_SHADER_DEBUG;
#endif
	//两个ID3D10Blob用来存放编译好的shader及错误消息
	ID3D10Blob	*shader(NULL);
	ID3D10Blob	*errMsg(NULL);
	//编译effect
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
	//创建输入顶点格式
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~颜色测试~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~简单伪光照~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
virtual_light_shader::virtual_light_shader(LPCWSTR filename) : shader_basic(filename)
{
}
void virtual_light_shader::init_handle()
{
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();
	normal_matrix_handle = fx_need->GetVariableByName("normal_matrix")->AsMatrix();
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();

	texture_diffuse_handle = fx_need->GetVariableByName("texture_diffuse")->AsShaderResource();
	texture_normal_handle = fx_need->GetVariableByName("texture_specular")->AsShaderResource();
	texture_specular_handle = fx_need->GetVariableByName("texturet_normal")->AsShaderResource();
}
engine_basic::engine_fail_reason virtual_light_shader::set_trans_world(XMFLOAT4X4 *mat_world)
{
	//世界变换
	engine_basic::engine_fail_reason check_fail = set_matrix(world_matrix_handle, mat_world);
	if (!check_fail.check_if_failed())
	{
		engine_basic::engine_fail_reason failed_message("set world matrix error in" + shader_file_string);
		return failed_message;
	}
	//法线变换
	XMMATRIX rec_mat = XMLoadFloat4x4(mat_world);
	XMVECTOR x_delta;
	XMMATRIX check = rec_mat;
	//法线变换
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
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
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
void virtual_light_shader::release()
{
	release_basic();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~显示图像~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
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
//shader管理器
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
engine_basic::engine_fail_reason shader_control::init_basic()
{

	std::shared_ptr<color_shader> shader_color_test = std::make_shared<color_shader>(L"F:\\Microsoft Visual Studio\\pancystar_engine2.0\\pancystar_engine2\\texturearray_package\\Debug\\color_test.cso");
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


	std::shared_ptr<virtual_light_shader> shader_vlight_test = std::make_shared<virtual_light_shader>(L"F:\\Microsoft Visual Studio\\pancystar_engine2.0\\pancystar_engine2\\texturearray_package\\Debug\\virtual_light.cso");
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

	std::shared_ptr<picture_show_shader> shader_picture_test = std::make_shared<picture_show_shader>(L"F:\\Microsoft Visual Studio\\pancystar_engine2.0\\pancystar_engine2\\texturearray_package\\Debug\\virtual_light.cso");
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
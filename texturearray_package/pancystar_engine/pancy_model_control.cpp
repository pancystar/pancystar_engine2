#include"pancy_model_control.h"
model_reader_pancymesh::model_reader_pancymesh()
{
}
engine_basic::engine_fail_reason model_reader_pancymesh::get_technique(ID3DX11EffectTechnique *teque_need)
{
	if (teque_need == NULL)
	{
		engine_basic::engine_fail_reason error_message("get render technique error");
		return error_message;
	}
	teque_pancy = teque_need;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason model_reader_pancymesh::read_texture_from_file(std::vector<string> file_name_list)
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

geometry_instance_view::geometry_instance_view(int ID_need)
{
	if_show = true;
	instance_ID = ID_need;
	XMStoreFloat4x4(&world_matrix, XMMatrixIdentity());
}
void geometry_instance_view::update(XMFLOAT4X4 mat_in, float delta_time)
{
	world_matrix = mat_in;
}




geometry_resource_view::geometry_resource_view(model_reader_pancymesh *model_data_in, int ID_need,bool if_dynamic_in)
{
	ID_instance_index = 0;
	resource_view_ID = ID_need;
	model_data = model_data_in;
	if_cull_front = false;
	if_dynamic = if_dynamic_in;
}
std::vector<XMFLOAT4X4> geometry_resource_view::get_matrix_list()
{
	world_matrix_array.clear();
	for (auto data_now = instance_list.begin(); data_now != instance_list.end(); ++data_now)
	{
		if (data_now->second.check_if_show())
		{
			XMFLOAT4X4 mat_need = data_now->second.get_world_matrix();
			world_matrix_array.push_back(mat_need);
		}
	}
	return world_matrix_array;
}
int geometry_resource_view::add_an_instance(XMFLOAT4X4 world_matrix)
{
	unique_ptr<geometry_instance_view> p1(new geometry_instance_view(ID_instance_index));
	p1->update(world_matrix,0);
	std::pair<int, geometry_instance_view> data_need(ID_instance_index, *p1);
	auto check_iferror = instance_list.insert(data_need);
	if (!check_iferror.second)
	{
		return -1;
	}
	ID_instance_index += 1;
	return ID_instance_index-1;
}
engine_basic::engine_fail_reason geometry_resource_view::delete_an_instance(int instance_ID)
{
	auto data_now = instance_list.find(instance_ID);
	if (data_now == instance_list.end())
	{
		stringstream stream, stream2;
		stream << instance_ID;
		stream2 << resource_view_ID;
		string string_temp = stream.str();
		string string_temp2 = stream2.str();
		engine_basic::engine_fail_reason error_message(string("could not find instance of the model") + "instanceID:" + string_temp + "modelID:" + string_temp2);
	}
	instance_list.erase(data_now);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason geometry_resource_view::sleep_a_instance(int instance_ID)
{
	auto data_now = instance_list.find(instance_ID);
	if (data_now == instance_list.end())
	{
		stringstream stream, stream2;
		stream << instance_ID;
		stream2 << resource_view_ID;
		string string_temp = stream.str();
		string string_temp2 = stream2.str();
		engine_basic::engine_fail_reason error_message(string("could not find instance of the model") + "instanceID:" + string_temp + "modelID:" + string_temp2);
	}
	data_now->second.sleep();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason geometry_resource_view::wakeup_a_instance(int instance_ID)
{
	auto data_now = instance_list.find(instance_ID);
	if (data_now == instance_list.end())
	{
		stringstream stream, stream2;
		stream << instance_ID;
		stream2 << resource_view_ID;
		string string_temp = stream.str();
		string string_temp2 = stream2.str();
		engine_basic::engine_fail_reason error_message(string("could not find instance of the model") + "instanceID:" + string_temp + "modelID:" + string_temp2);
	}
	data_now->second.wakeup();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason geometry_resource_view::update(int instance_ID, XMFLOAT4X4 mat_world, float delta_time)
{
	auto data_now = instance_list.find(instance_ID);
	if (data_now == instance_list.end())
	{
		stringstream stream, stream2;
		stream << instance_ID;
		stream2 << resource_view_ID;
		string string_temp = stream.str();
		string string_temp2 = stream2.str();
		engine_basic::engine_fail_reason error_message(string("could not find instance of the model") + "instanceID:" + string_temp + "modelID:" + string_temp2);
	}
	data_now->second.update(mat_world, delta_time);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void geometry_resource_view::draw(bool if_static)
{
	//查看模型访问器中的实例数量
	if (world_matrix_array.size() > 0)
	{
		if (if_static == true && if_dynamic == true) 
		{
			//当前的渲染pass只渲染静态物体，并且当前物体是动态的则跳过该次渲染
			return;
		}
		if (world_matrix_array.size() > 1)
		{
			//实例数量大于一则调用多遍绘制drawcall
			model_data->draw_mesh_instance(world_matrix_array.size());
		}
		else
		{
			//实例数量等于一则调用一遍绘制drawcall
			model_data->draw_mesh();
		}
	}
}
void geometry_resource_view::release() 
{
	model_data->release();
}


pancy_geometry_control_singleton::pancy_geometry_control_singleton() 
{
	model_view_list = new geometry_ResourceView_list<geometry_resource_view>();
}
engine_basic::engine_fail_reason pancy_geometry_control_singleton::load_a_model_type(string file_name_mesh, string file_name_mat, bool if_dynamic, int &model_type_ID)
{
	//从文件中导入模型资源
	model_reader_pancymesh *model_common = new model_reader_pancymesh_build<point_common>(file_name_mesh, file_name_mat);
	auto error_check = model_common->create();
	if (!error_check.check_if_failed())
	{
		model_type_ID = -1;
		return error_check;
	}
	//根据模型资源创建访问器
	geometry_resource_view *GRV_model = new geometry_resource_view(model_common, model_view_list->get_now_index(), if_dynamic);
	//添加到模型管理表中
	model_type_ID = model_view_list->add_new_geometry(*GRV_model);
	if (model_type_ID < 0)
	{
		engine_basic::engine_fail_reason error_mssage("could not add the model resource" + file_name_mesh);
		return error_mssage;
	}
	delete GRV_model;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_geometry_control_singleton::delete_a_model_type(int model_type_ID)
{
	return model_view_list->delete_geometry_byindex(model_type_ID);
}
engine_basic::engine_fail_reason pancy_geometry_control_singleton::get_a_model_type(geometry_resource_view **data_out, int model_type_ID)
{
	*data_out = model_view_list->get_geometry_byindex(model_type_ID);
	if (data_out == NULL)
	{
		stringstream stream;
		stream << model_type_ID;
		string string_temp = stream.str();
		engine_basic::engine_fail_reason error_mssage("could not get the model resource ID:" + model_type_ID);
		return error_mssage;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
//加载和删除一个模型实例
engine_basic::engine_fail_reason pancy_geometry_control_singleton::add_a_model_instance(int model_type_ID, XMFLOAT4X4 world_Matrix, pancy_model_ID &model_ID)
{
	stringstream stream;
	stream << model_type_ID;
	string string_temp = stream.str();
	auto model_view_data = model_view_list->get_geometry_byindex(model_type_ID);
	if (model_view_data == NULL)
	{
		engine_basic::engine_fail_reason error_mssage("could not find the model type ID:" + string_temp);
		return error_mssage;
	}
	int model_ID_now = model_view_data->add_an_instance(world_Matrix);
	if (model_type_ID < 0)
	{
		engine_basic::engine_fail_reason error_mssage("could not add the instance to model ID:" + string_temp);
		return error_mssage;
	}
	model_ID.model_type = model_type_ID;
	model_ID.model_instance = model_ID_now;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason pancy_geometry_control_singleton::delete_a_model_instance(pancy_model_ID model_ID)
{
	auto model_view_data = model_view_list->get_geometry_byindex(model_ID.model_type);
	if (model_view_data == NULL)
	{
		stringstream stream;
		stream << model_ID.model_type;
		string string_temp = stream.str();
		engine_basic::engine_fail_reason error_mssage("could not find the model type ID:" + string_temp);
		return error_mssage;
	}
	return model_view_data->delete_an_instance(model_ID.model_instance);
}
engine_basic::engine_fail_reason pancy_geometry_control_singleton::update_a_model_instance(pancy_model_ID model_ID, XMFLOAT4X4 world_Matrix, float delta_time)
{
	auto model_view_data = model_view_list->get_geometry_byindex(model_ID.model_type);
	if (model_view_data == NULL)
	{
		stringstream stream;
		stream << model_ID.model_type;
		string string_temp = stream.str();
		engine_basic::engine_fail_reason error_mssage("could not find the model type ID:" + string_temp);
		return error_mssage;
	}
	return model_view_data->update(model_ID.model_instance, world_Matrix, delta_time);
}
engine_basic::engine_fail_reason pancy_geometry_control_singleton::sleep_a_model_instance(pancy_model_ID model_ID)
{
	auto model_view_data = model_view_list->get_geometry_byindex(model_ID.model_type);
	if (model_view_data == NULL)
	{
		stringstream stream;
		stream << model_ID.model_type;
		string string_temp = stream.str();
		engine_basic::engine_fail_reason error_mssage("could not find the model type ID:" + string_temp);
		return error_mssage;
	}
	return model_view_data->sleep_a_instance(model_ID.model_instance);
}
engine_basic::engine_fail_reason pancy_geometry_control_singleton::wakeup_a_model_instance(pancy_model_ID model_ID)
{
	auto model_view_data = model_view_list->get_geometry_byindex(model_ID.model_type);
	if (model_view_data == NULL)
	{
		stringstream stream;
		stream << model_ID.model_type;
		string string_temp = stream.str();
		engine_basic::engine_fail_reason error_mssage("could not find the model type ID:" + string_temp);
		return error_mssage;
	}
	return model_view_data->wakeup_a_instance(model_ID.model_instance);
}
void pancy_geometry_control_singleton::release()
{
	model_view_list->release();
}
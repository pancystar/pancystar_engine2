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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~骨骼动画~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
model_reader_PancySkinMesh::model_reader_PancySkinMesh(string file_name_mesh, string file_name_mat, string file_name_bone) : model_reader_pancymesh_build<point_skincommon>(file_name_mesh, file_name_mat)
{
	hand_matrix_number = 0;
	root_skin = new skin_tree;
	strcpy(root_skin->bone_ID, "root_node");
	root_skin->son = new skin_tree;
	bone_file_name = file_name_bone;
	bone_num = 0;
	time_all = 0.0f;
	for (int i = 0; i < MAX_BONE_NUM; ++i)
	{
		XMStoreFloat4x4(&bone_matrix_array[i], XMMatrixIdentity());
		XMStoreFloat4x4(&offset_matrix_array[i], XMMatrixIdentity());
		XMStoreFloat4x4(&final_matrix_array[i], XMMatrixIdentity());
	}
}
engine_basic::engine_fail_reason model_reader_PancySkinMesh::load_model()
{
	engine_basic::engine_fail_reason check_error;
	check_error = load_skintree(bone_file_name);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = load_model_mesh();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason model_reader_PancySkinMesh::set_animation_byname(string name)
{
	for (auto anim_data = animation_list.begin(); anim_data != animation_list.end(); ++anim_data)
	{
		if (anim_data->animation_name == name)
		{
			engine_basic::engine_fail_reason succeed;
			return succeed;
		}
	}
	engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the animation" + name);
	return error_message;
}
engine_basic::engine_fail_reason model_reader_PancySkinMesh::set_animation_byindex(int index)
{
	if (index < 0 || index >= animation_list.size())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "animation index out of size");
		return error_message;
	}
	now_animation_choose = index;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

bool model_reader_PancySkinMesh::check_ifsame(char a[], char b[])
{
	int length = strlen(a);
	if (strlen(a) != strlen(b))
	{
		return false;
	}
	for (int i = 0; i < length; ++i)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}
	return true;
}
engine_basic::engine_fail_reason model_reader_PancySkinMesh::load_skintree(string filename)
{
	skin_instream.open(filename, ios::binary);
	if (!skin_instream.is_open())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "open file " + filename + " error");
		return error_message;
	}
	//读取偏移矩阵
	int bone_num_need;
	skin_instream.read(reinterpret_cast<char*>(&bone_num), sizeof(bone_num));
	skin_instream.read(reinterpret_cast<char*>(offset_matrix_array), bone_num*sizeof(XMFLOAT4X4));
	//先读取第一个入栈符
	char data[11];
	skin_instream.read(reinterpret_cast<char*>(data), sizeof(data));
	root_skin = new skin_tree();
	//递归重建骨骼树
	read_bone_tree(root_skin);
	//关闭文件
	skin_instream.close();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void model_reader_PancySkinMesh::read_bone_tree(skin_tree *now)
{
	//char data[11];
	//skin_instream.read(reinterpret_cast<char*>(now), sizeof(*now));
	//skin_instream.read(data, sizeof(data));
	char data[11];
	skin_instream.read(reinterpret_cast<char*>(now), sizeof(*now));
	now->brother = NULL;
	now->son = NULL;
	skin_instream.read(data, sizeof(data));
	while (strcmp(data, "*heaphead*") == 0)
	{
		//入栈符号，代表子节点
		skin_tree *now_point = new skin_tree();
		read_bone_tree(now_point);
		now_point->brother = now->son;
		now->son = now_point;
		skin_instream.read(data, sizeof(data));
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

engine_basic::engine_fail_reason model_reader_PancySkinMesh::load_animation_list(string file_name_animation, int &anim_ID)
{
	skin_instream.open(file_name_animation, ios::binary);
	animation_set anim_read;
	skin_instream.read(anim_read.animation_name, sizeof(anim_read.animation_name));
	skin_instream.read(reinterpret_cast<char*>(&anim_read.animation_length), sizeof(anim_read.animation_length));
	skin_instream.read(reinterpret_cast<char*>(&anim_read.number_animation), sizeof(anim_read.number_animation));
	for (int i = 0; i < anim_read.number_animation; ++i)
	{
		animation_data now_animdata;
		//骨骼名称
		skin_instream.read(now_animdata.bone_name, sizeof(now_animdata.bone_name));
		now_animdata.bone_point = find_tree(root_skin, now_animdata.bone_name);
		//旋转四元数
		skin_instream.read(reinterpret_cast<char*>(&now_animdata.number_rotation), sizeof(now_animdata.number_rotation));
		if (now_animdata.number_rotation != 0)
		{
			now_animdata.rotation_key = new quaternion_animation[now_animdata.number_rotation];
			skin_instream.read(reinterpret_cast<char*>(now_animdata.rotation_key), now_animdata.number_rotation * sizeof(quaternion_animation));
		}
		//缩放向量
		skin_instream.read(reinterpret_cast<char*>(&now_animdata.number_scaling), sizeof(now_animdata.number_scaling));
		if (now_animdata.number_scaling != 0)
		{
			now_animdata.scaling_key = new vector_animation[now_animdata.number_scaling];
			skin_instream.read(reinterpret_cast<char*>(now_animdata.scaling_key), now_animdata.number_scaling * sizeof(vector_animation));
		}
		//平移向量
		skin_instream.read(reinterpret_cast<char*>(&now_animdata.number_translation), sizeof(now_animdata.number_translation));
		if (now_animdata.number_translation != 0)
		{
			now_animdata.translation_key = new vector_animation[now_animdata.number_translation];
			skin_instream.read(reinterpret_cast<char*>(now_animdata.translation_key), now_animdata.number_translation * sizeof(vector_animation));
		}
		//加入迭代器并释放缓存
		anim_read.animation_datalist.push_back(now_animdata);
	}
	//加入迭代器并释放缓存
	anim_ID = animation_list.size();
	animation_list.push_back(anim_read);
	skin_instream.close();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
skin_tree* model_reader_PancySkinMesh::find_tree(skin_tree* p, char name[])
{
	if (check_ifsame(p->bone_ID, name))
	{
		return p;
	}
	else
	{
		skin_tree* q;
		if (p->brother != NULL)
		{
			q = find_tree(p->brother, name);
			if (q != NULL)
			{
				return q;
			}
		}
		if (p->son != NULL)
		{
			q = find_tree(p->son, name);
			if (q != NULL)
			{
				return q;
			}
		}
	}
	return NULL;
}
skin_tree* model_reader_PancySkinMesh::find_tree(skin_tree* p, int num)
{
	if (p->bone_number == num)
	{
		return p;
	}
	else
	{
		skin_tree* q;
		if (p->brother != NULL)
		{
			q = find_tree(p->brother, num);
			if (q != NULL)
			{
				return q;
			}
		}
		if (p->son != NULL)
		{
			q = find_tree(p->son, num);
			if (q != NULL)
			{
				return q;
			}
		}
	}
	return NULL;
}
void model_reader_PancySkinMesh::update_root(skin_tree *root, XMFLOAT4X4 matrix_parent)
{
	if (root == NULL)
	{
		return;
	}
	XMMATRIX rec = XMLoadFloat4x4(&root->animation_matrix);
	XMStoreFloat4x4(&root->now_matrix, rec * XMLoadFloat4x4(&matrix_parent));
	if (root->bone_number >= 0)
	{
		bone_matrix_array[root->bone_number] = root->now_matrix;
	}
	update_root(root->brother, matrix_parent);
	update_root(root->son, root->now_matrix);
}

void model_reader_PancySkinMesh::release()
{
	model_out_test->release();
	test_resource->Release();
	free_tree(root_skin);
	free_animation();
}
void model_reader_PancySkinMesh::free_animation()
{
	for (auto anim_setdata = animation_list.begin(); anim_setdata != animation_list.end(); ++anim_setdata)
	{
		for (auto anim_data = anim_setdata->animation_datalist.begin(); anim_data != anim_setdata->animation_datalist.end(); ++anim_data)
		{
			if (anim_data->rotation_key != NULL)
			{
				delete[] anim_data->rotation_key;
			}
			if (anim_data->scaling_key != NULL)
			{
				delete[] anim_data->scaling_key;
			}
			if (anim_data->translation_key != NULL)
			{
				delete[] anim_data->translation_key;
			}
		}
		anim_setdata->animation_datalist.clear();
	}
	animation_list.clear();
}
void model_reader_PancySkinMesh::free_tree(skin_tree *now)
{
	if (now->brother != NULL)
	{
		free_tree(now->brother);
	}
	if (now->son != NULL)
	{
		free_tree(now->son);
	}
	if (now != NULL)
	{
		free(now);
	}
}

XMFLOAT4X4* model_reader_PancySkinMesh::get_bone_matrix()
{
	for (int i = 0; i < bone_num; ++i)
	{
		XMStoreFloat4x4(&final_matrix_array[i], XMLoadFloat4x4(&offset_matrix_array[i]) * XMLoadFloat4x4(&bone_matrix_array[i]));
	}
	return final_matrix_array;
}
void model_reader_PancySkinMesh::update_animation(float delta_time)
{
	time_all = (time_all + delta_time);
	if (time_all >= animation_list[now_animation_choose].animation_length)
	{
		time_all -= animation_list[now_animation_choose].animation_length;
	}
	update_anim_data();
	XMFLOAT4X4 matrix_identi;
	XMStoreFloat4x4(&matrix_identi, XMMatrixIdentity());
	update_root(root_skin, matrix_identi);
}
void model_reader_PancySkinMesh::specify_animation_time(float animation_time)
{
	if (animation_time < 0.0f || animation_time > get_animation_length())
	{
		return;
	}
	time_all = animation_time;
	update_anim_data();
	XMFLOAT4X4 matrix_identi;
	XMStoreFloat4x4(&matrix_identi, XMMatrixIdentity());
	update_root(root_skin, matrix_identi);
}
void model_reader_PancySkinMesh::update_anim_data()
{
	for (auto now = animation_list[now_animation_choose].animation_datalist.begin(); now != animation_list[now_animation_choose].animation_datalist.end(); ++now)
	{
		XMMATRIX rec_trans, rec_scal;
		XMFLOAT4X4 rec_rot;
		int start_anim, end_anim;
		find_anim_sted(start_anim, end_anim, now->rotation_key, now->number_rotation);
		//四元数插值并寻找变换矩阵
		quaternion_animation rotation_now;
		if (start_anim == end_anim || end_anim >= now->number_rotation)
		{
			rotation_now = now->rotation_key[start_anim];
		}
		else
		{
			Interpolate(rotation_now, now->rotation_key[start_anim], now->rotation_key[end_anim], (time_all - now->rotation_key[start_anim].time) / (now->rotation_key[end_anim].time - now->rotation_key[start_anim].time));
		}
		Get_quatMatrix(rec_rot, rotation_now);
		//缩放变换
		find_anim_sted(start_anim, end_anim, now->scaling_key, now->number_scaling);
		vector_animation scalling_now;
		if (start_anim == end_anim)
		{
			scalling_now = now->scaling_key[start_anim];
		}
		else
		{
			Interpolate(scalling_now, now->scaling_key[start_anim], now->scaling_key[end_anim], (time_all - now->scaling_key[start_anim].time) / (now->scaling_key[end_anim].time - now->scaling_key[start_anim].time));
		}
		rec_scal = XMMatrixScaling(scalling_now.main_key[0], scalling_now.main_key[1], scalling_now.main_key[2]);
		//平移变换
		find_anim_sted(start_anim, end_anim, now->translation_key, now->number_translation);
		vector_animation translation_now;
		if (start_anim == end_anim)
		{
			translation_now = now->translation_key[start_anim];
		}
		else
		{
			Interpolate(translation_now, now->translation_key[start_anim], now->translation_key[end_anim], (time_all - now->translation_key[start_anim].time) / (now->translation_key[end_anim].time - now->translation_key[start_anim].time));
		}
		rec_trans = XMMatrixTranslation(translation_now.main_key[0], translation_now.main_key[1], translation_now.main_key[2]);
		XMStoreFloat4x4(&now->bone_point->animation_matrix, rec_scal * XMLoadFloat4x4(&rec_rot) * rec_trans);
	}
}
void model_reader_PancySkinMesh::Interpolate(quaternion_animation& pOut, quaternion_animation pStart, quaternion_animation pEnd, float pFactor)
{
	float cosom = pStart.main_key[0] * pEnd.main_key[0] + pStart.main_key[1] * pEnd.main_key[1] + pStart.main_key[2] * pEnd.main_key[2] + pStart.main_key[3] * pEnd.main_key[3];
	quaternion_animation end = pEnd;
	if (cosom < static_cast<float>(0.0))
	{
		cosom = -cosom;
		end.main_key[0] = -end.main_key[0];
		end.main_key[1] = -end.main_key[1];
		end.main_key[2] = -end.main_key[2];
		end.main_key[3] = -end.main_key[3];
	}
	float sclp, sclq;
	if ((static_cast<float>(1.0) - cosom) > static_cast<float>(0.0001))
	{
		float omega, sinom;
		omega = acos(cosom);
		sinom = sin(omega);
		sclp = sin((static_cast<float>(1.0) - pFactor) * omega) / sinom;
		sclq = sin(pFactor * omega) / sinom;
	}
	else
	{
		sclp = static_cast<float>(1.0) - pFactor;
		sclq = pFactor;
	}

	pOut.main_key[0] = sclp * pStart.main_key[0] + sclq * end.main_key[0];
	pOut.main_key[1] = sclp * pStart.main_key[1] + sclq * end.main_key[1];
	pOut.main_key[2] = sclp * pStart.main_key[2] + sclq * end.main_key[2];
	pOut.main_key[3] = sclp * pStart.main_key[3] + sclq * end.main_key[3];
}
void model_reader_PancySkinMesh::Interpolate(vector_animation& pOut, vector_animation pStart, vector_animation pEnd, float pFactor)
{
	for (int i = 0; i < 3; ++i)
	{
		pOut.main_key[i] = pStart.main_key[i] + pFactor * (pEnd.main_key[i] - pStart.main_key[i]);
	}
}
void model_reader_PancySkinMesh::find_anim_sted(int &st, int &ed, quaternion_animation *input, int num_animation)
{
	if (time_all < 0)
	{
		st = 0;
		ed = 1;
		return;
	}
	if (time_all > input[num_animation - 1].time)
	{
		st = num_animation - 1;
		ed = num_animation - 1;
		return;
	}
	for (int i = 0; i < num_animation; ++i)
	{
		if (time_all >= input[i].time && time_all <= input[i + 1].time)
		{
			st = i;
			ed = i + 1;
			return;
		}
	}
	st = num_animation - 1;
	ed = num_animation - 1;
}
void model_reader_PancySkinMesh::find_anim_sted(int &st, int &ed, vector_animation *input, int num_animation)
{
	if (time_all < 0)
	{
		st = 0;
		ed = 1;
		return;
	}
	if (time_all > input[num_animation - 1].time)
	{
		st = num_animation - 1;
		ed = num_animation - 1;
		return;
	}
	for (int i = 0; i < num_animation - 1; ++i)
	{
		if (time_all >= input[i].time && time_all <= input[i + 1].time)
		{
			st = i;
			ed = i + 1;
			return;
		}
	}
	st = num_animation - 1;
	ed = num_animation - 1;
}
void model_reader_PancySkinMesh::Get_quatMatrix(XMFLOAT4X4 &resMatrix, quaternion_animation& pOut)
{
	resMatrix._11 = static_cast<float>(1.0) - static_cast<float>(2.0) * (pOut.main_key[1] * pOut.main_key[1] + pOut.main_key[2] * pOut.main_key[2]);
	resMatrix._21 = static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[1] - pOut.main_key[2] * pOut.main_key[3]);
	resMatrix._31 = static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[2] + pOut.main_key[1] * pOut.main_key[3]);
	resMatrix._41 = 0.0f;

	resMatrix._12 = static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[1] + pOut.main_key[2] * pOut.main_key[3]);
	resMatrix._22 = static_cast<float>(1.0) - static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[0] + pOut.main_key[2] * pOut.main_key[2]);
	resMatrix._32 = static_cast<float>(2.0) * (pOut.main_key[1] * pOut.main_key[2] - pOut.main_key[0] * pOut.main_key[3]);
	resMatrix._42 = 0.0f;

	resMatrix._13 = static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[2] - pOut.main_key[1] * pOut.main_key[3]);
	resMatrix._23 = static_cast<float>(2.0) * (pOut.main_key[1] * pOut.main_key[2] + pOut.main_key[0] * pOut.main_key[3]);
	resMatrix._33 = static_cast<float>(1.0) - static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[0] + pOut.main_key[1] * pOut.main_key[1]);
	resMatrix._43 = 0.0f;

	resMatrix._14 = 0.0f;
	resMatrix._24 = 0.0f;
	resMatrix._34 = 0.0f;
	resMatrix._44 = 1.0f;
}
//几何体的实例访问信息
geometry_instance_view::geometry_instance_view(int ID_need)
{
	now_animation_use = -1;
	animation_time = 0.0f;
	if_skin = false;
	skindata = NULL;
	if_show = true;
	instance_ID = ID_need;
	XMStoreFloat4x4(&world_matrix, XMMatrixIdentity());
}
geometry_instance_view::geometry_instance_view(int ID_need, model_reader_PancySkinMesh *skin_data_in)
{
	now_animation_use = -1;
	animation_time = 0.0f;
	if_skin = true;
	skindata = skin_data_in;
	if_show = true;
	instance_ID = ID_need;
	XMStoreFloat4x4(&world_matrix, XMMatrixIdentity());
}
engine_basic::engine_fail_reason geometry_instance_view::get_bone_matrix(XMFLOAT4X4** mat_out, int &bone_num)
{
	if (!if_skin)
	{
		bone_num = 0;
		mat_out = NULL;
		engine_basic::engine_fail_reason check_error(E_FAIL, "the model do not have bone &skin");
		return check_error;
	}
	bone_num = skindata->get_bone_num();
	*mat_out = bone_matrix;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason geometry_instance_view::Set_AnimationUse_ByID(int anim_ID)
{
	auto check_error = skindata->set_animation_byindex(anim_ID);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	now_animation_use = anim_ID;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void geometry_instance_view::update(XMFLOAT4X4 mat_in, float delta_time)
{
	animation_time += delta_time;
	if (if_skin && now_animation_use != -1)
	{
		//更新骨骼变换矩阵
		skindata->set_animation_byindex(now_animation_use);
		animation_time = fmod(animation_time, skindata->get_animation_length());
		skindata->specify_animation_time(animation_time);
		skindata->update_animation(delta_time);
		XMFLOAT4X4 *bonemat_ptr = skindata->get_bone_matrix();
		for (int i = 0; i < skindata->get_bone_num(); ++i)
		{
			bone_matrix[i] = bonemat_ptr[i];
		}
		world_matrix = mat_in;
	}
}




geometry_resource_view::geometry_resource_view(model_reader_pancymesh *model_data_in, int ID_need, bool if_dynamic_in, bool if_skin_in)
{
	ID_instance_index = 0;
	max_instance_num = 0;
	resource_view_ID = ID_need;
	model_data = model_data_in;
	if_cull_front = false;
	if_dynamic = if_dynamic_in;
	if_skin = if_skin_in;
}
engine_basic::engine_fail_reason geometry_resource_view::create(int max_instance_num_in)
{
	max_instance_num = max_instance_num_in;
	if (if_skin)
	{
		auto check_error = create_buffer();
		if (!check_error.check_if_failed())
		{
			return check_error;
		}
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason geometry_resource_view::create_buffer()
{
	ID3D11Buffer *bone_matrix_buffer;
	D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
	//还原动画数据
	auto animation_point = dynamic_cast<model_reader_PancySkinMesh*>(model_data);

	D3D11_BUFFER_DESC HDR_buffer_desc;
	HDR_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;            //通用类型
	HDR_buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;//缓存类型为uav+srv
	HDR_buffer_desc.ByteWidth = max_instance_num * animation_point->get_bone_num() * sizeof(XMFLOAT4X4);        //顶点缓存的大小
	HDR_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HDR_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	HDR_buffer_desc.StructureByteStride = sizeof(XMFLOAT4X4);


	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&HDR_buffer_desc, NULL, &bone_matrix_buffer);
	if (FAILED(hr))
	{
		stringstream stream;
		stream << resource_view_ID;
		string string_temp = stream.str();
		engine_basic::engine_fail_reason error_message(hr, "create bone matrix buffer error in model resourceL: " + string_temp);
		return error_message;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC DescSRV;
	ZeroMemory(&DescSRV, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	DescSRV.Format = DXGI_FORMAT_UNKNOWN;
	DescSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	DescSRV.Buffer.FirstElement = 0;
	DescSRV.Buffer.NumElements = max_instance_num * animation_point->get_bone_num();
	
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(bone_matrix_buffer, &DescSRV,&bone_matrix_buffer_SRV);
	if (FAILED(hr))
	{
		stringstream stream;
		stream << resource_view_ID;
		string string_temp = stream.str();
		engine_basic::engine_fail_reason error_message(hr, "create bone matrix buffer error in model resourceL: " + string_temp);
		return error_message;
	}
	bone_matrix_buffer->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
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
ID3D11ShaderResourceView * geometry_resource_view::get_bone_matrix_list()
{
	if (!if_skin)
	{
		return NULL;
	}
	auto animation_point = dynamic_cast<model_reader_PancySkinMesh*>(model_data);
	XMFLOAT4X4 *bone_matrix_CPU_buffer;
	int now_point = 0;
	bone_matrix_CPU_buffer = new XMFLOAT4X4[instance_list.size() * animation_point->get_bone_num()];
	for (auto data_now = instance_list.begin(); data_now != instance_list.end(); ++data_now)
	{
		XMFLOAT4X4 *data;
		int bone_num;
		engine_basic::engine_fail_reason check_error = data_now->second.get_bone_matrix(&data, bone_num);
		for (int i = 0; i < bone_num; ++i)
		{
			XMStoreFloat4x4(&bone_matrix_CPU_buffer[now_point++],XMMatrixTranspose(XMLoadFloat4x4(&data[i])));
			//bone_matrix_CPU_buffer[now_point++] = data[i];
		}
	}
	ID3D11Resource *data;
	bone_matrix_buffer_SRV->GetResource(&data);
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Map(data, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, bone_matrix_CPU_buffer, instance_list.size() * animation_point->get_bone_num()*sizeof(XMFLOAT4X4));
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Unmap(data, 0);
	data->Release();
	/*
	bone_matrix_array.clear();
	for (auto data_now = instance_list.begin(); data_now != instance_list.end(); ++data_now)
	{
		if (data_now->second.check_if_show())
		{
			XMFLOAT4X4 *data;
			int bone_num;
			engine_basic::engine_fail_reason check_error = data_now->second.get_bone_matrix(&data, bone_num);
			for (int i = 0; i < bone_num; ++i)
			{
				bone_matrix_array.push_back(data[i]);
			}
		}
	}
	return bone_matrix_array;
	*/
	return bone_matrix_buffer_SRV;
}
int geometry_resource_view::get_bone_mat_num()
{
	if (!if_skin)
	{
		return 0;
	}
	auto animation_point = dynamic_cast<model_reader_PancySkinMesh*>(model_data);
	return animation_point->get_bone_num();
}
engine_basic::engine_fail_reason geometry_resource_view::Load_Animation_FromFile(string file_mame, int &anim_ID)
{
	stringstream stream;
	stream << resource_view_ID;
	string string_temp = stream.str();
	if (!if_skin)
	{
		engine_basic::engine_fail_reason error_mssage("the model type ID:" + string_temp + "is not a skin model");
		return error_mssage;
	}
	auto animation_point = dynamic_cast<model_reader_PancySkinMesh*>(model_data);
	engine_basic::engine_fail_reason check_error = animation_point->load_animation_list(file_mame, anim_ID);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
int geometry_resource_view::add_an_instance(XMFLOAT4X4 world_matrix)
{
	geometry_instance_view *p1;
	if (!if_skin)
	{
		p1 = new geometry_instance_view(ID_instance_index);
		//unique_ptr<geometry_instance_view> p1(new geometry_instance_view(ID_instance_index));
	}
	else
	{
		auto animation_point = dynamic_cast<model_reader_PancySkinMesh*>(model_data);
		p1 = new geometry_instance_view(ID_instance_index, animation_point);
		///unique_ptr<geometry_instance_view> p1(new geometry_instance_view(ID_instance_index));
	}
	p1->update(world_matrix, 0);
	std::pair<int, geometry_instance_view> data_need(ID_instance_index, *p1);
	auto check_iferror = instance_list.insert(data_need);
	if (!check_iferror.second)
	{
		return -1;
	}
	ID_instance_index += 1;
	delete p1;
	return ID_instance_index - 1;

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
engine_basic::engine_fail_reason geometry_resource_view::set_a_instance_anim(int instance_ID, int animation_ID)
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
	engine_basic::engine_fail_reason check_error = data_now->second.Set_AnimationUse_ByID(animation_ID);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
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
	if (if_skin)
	{
		bone_matrix_buffer_SRV->Release();
	}
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
	geometry_resource_view *GRV_model = new geometry_resource_view(model_common, model_view_list->get_now_index(), if_dynamic, false);
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
engine_basic::engine_fail_reason pancy_geometry_control_singleton::load_a_skinmodel_type(string file_name_mesh, string file_name_mat, string file_name_bone, bool if_dynamic, int &model_type_ID, int max_instance_num)
{
	//从文件中导入模型资源
	model_reader_pancymesh *model_common = new model_reader_PancySkinMesh(file_name_mesh, file_name_mat, file_name_bone);
	auto error_check = model_common->create();
	if (!error_check.check_if_failed())
	{
		model_type_ID = -1;
		return error_check;
	}
	//根据模型资源创建访问器
	geometry_resource_view *GRV_model = new geometry_resource_view(model_common, model_view_list->get_now_index(), if_dynamic, true);
	error_check = GRV_model->create(max_instance_num);
	if (!error_check.check_if_failed())
	{
		model_type_ID = -1;
		return error_check;
	}
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
engine_basic::engine_fail_reason pancy_geometry_control_singleton::load_a_skinmodel_animation(int model_type_ID, string file_name_animation, int& animition_ID)
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
	auto check_error = model_view_data->Load_Animation_FromFile(file_name_animation, animition_ID);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
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
engine_basic::engine_fail_reason pancy_geometry_control_singleton::set_a_instance_animation(pancy_model_ID model_ID, int animation_ID)
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
	auto check_error = model_view_data->set_a_instance_anim(model_ID.model_instance, animation_ID);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
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
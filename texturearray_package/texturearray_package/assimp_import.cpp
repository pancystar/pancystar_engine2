#include"assimp_import.h"
assimp_basic::assimp_basic(const char* pFile, const char *texture_path)
{
	filename = pFile;
	strcpy(rec_texpath, texture_path);
	model_need = NULL;
	matlist_need = NULL;
	mesh_need = NULL;
}
std::string assimp_basic::get_mesh_name_bypart(int mesh_id)
{
	if (mesh_id >= meshpart_name.size())
	{
		return "";
	}
	else 
	{
		return meshpart_name[mesh_id];
	}
}
int assimp_basic::get_meshnum()
{
	if (model_need == NULL)
	{
		return 0;
	}
	return mesh_optimization;
}
void assimp_basic::remove_texture_path(char rec[])
{
	int rec_num = strlen(rec);
	int start = 0;
	for (int i = 0; i < rec_num; ++i)
	{
		if (rec[i] == '\\')
		{
			start = i + 1;
		}
	}
	strcpy(rec, &rec[start]);
}
void assimp_basic::change_texturedesc_2dds(char rec[])
{
	int rec_num = strlen(rec);
	int start = 0;
	for (int i = 0; i < rec_num; ++i)
	{
		if (rec[i] == '.')
		{
			rec[i + 1] = 'd';
			rec[i + 2] = 'd';
			rec[i + 3] = 's';
			rec[i + 4] = '\0';
		}
	}
	strcpy(rec, &rec[start]);
}
engine_basic::engine_fail_reason assimp_basic::model_create(bool if_adj,int alpha_partnum, int* alpha_part)
{
	//aiProcess_ConvertToLeftHanded;
	model_need = importer.ReadFile(filename,
		aiProcess_MakeLeftHanded |
		aiProcess_FlipWindingOrder |
		aiProcess_CalcTangentSpace |             //计算切线和副法线
												 //aiProcess_Triangulate |                 //将四边形面转换为三角面
		aiProcess_JoinIdenticalVertices		//合并相同的顶点
											//aiProcess_SortByPType
		);//将不同图元放置到不同的模型中去，图片类型可能是点、直线、三角形等
	if (!model_need)
	{
		engine_basic::engine_fail_reason fail_message("read model" + filename + "error");
		return fail_message;
	}
	matlist_need = new material_list[model_need->mNumMaterials];
	for (unsigned int i = 0; i < model_need->mNumMaterials; ++i)
	{
		const aiMaterial* pMaterial = model_need->mMaterials[i];
		aiString Path;
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0 && pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			strcpy(matlist_need[i].texture_diffuse, Path.data);
			remove_texture_path(matlist_need[i].texture_diffuse);
			change_texturedesc_2dds(matlist_need[i].texture_diffuse);
			char rec_name[512];
			strcpy(rec_name, rec_texpath);
			strcat(rec_name, matlist_need[i].texture_diffuse);
			strcpy(matlist_need[i].texture_diffuse, rec_name);
		}
		if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0 && pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			strcpy(matlist_need[i].texture_normal, Path.data);
			remove_texture_path(matlist_need[i].texture_normal);
			change_texturedesc_2dds(matlist_need[i].texture_normal);
			char rec_name[512];
			strcpy(rec_name, rec_texpath);
			strcat(rec_name, matlist_need[i].texture_normal);
			strcpy(matlist_need[i].texture_normal, rec_name);
		}
	}
	HRESULT hr;
	engine_basic::engine_fail_reason check_fail = init_mesh(if_adj);
	if (!check_fail.check_if_failed())
	{
		return check_fail;
	}
	check_fail = init_texture();
	if (!check_fail.check_if_failed())
	{
		return check_fail;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
bool assimp_basic::check_if_anim()
{
	if (model_need->mNumAnimations == 0) 
	{
		return false;
	}
	return true;
}
engine_basic::engine_fail_reason assimp_basic::init_texture()
{
	HRESULT hr_need;
	for (int i = 0; i < model_need->mNumMaterials; ++i)
	{
		//创建漫反射贴图
		if (matlist_need[i].texture_diffuse[0] != '\0')
		{
			//转换文件名为unicode
			size_t len = strlen(matlist_need[i].texture_diffuse) + 1;
			size_t converted = 0;
			wchar_t *texture_name;
			texture_name = (wchar_t*)malloc(len*sizeof(wchar_t));
			mbstowcs_s(&converted, texture_name, len, matlist_need[i].texture_diffuse, _TRUNCATE);
			//根据文件名创建纹理资源
			//hr_need = CreateDDSTextureFromFile(device_pancy, texture_name, 0, &matlist_need[i].tex_diffuse_resource, 0, 0);
			hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), texture_name, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0., false, NULL, &matlist_need[i].tex_diffuse_resource);
			if (FAILED(hr_need))
			{
				engine_basic::engine_fail_reason check_fail(hr_need,std::string("create diffuse texture") + matlist_need[i].texture_diffuse + "error");
				return check_fail;
			}
			//释放临时文件名
			free(texture_name);
			texture_name = NULL;
		}
		//创建法线贴图
		if (matlist_need[i].texture_normal[0] != '\0')
		{
			//转换文件名为unicode
			size_t len = strlen(matlist_need[i].texture_normal) + 1;
			size_t converted = 0;
			wchar_t *texture_name;
			texture_name = (wchar_t*)malloc(len*sizeof(wchar_t));
			mbstowcs_s(&converted, texture_name, len, matlist_need[i].texture_normal, _TRUNCATE);
			//根据文件名创建纹理资源

			ID3D11Resource* Texture = NULL;
			//hr_need = CreateDDSTextureFromFile(device_pancy, texture_name, &Texture, &matlist_need[i].texture_normal_resource, 0, 0);
			hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), texture_name, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0., false, NULL, &matlist_need[i].texture_normal_resource);

			//contex_pancy->GenerateMips();
			if (FAILED(hr_need))
			{
				engine_basic::engine_fail_reason check_fail(hr_need, std::string("create normal texture") + matlist_need[i].texture_normal + "error");
				return check_fail;
			}
			//释放临时文件名
			free(texture_name);
			texture_name = NULL;
		}
	}
	material_optimization = model_need->mNumMaterials;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
HRESULT assimp_basic::get_technique(ID3DX11EffectTechnique *teque_need)
{
	if (teque_need == NULL)
	{
		MessageBox(0, L"get render technique error", L"tip", MB_OK);
		return E_FAIL;
	}
	teque_pancy = teque_need;
	return S_OK;
}
void assimp_basic::get_texture(material_list *texture_need, int i)
{
	*texture_need = matlist_need[mesh_need[i].material_use];
}
void assimp_basic::get_texture_byindex(material_list *texture_need, int index)
{
	*texture_need = matlist_need[index];
}
void assimp_basic::release()
{
	if (model_need != NULL)
	{
		//释放纹理资源
		for (int i = 0; i < material_optimization; ++i)
		{
			if (matlist_need[i].tex_diffuse_resource != NULL)
			{
				matlist_need[i].tex_diffuse_resource->Release();
				matlist_need[i].tex_diffuse_resource = NULL;
			}
			if (matlist_need[i].texture_normal_resource != NULL)
			{
				matlist_need[i].texture_normal_resource->Release();
				matlist_need[i].texture_normal_resource = NULL;
			}
		}
		//释放缓冲区资源
		for (int i = 0; i < mesh_optimization; i++)
		{
			mesh_need[i].point_buffer->release();
		}
		//释放表资源
		delete[] mesh_need;
		//释放表资源
		delete[] matlist_need;
		model_need->~aiScene();
	}
}
void assimp_basic::draw_part(int i)
{
	mesh_need[i].point_buffer->get_teque(teque_pancy);
	mesh_need[i].point_buffer->show_mesh();
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~骨骼动画~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
model_reader_skin::model_reader_skin(const char* filename, const char* texture_path) : model_reader_assimp(filename, texture_path)
{
	hand_matrix_number = 0;
	root_skin = new skin_tree;
	strcpy(root_skin->bone_ID, "root_node");
	root_skin->son = new skin_tree;
	first_animation = NULL;
	bone_num = 0;
	time_all = 0.0f;
	//float rec_need1[16] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	for (int i = 0; i < 100; ++i)
	{
		XMStoreFloat4x4(&bone_matrix_array[i], XMMatrixIdentity());
		XMStoreFloat4x4(&offset_matrix_array[i], XMMatrixIdentity());
		XMStoreFloat4x4(&final_matrix_array[i], XMMatrixIdentity());
	}

	for (int i = 0; i <	 100; ++i)
	{
		for (int j = 0; j < 100; ++j)
		{
			tree_node_num[i][j] = 0;
		}
	}
}
void model_reader_skin::set_matrix(XMFLOAT4X4 &out, aiMatrix4x4 *in)
{
	out._11 = in->a1;
	out._21 = in->a2;
	out._31 = in->a3;
	out._41 = in->a4;

	out._12 = in->b1;
	out._22 = in->b2;
	out._32 = in->b3;
	out._42 = in->b4;

	out._13 = in->c1;
	out._23 = in->c2;
	out._33 = in->c3;
	out._43 = in->c4;

	out._14 = in->d1;
	out._24 = in->d2;
	out._34 = in->d3;
	out._44 = in->d4;
}

bool model_reader_skin::check_ifsame(char a[], char b[])
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
aiNode *model_reader_skin::find_skinroot(aiNode *now_node, char root_name[])
{
	if (now_node == NULL)
	{
		return NULL;
	}
	if (check_ifsame(root_name, now_node->mName.data))
	{
		return now_node;
	}
	for (int i = 0; i < now_node->mNumChildren; ++i)
	{
		aiNode *p = find_skinroot(now_node->mChildren[i], root_name);
		if (p != NULL)
		{
			return p;
		}
	}
	return NULL;
}
HRESULT model_reader_skin::build_skintree(aiNode *now_node, skin_tree *now_root)
{
	if (now_node != NULL)
	{
		strcpy(now_root->bone_ID, now_node->mName.data);
		set_matrix(now_root->animation_matrix, &now_node->mTransformation);
		//set_matrix(now_root->basic_matrix, &now_node->mTransformation);
		now_root->bone_number = -12138;
		//bone_num += 1;
		if (now_node->mNumChildren > 0)
		{
			//如果有至少一个儿子则建立儿子结点
			now_root->son = new skin_tree();
			build_skintree(now_node->mChildren[0], now_root->son);
		}
		skin_tree *p = now_root->son;
		for (int i = 1; i < now_node->mNumChildren; ++i)
		{
			//建立所有的兄弟链
			p->brother = new skin_tree();
			build_skintree(now_node->mChildren[i], p->brother);
			p = p->brother;
		}
	}
	return S_OK;

}
HRESULT model_reader_skin::build_animation_list()
{
	for (int i = 0; i < model_need->mNumAnimations; ++i)
	{
		animation_set *p = new animation_set;
		p->animation_length = model_need->mAnimations[i]->mDuration;
		strcpy(p->animation_name, model_need->mAnimations[i]->mName.data);
		p->number_animation = model_need->mAnimations[i]->mNumChannels;
		p->head_animition = NULL;
		for (int j = 0; j < p->number_animation; ++j)
		{
			animation_data *q = new animation_data;
			strcpy(q->bone_name, model_need->mAnimations[i]->mChannels[j]->mNodeName.data);
			q->bone_point = find_tree(root_skin, q->bone_name);
			//旋转四元数
			q->number_rotation = model_need->mAnimations[i]->mChannels[j]->mNumRotationKeys;
			q->rotation_key = new quaternion_animation[q->number_rotation];

			for (int k = 0; k < q->number_rotation; ++k)
			{
				q->rotation_key[k].time = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mTime;
				q->rotation_key[k].main_key[0] = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.x;
				q->rotation_key[k].main_key[1] = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.y;
				q->rotation_key[k].main_key[2] = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.z;
				q->rotation_key[k].main_key[3] = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.w;
			}
			//平移向量
			q->number_translation = model_need->mAnimations[i]->mChannels[j]->mNumPositionKeys;
			q->translation_key = new vector_animation[q->number_translation];
			for (int k = 0; k < q->number_translation; ++k)
			{
				q->translation_key[k].time = model_need->mAnimations[i]->mChannels[j]->mPositionKeys[k].mTime;
				q->translation_key[k].main_key[0] = model_need->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.x;
				q->translation_key[k].main_key[1] = model_need->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.y;
				q->translation_key[k].main_key[2] = model_need->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.z;
			}
			//缩放向量
			q->number_scaling = model_need->mAnimations[i]->mChannels[j]->mNumScalingKeys;
			q->scaling_key = new vector_animation[q->number_scaling];
			for (int k = 0; k < q->number_scaling; ++k)
			{
				q->scaling_key[k].time = model_need->mAnimations[i]->mChannels[j]->mScalingKeys[k].mTime;
				q->scaling_key[k].main_key[0] = model_need->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.x;
				q->scaling_key[k].main_key[1] = model_need->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.y;
				q->scaling_key[k].main_key[2] = model_need->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.z;
			}

			q->next = p->head_animition;
			p->head_animition = q;
		}
		p->next = first_animation;
		first_animation = p;
	}
	return S_OK;
}
skin_tree* model_reader_skin::find_tree(skin_tree* p, char name[])
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
skin_tree* model_reader_skin::find_tree(skin_tree* p, int num)
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
void model_reader_skin::update_root(skin_tree *root, XMFLOAT4X4 matrix_parent)
{
	if (root == NULL)
	{
		return;
	}
	//float rec[16];
	XMMATRIX rec = XMLoadFloat4x4(&root->animation_matrix);
	XMStoreFloat4x4(&root->now_matrix, rec * XMLoadFloat4x4(&matrix_parent));
	//memcpy(rec, root->animation_matrix, 16 * sizeof(float));
	//MatrixMultiply(root->now_matrix, rec, matrix_parent);
	if (root->bone_number >= 0)
	{//memcpy(&bone_matrix_array[root->bone_number], root->now_matrix, 16 * sizeof(float));
		bone_matrix_array[root->bone_number] = root->now_matrix;
	}
	update_root(root->brother, matrix_parent);
	update_root(root->son, root->now_matrix);
}
void model_reader_skin::update_mesh_offset(int i)
{
	for (int j = 0; j < model_need->mMeshes[i]->mNumBones; ++j)
	{
		set_matrix(offset_matrix_array[tree_node_num[i][j]], &model_need->mMeshes[i]->mBones[j]->mOffsetMatrix);
	}
}
void model_reader_skin::update_mesh_offset()
{
	for (int i = 0; i < model_need->mNumMeshes; ++i)
	{
		for (int j = 0; j < model_need->mMeshes[i]->mNumBones; ++j)
		{
			set_matrix(offset_matrix_array[tree_node_num[i][j]], &model_need->mMeshes[i]->mBones[j]->mOffsetMatrix);
		}
		int a = 0;
	}
	int b = 0;
}
int model_reader_skin::find_min(float x1, float x2, float x3, float x4)
{
	int min = 0;
	float min_rec = x1;
	if (x2 < x1)
	{
		min = 1;
		min_rec = x2;
	}
	if (x3 < x2)
	{
		min = 2;
		min_rec = x3;
	}
	if (x4 < x3)
	{
		min = 3;
		min_rec = x4;
	}
	return min;
}
XMFLOAT4X4 model_reader_skin::get_hand_matrix()
{
	return final_matrix_array[hand_matrix_number];
}
engine_basic::engine_fail_reason  model_reader_skin::init_mesh(bool if_adj)
{
	build_skintree(model_need->mRootNode, root_skin->son);
	build_animation_list();
	for (int i = 0; i < model_need->mNumMeshes; i++)
	{
		//获取模型的第i个模块
		const aiMesh* paiMesh = model_need->mMeshes[i];
		std::string data_name = paiMesh->mName.C_Str();
		int count = 0;
		for (int i = 0; i < data_name.size(); ++i)
		{
			if (data_name[i] == ' ')
			{
				count = i + 1;
			}
		}
		std::string final_name;
		for (int i = count; i < data_name.size(); ++i)
		{
			final_name += data_name[i];
		}
		meshpart_name.push_back(final_name);
		vertex_final_num += paiMesh->mNumVertices;
		index_pack_num += paiMesh->mNumFaces * 3;
	}
	point_pack_list = (point_skincommon*)malloc(vertex_final_num * sizeof(point_skincommon));
	index_pack_list = (unsigned int*)malloc(index_pack_num * sizeof(unsigned int));
	int count_point_pack = 0;
	int count_index_pack = 0;
	point_skincommon *point_need;
	unsigned int *index_need;
	//创建网格记录表
	mesh_need = new meshview_list[model_need->mNumMeshes];
	mesh_optimization = model_need->mNumMeshes;
	int now_used_bone_num = 0;
	int now_index_start = count_point_pack;
	for (int i = 0; i < model_need->mNumMeshes; i++)
	{
		//创建顶点缓存区
		const aiMesh* paiMesh = model_need->mMeshes[i];
		mesh_need[i].material_use = paiMesh->mMaterialIndex;
		
		point_need = (point_skincommon*)malloc(paiMesh->mNumVertices * sizeof(point_skincommon));
		index_need = (unsigned int*)malloc(paiMesh->mNumFaces * 3 * sizeof(unsigned int));
		for (int j = 0; j < paiMesh->mNumVertices; ++j)
		{
			point_need[j].bone_id.x = 99;
			point_need[j].bone_id.y = 99;
			point_need[j].bone_id.z = 99;
			point_need[j].bone_id.w = 99;
			point_need[j].bone_weight.x = 0.0f;
			point_need[j].bone_weight.y = 0.0f;
			point_need[j].bone_weight.z = 0.0f;
			point_need[j].bone_weight.w = 0.0f;
		}
		//顶点缓存
		for (unsigned int j = 0; j < paiMesh->mNumVertices; j++)
		{
			point_need[j].position.x = paiMesh->mVertices[j].x;
			point_need[j].position.y = paiMesh->mVertices[j].y;
			point_need[j].position.z = paiMesh->mVertices[j].z;

			point_need[j].normal.x = paiMesh->mNormals[j].x;
			point_need[j].normal.y = paiMesh->mNormals[j].y;
			point_need[j].normal.z = paiMesh->mNormals[j].z;

			if (paiMesh->HasTextureCoords(0))
			{
				point_need[j].tex.x = paiMesh->mTextureCoords[0][j].x;
				point_need[j].tex.y = 1 - paiMesh->mTextureCoords[0][j].y;
			}
			else
			{
				point_need[j].tex.x = 0.0f;
				point_need[j].tex.y = 0.0f;
			}
			if (paiMesh->mTangents != NULL)
			{
				point_need[j].tangent.x = paiMesh->mTangents[j].x;
				point_need[j].tangent.y = paiMesh->mTangents[j].y;
				point_need[j].tangent.z = paiMesh->mTangents[j].z;
			}
			else
			{
				point_need[j].tangent.x = 0.0f;
				point_need[j].tangent.y = 0.0f;
				point_need[j].tangent.z = 0.0f;
			}
		}

		for (int j = 0; j < paiMesh->mNumBones; ++j)
		{
			skin_tree * now_node = find_tree(root_skin, paiMesh->mBones[j]->mName.data);
			if (now_node->bone_number == -12138)
			{
				now_node->bone_number = now_used_bone_num++;
				/*
				if (strcmp(now_node->bone_ID, "GothGirlRArmPalm") == 0)
				{
					hand_matrix_number = now_used_bone_num - 1;
				}
				*/
				bone_num += 1;
			}
			tree_node_num[i][j] = now_node->bone_number;
			//set_matrix(now_node->basic_matrix, &paiMesh->mBones[j]->mOffsetMatrix);
			for (int k = 0; k < paiMesh->mBones[j]->mNumWeights; ++k)
			{
				if (point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.x == 99)
				{
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.x = now_node->bone_number;
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.x = paiMesh->mBones[j]->mWeights[k].mWeight;
				}
				else if (point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.y == 99)
				{
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.y = now_node->bone_number;
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.y = paiMesh->mBones[j]->mWeights[k].mWeight;
				}
				else if (point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.z == 99)
				{
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.z = now_node->bone_number;
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.z = paiMesh->mBones[j]->mWeights[k].mWeight;
				}
				else if (point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.w == 99)
				{
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.w = now_node->bone_number;
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.w = paiMesh->mBones[j]->mWeights[k].mWeight;
				}
				else
				{
					XMUINT4 rec_number = point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id;
					XMFLOAT4 rec_weight = point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight;
					int rec_min_weight = find_min(rec_weight.x, rec_weight.y, rec_weight.z, rec_weight.w);

					if (rec_min_weight == 0 && rec_weight.x < paiMesh->mBones[j]->mWeights[k].mWeight)
					{
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.x = now_node->bone_number;
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.x += paiMesh->mBones[j]->mWeights[k].mWeight;
					}
					if (rec_min_weight == 1 && rec_weight.y < paiMesh->mBones[j]->mWeights[k].mWeight)
					{
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.y = now_node->bone_number;
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.y += paiMesh->mBones[j]->mWeights[k].mWeight;
					}
					if (rec_min_weight == 2 && rec_weight.z < paiMesh->mBones[j]->mWeights[k].mWeight)
					{
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.z = now_node->bone_number;
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.z += paiMesh->mBones[j]->mWeights[k].mWeight;
					}
					if (rec_min_weight == 3 && rec_weight.w < paiMesh->mBones[j]->mWeights[k].mWeight)
					{
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.w = now_node->bone_number;
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.w += paiMesh->mBones[j]->mWeights[k].mWeight;
					}
				}
			}
		}

		for (int j = 0; j < paiMesh->mNumVertices; ++j)
		{
			point_pack_list[count_point_pack] = point_need[j];
			point_pack_list[count_point_pack].tex_id.x = i;
			count_point_pack += 1;
		}
		int count_index = 0;
		for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
		{
			if (paiMesh->mFaces[i].mNumIndices == 3)
			{
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[0];
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[1];
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[2];
				index_pack_list[count_index_pack++] = paiMesh->mFaces[j].mIndices[0] + now_index_start;
				index_pack_list[count_index_pack++] = paiMesh->mFaces[j].mIndices[1] + now_index_start;
				index_pack_list[count_index_pack++] = paiMesh->mFaces[j].mIndices[2] + now_index_start;
			}
			else
			{
				//index_need[count_index++] = 0;
				//index_need[count_index++] = 0;
				//index_need[count_index++] = 0;
				engine_basic::engine_fail_reason fail_message("model" + filename + "find no triangle face");
				return fail_message;
				//return E_FAIL;
			}
		}
		//根据内存信息创建显存区
		//模型的第i个模块的顶点及索引信息
		mesh_need[i].point_buffer = new mesh_model<point_skincommon>(point_need, index_need,paiMesh->mNumVertices, paiMesh->mNumFaces * 3,if_adj);
		auto check_error = mesh_need[i].point_buffer->create_object();
		if (!check_error.check_if_failed())
		{
			return check_error;
		}
		now_index_start = count_point_pack;
		//释放内存
		free(point_need);
		point_need = NULL;
		free(index_need);
		index_need = NULL;
	}


	//float matrix_identi[] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	XMFLOAT4X4 matrix_identi;
	XMStoreFloat4x4(&matrix_identi, XMMatrixIdentity());
	update_root(root_skin, matrix_identi);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void model_reader_skin::release_all()
{
	release();
	free_tree(root_skin);
}
void model_reader_skin::free_tree(skin_tree *now)
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
XMFLOAT4X4* model_reader_skin::get_bone_matrix(int i, int &num_bone)
{
	num_bone = model_need->mMeshes[i]->mNumBones;
	for (int j = 0; j < model_need->mMeshes[i]->mNumBones; ++j)
	{
		//MatrixMultiply(&final_matrix_array[tree_node_num[i][j] * 16], &offset_matrix_array[tree_node_num[i][j] * 16], &bone_matrix_array[tree_node_num[i][j] * 16]);
		XMStoreFloat4x4(&final_matrix_array[tree_node_num[i][j]], XMLoadFloat4x4(&offset_matrix_array[tree_node_num[i][j]]) * XMLoadFloat4x4(&bone_matrix_array[tree_node_num[i][j]]));
	}
	return final_matrix_array;
}
XMFLOAT4X4* model_reader_skin::get_bone_matrix()
{
	for (int i = 0; i < 100; ++i)
	{
		//MatrixMultiply(&final_matrix_array[tree_node_num[i][j] * 16], &offset_matrix_array[tree_node_num[i][j] * 16], &bone_matrix_array[tree_node_num[i][j] * 16]);
		XMStoreFloat4x4(&final_matrix_array[i], XMLoadFloat4x4(&offset_matrix_array[i]) * XMLoadFloat4x4(&bone_matrix_array[i]));
	}
	return final_matrix_array;
}
void model_reader_skin::update_animation(float delta_time)
{
	time_all = (time_all + delta_time);
	if (time_all >= first_animation->animation_length)
	{
		time_all -= first_animation->animation_length;
	}
	update_anim_data(first_animation->head_animition);
	//float matrix_identi[] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	XMFLOAT4X4 matrix_identi;
	XMStoreFloat4x4(&matrix_identi, XMMatrixIdentity());
	update_root(root_skin, matrix_identi);
}
void model_reader_skin::specify_animation_time(float animation_time)
{
	if (animation_time < 0.0f || animation_time > get_animation_length())
	{
		return;
	}
	time_all = animation_time;
	//if (time_all >= first_animation->animation_length)
	//{
	//	time_all -= first_animation->animation_length;
	//}
	update_anim_data(first_animation->head_animition);
	//float matrix_identi[] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	XMFLOAT4X4 matrix_identi;
	XMStoreFloat4x4(&matrix_identi, XMMatrixIdentity());
	update_root(root_skin, matrix_identi);
}
void model_reader_skin::update_anim_data(animation_data *now)
{
	//float rec_trans[16], rec_rot[16], rec_scal[16], rec_mid[16];
	XMMATRIX rec_trans, rec_scal;
	XMFLOAT4X4 rec_rot;
	int start_anim, end_anim;
	if (now == NULL)
	{
		return;
	}
	find_anim_sted(start_anim, end_anim, now->rotation_key, now->number_rotation);
	//四元数插值并寻找变换矩阵
	quaternion_animation rotation_now;
	//rotation_now = now->rotation_key[0];
	if (start_anim == end_anim || end_anim >= now->number_rotation)
	{
		rotation_now = now->rotation_key[start_anim];
	}
	else
	{
		Interpolate(rotation_now, now->rotation_key[start_anim], now->rotation_key[end_anim], (time_all - now->rotation_key[start_anim].time) / (now->rotation_key[end_anim].time - now->rotation_key[start_anim].time));
	}
	//
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
	//scalling_now = now->scaling_key[0];
	rec_scal = XMMatrixScaling(scalling_now.main_key[0], scalling_now.main_key[1], scalling_now.main_key[2]);
	//MatrixScaling(rec_scal, scalling_now.main_key[0], scalling_now.main_key[1], scalling_now.main_key[2]);
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
	//translation_now = now->translation_key[0];
	rec_trans = XMMatrixTranslation(translation_now.main_key[0], translation_now.main_key[1], translation_now.main_key[2]);
	//MatrixTranslation(rec_trans, translation_now.main_key[0], translation_now.main_key[1], translation_now.main_key[2]);
	//总变换
	//MatrixMultiply(rec_mid, rec_scal, rec_rot);
	//MatrixMultiply(now->bone_point->animation_matrix, rec_mid, rec_trans);
	XMStoreFloat4x4(&now->bone_point->animation_matrix, rec_scal * XMLoadFloat4x4(&rec_rot) * rec_trans);

	update_anim_data(now->next);
}
void model_reader_skin::Interpolate(quaternion_animation& pOut, quaternion_animation pStart, quaternion_animation pEnd, float pFactor)
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
void model_reader_skin::Interpolate(vector_animation& pOut, vector_animation pStart, vector_animation pEnd, float pFactor)
{
	for (int i = 0; i < 3; ++i)
	{
		pOut.main_key[i] = pStart.main_key[i] + pFactor * (pEnd.main_key[i] - pStart.main_key[i]);
	}
}
void model_reader_skin::find_anim_sted(int &st, int &ed, quaternion_animation *input, int num_animation)
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
void model_reader_skin::find_anim_sted(int &st, int &ed, vector_animation *input, int num_animation)
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
void model_reader_skin::Get_quatMatrix(XMFLOAT4X4 &resMatrix, quaternion_animation& pOut)
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
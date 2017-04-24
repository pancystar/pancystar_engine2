#include"assimp_import.h"
assimp_basic::assimp_basic(char* pFile, char *texture_path)
{
	filename = pFile;
	strcpy(rec_texpath, texture_path);
	model_need = NULL;
	matlist_need = NULL;
	mesh_need = NULL;
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
			char rec_name[128];
			strcpy(rec_name, rec_texpath);
			strcat(rec_name, matlist_need[i].texture_diffuse);
			strcpy(matlist_need[i].texture_diffuse, rec_name);
		}
		if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0 && pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			strcpy(matlist_need[i].texture_normal, Path.data);
			remove_texture_path(matlist_need[i].texture_normal);
			change_texturedesc_2dds(matlist_need[i].texture_normal);
			char rec_name[128];
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
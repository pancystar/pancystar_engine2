#include"assimp_import.h"
assimp_basic::assimp_basic(const char* pFile, const char *texture_path)
{
	filename = pFile;
	strcpy(rec_texpath, texture_path);
	model_need = NULL;
	model_need_CCW = NULL;
	matlist_need = NULL;
	mesh_need = NULL;
	if_fbx_meshanimation = false;
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
engine_basic::engine_fail_reason assimp_basic::model_create(bool if_adj, int alpha_partnum, int* alpha_part)
{
	HRESULT hr;
	engine_basic::engine_fail_reason check_fail;
	//aiProcess_ConvertToLeftHanded;
	model_need = importer.ReadFile(filename,
		aiProcess_MakeLeftHanded |
		aiProcess_FlipWindingOrder |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices
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
		else if (pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0 && pMaterial->GetTexture(aiTextureType_NORMALS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			strcpy(matlist_need[i].texture_normal, Path.data);
			remove_texture_path(matlist_need[i].texture_normal);
			change_texturedesc_2dds(matlist_need[i].texture_normal);
			char rec_name[512];
			strcpy(rec_name, rec_texpath);
			strcat(rec_name, matlist_need[i].texture_normal);
			strcpy(matlist_need[i].texture_normal, rec_name);
		}
		else 
		{
			strcpy(matlist_need[i].texture_normal,"F:\\Microsoft Visual Studio\\pancystar_engine2.0\\pancystar_engine2\\texturearray_package\\texturearray_package\\pbr_basic\\basic_normal.dds");
		}
	}

	check_fail = init_mesh(if_adj);
	if (!check_fail.check_if_failed())
	{
		return check_fail;
	}
	check_fail = init_texture();
	if (!check_fail.check_if_failed())
	{
		return check_fail;
	}


	FBXanim_import = new mesh_animation_FBX(filename, vertex_final_num, index_pack_num);
	check_fail = FBXanim_import->create(index_pack_list, normal_buffer, tangent_buffer);
	if (check_fail.check_if_failed())
	{
		if_fbx_meshanimation = true;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
bool assimp_basic::check_if_anim()
{
	if (model_need == NULL || model_need->mNumAnimations == 0)
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
			//hr_need = CreateDDSTextureFromFile(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), texture_name, 0, &matlist_need[i].tex_diffuse_resource, 0, 0);
			hr_need = CreateDDSTextureFromFileEx(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex(), texture_name, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0., false, NULL, &matlist_need[i].tex_diffuse_resource);
			if (FAILED(hr_need))
			{
				engine_basic::engine_fail_reason check_fail(hr_need, std::string("create diffuse texture") + matlist_need[i].texture_diffuse + "error");
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
		if (if_fbx_meshanimation)
		{
			FBXanim_import->release();
		}
		model_need->~aiScene();
	}

}
void assimp_basic::draw_part(int i)
{
	mesh_need[i].point_buffer->get_teque(teque_pancy);
	mesh_need[i].point_buffer->show_mesh();
}
int assimp_basic::get_part_offset(int part)
{
	int all_need = 0;
	int count_turn = 0;
	for (auto data_num = model_part_vertex_num.begin(); data_num != model_part_vertex_num.end(); ++data_num)
	{
		if (count_turn < part)
		{
			all_need += *data_num._Ptr;
			count_turn++;
		}
		else
		{
			break;
		}
	}
	return all_need;
}
//FBX网格动画
mesh_animation_FBX::mesh_animation_FBX(std::string file_name_in, int point_num_in, int point_index_num_in)
{
	if_fbx_file = false;
	if (file_name_in.size() >= 3) 
	{
		int tail = file_name_in.size() - 1;
		if (file_name_in[tail] == 'x' && file_name_in[tail - 1] == 'b' && file_name_in[tail - 2] == 'f') 
		{
			if_fbx_file = true;
		}
	}
	lFilePath = new FbxString(file_name_in.c_str());
	point_index_num = point_index_num_in;
	point_num = point_num_in;
}
engine_basic::engine_fail_reason mesh_animation_FBX::create(UINT *index_buffer_in, XMFLOAT3 *normal_in, XMFLOAT3 *tangent_in)
{
	if (!if_fbx_file)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "isn't a fbx file");
		return error_message;
	}
	engine_basic::engine_fail_reason check_error;
	InitializeSdkObjects(lSdkManager, lScene);
	if (lFilePath->IsEmpty())
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "need a file name");
		return error_message;
	}
	else
	{
		bool if_succeed = LoadScene(lSdkManager, lScene, lFilePath->Buffer());
		if (if_succeed == false)
		{
			engine_basic::engine_fail_reason error_message(E_FAIL, "An error occurred while loading the scene" + std::string(lFilePath->Buffer()));
			return error_message;
		}
	}
	auto pNode = lScene->GetRootNode();
	//获取网格信息
	if (pNode == NULL)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the root node of FBX file" + std::string(lFilePath->Buffer()));
		return error_message;
	}
	check_error = find_tree_mesh(pNode);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	//检验动画信息
	const bool lHasVertexCache = lMesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
		(static_cast<FbxVertexCacheDeformer*>(lMesh->GetDeformer(0, FbxDeformer::eVertexCache)))->Active.Get();
	if (!lHasVertexCache)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "the FBX file don't have animation message" + std::string(lFilePath->Buffer()));
		return error_message;
	}
	//获取时间信息
	PreparePointCacheData(lScene, anim_start, anim_end);
	auto FPS_rec = anim_end.GetFrameRate(fbxsdk::FbxTime::EMode::eDefaultMode);
	auto framenum_rec = anim_end.GetFrameCount();
	frame_per_second = static_cast<int>(FPS_rec);
	frame_num = static_cast<int>(framenum_rec);
	anim_frame.SetTime(0, 0, 0, 1, 0, lScene->GetGlobalSettings().GetTimeMode());
	if (frame_num == 0)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the mesh animation of FBX file" + std::string(lFilePath->Buffer()));
		return error_message;
	}

	//检验模型是否匹配
	int lPolygonCount = lMesh->GetPolygonCount();
	if (lPolygonCount * 3 != point_index_num)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "The model from assimp have different face count" + std::string(lFilePath->Buffer()));
		return error_message;
	}
	index_buffer = new UINT[point_index_num];
	for (int i = 0; i < point_index_num / 3; ++i)
	{
		index_buffer[i * 3 + 0] = index_buffer_in[i * 3 + 0];
		index_buffer[i * 3 + 1] = index_buffer_in[i * 3 + 1];
		index_buffer[i * 3 + 2] = index_buffer_in[i * 3 + 2];
	}
	/*
	int rec_mid;
	rec_mid = index_buffer[traingle_point_2];
	index_buffer[traingle_point_2] = index_buffer[traingle_point_0];
	index_buffer[traingle_point_0] = rec_mid;
	*/
	//开启顶点动画缓冲
	auto time_now = anim_start;
	FbxVector4* lVertexArray = NULL;
	lVertexArray = new FbxVector4[lVertexCount];
	for (int i = 0; i < frame_num; ++i)
	{
		time_now += anim_frame;
		int check = time_now.GetFrameCount();
		check_error = ReadVertexCacheData(lMesh, time_now, lVertexArray);
		if (!check_error.check_if_failed())
		{
			return check_error;
		}
		UpdateVertexPosition(lMesh, lVertexArray, normal_in, tangent_in);
	}
	//计算法线
	//compute_normal();
	bool lResult = true;
	DestroySdkObjects(lSdkManager, lResult);
	check_error = build_buffer();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason mesh_animation_FBX::build_buffer()
{
	ID3D11Buffer *buffer_vertex_need;
	//XMFLOAT3 *point_data_now;
	//point_data_now = new XMFLOAT3[anim_data_list.size() * anim_data_list.begin()._Ptr->point_num];
	mesh_animation_data *point_data_now;
	point_data_now = new mesh_animation_data[anim_data_list.size() * anim_data_list.begin()._Ptr->point_num];
	int size = 0;
	for (auto data_check = anim_data_list.begin(); data_check != anim_data_list.end(); ++data_check)
	{
		for (int i = 0; i < data_check->point_num; ++i)
		{
			point_data_now[size++] = data_check->point_data[i];
		}
	}
	//创建顶点动画缓冲区
	D3D11_BUFFER_DESC FBX_vertex_desc;
	FBX_vertex_desc.Usage = D3D11_USAGE_DEFAULT;            //通用类型
	FBX_vertex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;//缓存类型为srv
	FBX_vertex_desc.ByteWidth = size*sizeof(mesh_animation_data);        //顶点缓存的大小
	FBX_vertex_desc.CPUAccessFlags = 0;
	FBX_vertex_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	FBX_vertex_desc.StructureByteStride = sizeof(mesh_animation_data);

	D3D11_SUBRESOURCE_DATA resource_buffer = { 0 };
	resource_buffer.pSysMem = point_data_now;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&FBX_vertex_desc, &resource_buffer, &buffer_vertex_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create FBX model animation data buffer error");
		return check_error;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC DescSRV;
	ZeroMemory(&DescSRV, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	DescSRV.Format = DXGI_FORMAT_UNKNOWN;
	DescSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	DescSRV.Buffer.FirstElement = 0;
	DescSRV.Buffer.NumElements = size;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(buffer_vertex_need, &DescSRV, &point_buffer);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason check_error(hr, "create FBX model animation data SRV buffer error");
		return check_error;
	}
	buffer_vertex_need->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void mesh_animation_FBX::release()
{
	point_buffer->Release();
}
engine_basic::engine_fail_reason mesh_animation_FBX::find_tree_mesh(FbxNode *pNode)
{
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
	if (lNodeAttribute && lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		lMesh = pNode->GetMesh();
		lVertexCount = lMesh->GetControlPointsCount();
		engine_basic::engine_fail_reason succeed;
		return succeed;
	}
	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		return find_tree_mesh(pNode->GetChild(lChildIndex));
	}
	engine_basic::engine_fail_reason error_message(E_FAIL, "could not find the mesh data in FBX file");
	return error_message;
}
void mesh_animation_FBX::InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
	//The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
	pManager = FbxManager::Create();
	if (!pManager)
	{
		FBXSDK_printf("Error: Unable to create FBX Manager!\n");
		exit(1);
	}
	else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	pScene = FbxScene::Create(pManager, "My Scene");
	if (!pScene)
	{
		FBXSDK_printf("Error: Unable to create FBX scene!\n");
		exit(1);
	}
}
void mesh_animation_FBX::DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
{
	//Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
	if (pManager) pManager->Destroy();
	if (pExitStatus) FBXSDK_printf("Program Success!\n");
}
bool mesh_animation_FBX::SaveScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename, int pFileFormat, bool pEmbedMedia)
{
	int lMajor, lMinor, lRevision;
	bool lStatus = true;

	// Create an exporter.
	FbxExporter* lExporter = FbxExporter::Create(pManager, "");

	if (pFileFormat < 0 || pFileFormat >= pManager->GetIOPluginRegistry()->GetWriterFormatCount())
	{
		// Write in fall back format in less no ASCII format found
		pFileFormat = pManager->GetIOPluginRegistry()->GetNativeWriterFormat();

		//Try to export in ASCII if possible
		int lFormatIndex, lFormatCount = pManager->GetIOPluginRegistry()->GetWriterFormatCount();

		for (lFormatIndex = 0; lFormatIndex < lFormatCount; lFormatIndex++)
		{
			if (pManager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
			{
				FbxString lDesc = pManager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
				const char *lASCII = "ascii";
				if (lDesc.Find(lASCII) >= 0)
				{
					pFileFormat = lFormatIndex;
					break;
				}
			}
		}
	}

	// Set the export states. By default, the export states are always set to 
	// true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below 
	// shows how to change these states.
	IOS_REF.SetBoolProp(EXP_FBX_MATERIAL, true);
	IOS_REF.SetBoolProp(EXP_FBX_TEXTURE, true);
	IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED, pEmbedMedia);
	IOS_REF.SetBoolProp(EXP_FBX_SHAPE, true);
	IOS_REF.SetBoolProp(EXP_FBX_GOBO, true);
	IOS_REF.SetBoolProp(EXP_FBX_ANIMATION, true);
	IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

	// Initialize the exporter by providing a filename.
	if (lExporter->Initialize(pFilename, pFileFormat, pManager->GetIOSettings()) == false)
	{
		FBXSDK_printf("Call to FbxExporter::Initialize() failed.\n");
		FBXSDK_printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
		return false;
	}

	FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
	FBXSDK_printf("FBX file format version %d.%d.%d\n\n", lMajor, lMinor, lRevision);

	// Export the scene.
	lStatus = lExporter->Export(pScene);

	// Destroy the exporter.
	lExporter->Destroy();
	return lStatus;
}
bool mesh_animation_FBX::LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
{
	int lFileMajor, lFileMinor, lFileRevision;
	int lSDKMajor, lSDKMinor, lSDKRevision;
	//int lFileFormat = -1;
	int i, lAnimStackCount;
	bool lStatus;
	char lPassword[1024];

	// Get the file version number generate by the FBX SDK.
	FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

	// Create an importer.
	FbxImporter* lImporter = FbxImporter::Create(pManager, "");

	// Initialize the importer by providing a filename.
	const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
	lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

	if (!lImportStatus)
	{
		FbxString error = lImporter->GetStatus().GetErrorString();
		FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
		FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

		if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
		{
			FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
			FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
		}

		return false;
	}

	FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

	if (lImporter->IsFBX())
	{
		FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

		// From this point, it is possible to access animation stack information without
		// the expense of loading the entire file.

		FBXSDK_printf("Animation Stack Information\n");

		lAnimStackCount = lImporter->GetAnimStackCount();

		FBXSDK_printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
		FBXSDK_printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
		FBXSDK_printf("\n");

		for (i = 0; i < lAnimStackCount; i++)
		{
			FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

			FBXSDK_printf("    Animation Stack %d\n", i);
			FBXSDK_printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
			FBXSDK_printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

			// Change the value of the import name if the animation stack should be imported 
			// under a different name.
			FBXSDK_printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

			// Set the value of the import state to false if the animation stack should be not
			// be imported. 
			FBXSDK_printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
			FBXSDK_printf("\n");
		}

		// Set the import states. By default, the import states are always set to 
		// true. The code below shows how to change these states.
		IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
		IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
		IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
		IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
		IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
		IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
		IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
	}

	// Import the scene.
	lStatus = lImporter->Import(pScene);

	if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
	{
		FBXSDK_printf("Please enter password: ");

		lPassword[0] = '\0';

		FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
			scanf("%s", lPassword);
		FBXSDK_CRT_SECURE_NO_WARNING_END

			FbxString lString(lPassword);

		IOS_REF.SetStringProp(IMP_FBX_PASSWORD, lString);
		IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

		lStatus = lImporter->Import(pScene);

		if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
		{
			FBXSDK_printf("\nPassword is wrong, import aborted.\n");
		}
	}

	// Destroy the importer.
	lImporter->Destroy();

	return lStatus;
}
void mesh_animation_FBX::PreparePointCacheData(FbxScene* pScene, FbxTime &pCache_Start, FbxTime &pCache_Stop)
{
	// This function show how to cycle through scene elements in a linear way.
	const int lNodeCount = pScene->GetSrcObjectCount<FbxNode>();
	FbxStatus lStatus;

	for (int lIndex = 0; lIndex < lNodeCount; lIndex++)
	{
		FbxNode* lNode = pScene->GetSrcObject<FbxNode>(lIndex);

		if (lNode->GetGeometry())
		{
			int i, lVertexCacheDeformerCount = lNode->GetGeometry()->GetDeformerCount(FbxDeformer::eVertexCache);

			// There should be a maximum of 1 Vertex Cache Deformer for the moment
			lVertexCacheDeformerCount = lVertexCacheDeformerCount > 0 ? 1 : 0;

			for (i = 0; i < lVertexCacheDeformerCount; ++i)
			{
				// Get the Point Cache object
				FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(lNode->GetGeometry()->GetDeformer(i, FbxDeformer::eVertexCache));
				if (!lDeformer) continue;
				FbxCache* lCache = lDeformer->GetCache();
				if (!lCache) continue;

				// Process the point cache data only if the constraint is active
				if (lDeformer->Active.Get())
				{
					auto data_check = lCache->GetCacheFileFormat();
					if (lCache->GetCacheFileFormat() == FbxCache::eMaxPointCacheV2)
					{
						// This code show how to convert from PC2 to MC point cache format
						// turn it on if you need it.
#if 0 
						if (!lCache->ConvertFromPC2ToMC(FbxCache::eMCOneFile,
							FbxTime::GetFrameRate(pScene->GetGlobalTimeSettings().GetTimeMode())))
						{
							// Conversion failed, retrieve the error here
							FbxString lTheErrorIs = lCache->GetStaus().GetErrorString();
						}
#endif
					}
					else if (lCache->GetCacheFileFormat() == FbxCache::eMayaCache)
					{
						// This code show how to convert from MC to PC2 point cache format
						// turn it on if you need it.
						//#if 0 
						if (!lCache->ConvertFromMCToPC2(FbxTime::GetFrameRate(pScene->GetGlobalSettings().GetTimeMode()), 0, &lStatus))
						{
							// Conversion failed, retrieve the error here
							FbxString lTheErrorIs = lStatus.GetErrorString();
						}
						//#endif
					}


					// Now open the cache file to read from it
					if (!lCache->OpenFileForRead(&lStatus))
					{
						// Cannot open file 
						FbxString lTheErrorIs = lStatus.GetErrorString();

						// Set the deformer inactive so we don't play it back
						lDeformer->Active = false;
					}
					else
					{
						// get the start and stop time of the cache
						FbxTime lChannel_Start;
						FbxTime lChannel_Stop;
						int lChannelIndex = lCache->GetChannelIndex(lDeformer->Channel.Get());
						if (lCache->GetAnimationRange(lChannelIndex, lChannel_Start, lChannel_Stop))
						{
							// get the smallest start time
							if (lChannel_Start < pCache_Start) pCache_Start = lChannel_Start;

							// get the biggest stop time
							if (lChannel_Stop > pCache_Stop)  pCache_Stop = lChannel_Stop;
						}
					}
				}
			}
		}
	}
}
XMFLOAT3 mesh_animation_FBX::get_normal_vert(FbxMesh * pMesh, int vertex_count)
{
	XMFLOAT3 pNormal;
	int rec = pMesh->GetElementNormalCount();
	FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal(0);
	auto mode_map = leNormal->GetMappingMode();
	auto mode_reference = leNormal->GetReferenceMode();
	if (mode_reference == FbxGeometryElement::eDirect) 
	{
		pNormal.x = leNormal->GetDirectArray().GetAt(vertex_count)[0];
		pNormal.y = leNormal->GetDirectArray().GetAt(vertex_count)[1];
		pNormal.z = leNormal->GetDirectArray().GetAt(vertex_count)[2];
	}
	else 
	{
		int id = leNormal->GetIndexArray().GetAt(vertex_count);
		pNormal.x = leNormal->GetDirectArray().GetAt(id)[0];
		pNormal.y = leNormal->GetDirectArray().GetAt(id)[1];
		pNormal.z = leNormal->GetDirectArray().GetAt(id)[2];
	}
	return pNormal;
}
engine_basic::engine_fail_reason mesh_animation_FBX::ReadVertexCacheData(FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray)
{
	FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(pMesh->GetDeformer(0, FbxDeformer::eVertexCache));
	FbxCache*               lCache = lDeformer->GetCache();
	int                     lChannelIndex = lCache->GetChannelIndex(lDeformer->Channel.Get());
	unsigned int            lVertexCount = (unsigned int)pMesh->GetControlPointsCount();
	bool                    lReadSucceed = false;
	float*                  lReadBuf = NULL;
	unsigned int			BufferSize = 0;
	FbxString lChnlName, lChnlInterp;
	lCache->GetChannelInterpretation(lChannelIndex, lChnlInterp);
	if (lDeformer->Type.Get() != FbxVertexCacheDeformer::ePositions)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "animation data type not support");
		return error_message;
	}
	unsigned int Length = 0;
	lCache->Read(NULL, Length, FBXSDK_TIME_ZERO, lChannelIndex);
	if (Length != lVertexCount * 3)
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "the content of the cache is by vertex not by control points (we don't support it here)");
		return error_message;
	}
	lReadSucceed = lCache->Read(&lReadBuf, BufferSize, pTime, lChannelIndex);
	//lReadSucceed = lCache->Read(&lReadBuf, BufferSize, pTime, 1);
	if (lReadSucceed)
	{
		unsigned int lReadBufIndex = 0;
		while (lReadBufIndex < 3 * lVertexCount)
		{
			pVertexArray[lReadBufIndex / 3].mData[0] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
			pVertexArray[lReadBufIndex / 3].mData[1] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
			pVertexArray[lReadBufIndex / 3].mData[2] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
		}
	}
	else
	{
		engine_basic::engine_fail_reason error_message(E_FAIL, "read animation data error");
		return error_message;
	}
	/*
	//检验控制点变换效果
	FbxVector4 check_rec1 = pMesh->GetControlPointAt(500);
	std::vector<XMFLOAT3> normle_need,normle_need2;
	for (int i = 0; i < pMesh->GetPolygonCount() * 3; ++i)
	{
		normle_need.push_back(get_normal_vert(pMesh,i));
	}

	for (int i = 0; i < pMesh->GetControlPointsCount(); ++i)
	{
		FbxVector4 pVertices_now = pVertexArray[i];
		pMesh->SetControlPointAt(pVertices_now, i);
	}
	FbxVector4 check_rec2 = pMesh->GetControlPointAt(500);
	//创建新的法线及切线
	bool check_tre = pMesh->GenerateNormals(true);
	bool check_tre2 = pMesh->GenerateTangentsDataForAllUVSets(true);
	//重新检验
	for (int i = 0; i < pMesh->GetPolygonCount() * 3; ++i)
	{
		normle_need2.push_back(get_normal_vert(pMesh, i));
	}
	*/


	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void mesh_animation_FBX::UpdateVertexPosition(FbxMesh * pMesh, const FbxVector4 * pVertices, XMFLOAT3 *normal_in,XMFLOAT3 *tangent_in)
{
	//创建基于assimp的顶点数组
	mesh_animation_per_frame now_frame_data(point_num);
	//读取fbx动画数据
	int TRIANGLE_VERTEX_COUNT = 3;
	int VERTEX_STRIDE = 4;
	// Convert to the same sequence with data in GPU.
	float * lVertices = NULL;
	int lVertexCount = 0;
	const int lPolygonCount = pMesh->GetPolygonCount();
	lVertexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
	lVertices = new float[lVertexCount * VERTEX_STRIDE];
	
	lVertexCount = 0;
	for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
	{
		
		//获取当前对应的assimp顶点
		int traingle_point_0 = lPolygonIndex * TRIANGLE_VERTEX_COUNT + 0;
		int traingle_point_1 = lPolygonIndex * TRIANGLE_VERTEX_COUNT + 1;
		int traingle_point_2 = lPolygonIndex * TRIANGLE_VERTEX_COUNT + 2;
		const int lControlPointIndex_0 = pMesh->GetPolygonVertex(lPolygonIndex, 0);
		const int lControlPointIndex_1 = pMesh->GetPolygonVertex(lPolygonIndex, 1);
		const int lControlPointIndex_2 = pMesh->GetPolygonVertex(lPolygonIndex, 2);

		int vertex_index_assimp_0 = index_buffer[traingle_point_2];
		int vertex_index_assimp_1 = index_buffer[traingle_point_1];
		int vertex_index_assimp_2 = index_buffer[traingle_point_0];
		now_frame_data.point_data[vertex_index_assimp_0].position.x = static_cast<float>(pVertices[lControlPointIndex_0][0]);
		now_frame_data.point_data[vertex_index_assimp_0].position.y = static_cast<float>(pVertices[lControlPointIndex_0][1]);
		now_frame_data.point_data[vertex_index_assimp_0].position.z = -static_cast<float>(pVertices[lControlPointIndex_0][2]);
		//now_frame_data.point_data[vertex_index_assimp_0].normal = normal_in[vertex_index_assimp_0];
		//now_frame_data.point_data[vertex_index_assimp_0].tangent = tangent_in[vertex_index_assimp_0];
		//now_frame_data.point_data[vertex_index_assimp_0].normal.x = normal_in.x;
		//now_frame_data.point_data[vertex_index_assimp_0].normal.y = get_normal_vert(pMesh, lVertexCount).y;
		//now_frame_data.point_data[vertex_index_assimp_0].normal.z = get_normal_vert(pMesh, lVertexCount).z;

		now_frame_data.point_data[vertex_index_assimp_1].position.x = static_cast<float>(pVertices[lControlPointIndex_1][0]);
		now_frame_data.point_data[vertex_index_assimp_1].position.y = static_cast<float>(pVertices[lControlPointIndex_1][1]);
		now_frame_data.point_data[vertex_index_assimp_1].position.z = -static_cast<float>(pVertices[lControlPointIndex_1][2]);
		//now_frame_data.point_data[vertex_index_assimp_1].normal = normal_in[vertex_index_assimp_1];
		//now_frame_data.point_data[vertex_index_assimp_1].tangent = tangent_in[vertex_index_assimp_1];
		//now_frame_data.point_data[vertex_index_assimp_1].normal.x += get_normal_vert(pMesh, lVertexCount+1).x;
		//now_frame_data.point_data[vertex_index_assimp_1].normal.y += get_normal_vert(pMesh, lVertexCount+1).y;
		//now_frame_data.point_data[vertex_index_assimp_1].normal.z += get_normal_vert(pMesh, lVertexCount+1).z;


		now_frame_data.point_data[vertex_index_assimp_2].position.x = static_cast<float>(pVertices[lControlPointIndex_2][0]);
		now_frame_data.point_data[vertex_index_assimp_2].position.y = static_cast<float>(pVertices[lControlPointIndex_2][1]);
		now_frame_data.point_data[vertex_index_assimp_2].position.z = -static_cast<float>(pVertices[lControlPointIndex_2][2]);
		//now_frame_data.point_data[vertex_index_assimp_2].normal = normal_in[vertex_index_assimp_2];
		//now_frame_data.point_data[vertex_index_assimp_2].tangent = tangent_in[vertex_index_assimp_2];
		//now_frame_data.point_data[vertex_index_assimp_2].normal.x += get_normal_vert(pMesh, lVertexCount + 2).x;
		//now_frame_data.point_data[vertex_index_assimp_2].normal.y += get_normal_vert(pMesh, lVertexCount + 2).y;
		//now_frame_data.point_data[vertex_index_assimp_2].normal.z += get_normal_vert(pMesh, lVertexCount + 2).z;

		lVertexCount += 3;
		/*
		int vertex_index_assimp = index_buffer[lVertexCount];
		for (int lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex)
		{
			const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
			//获取当前对应的assimp顶点
			int vertex_index_assimp = index_buffer[lVertexCount];
			now_frame_data.point_data[vertex_index_assimp].position.x = static_cast<float>(pVertices[lControlPointIndex][0]);
			now_frame_data.point_data[vertex_index_assimp].position.y = static_cast<float>(pVertices[lControlPointIndex][1]);
			now_frame_data.point_data[vertex_index_assimp].position.z = -static_cast<float>(pVertices[lControlPointIndex][2]);
			++lVertexCount;
		}*/
	}
	anim_data_list.push_back(now_frame_data);
	int a = 0;
}

void mesh_animation_FBX::compute_normal()
{
	FbxGeometryConverter lGeometryConverter(lSdkManager);
	int triangle_num = point_index_num / 3;

	XMFLOAT3 *positions = new XMFLOAT3[point_num];
	XMFLOAT3 *normals = new XMFLOAT3[point_num];
	UINT *index_test = new UINT[point_index_num];
	for (int i = 0; i < triangle_num; ++i) 
	{
		index_test[3 * i + 0] = index_buffer[3 * i + 0];
		index_test[3 * i + 1] = index_buffer[3 * i + 1];
		index_test[3 * i + 2] = index_buffer[3 * i + 2];
	}
	for (auto now_frame_data = anim_data_list.begin(); now_frame_data != anim_data_list.end(); ++now_frame_data)
	{
		
		for (int i = 0; i < now_frame_data._Ptr->point_num; ++i)
		{
			positions[i] = now_frame_data._Ptr->point_data[i].position;
		}
		ComputeNormals(index_buffer, triangle_num, positions, point_num, CNORM_WEIGHT_BY_AREA, normals);
		
		XMFLOAT3 *new_normal = new XMFLOAT3[now_frame_data._Ptr->point_num];
		for (int i = 0; i < now_frame_data._Ptr->point_num; ++i)
		{
			now_frame_data._Ptr->point_data[i].normal = normals[i];
		}
		/*
		for (int i = 0; i < triangle_num; ++i)
		{
			
			//求三角面的三个点
			int index_triangle_0 = index_buffer[i * 3 + 0];
			int index_triangle_1 = index_buffer[i * 3 + 1];
			int index_triangle_2 = index_buffer[i * 3 + 2];
			
			XMFLOAT3 point_triangle_0 = now_frame_data._Ptr->point_data[index_triangle_0].position;
			XMFLOAT3 point_triangle_1 = now_frame_data._Ptr->point_data[index_triangle_1].position;
			XMFLOAT3 point_triangle_2 = now_frame_data._Ptr->point_data[index_triangle_2].position;
			
			//求两个切向量
			XMFLOAT3 vector_u, vector_v, vec_cross;
			vector_u.x = point_triangle_1.x - point_triangle_0.x;
			vector_u.y = point_triangle_1.y - point_triangle_0.y;
			vector_u.z = point_triangle_1.z - point_triangle_0.z;
			XMStoreFloat3(&vector_u,XMVector3Normalize(XMLoadFloat3(&vector_u)));

			vector_v.x = point_triangle_2.x - point_triangle_0.x;
			vector_v.y = point_triangle_2.y - point_triangle_0.y;
			vector_v.z = point_triangle_2.z - point_triangle_0.z;
			XMStoreFloat3(&vector_v, XMVector3Normalize(XMLoadFloat3(&vector_v)));
			//求叉积
			auto cross_vec_rec = XMVector3Cross(XMLoadFloat3(&vector_u), XMLoadFloat3(&vector_v));
			auto cross_vec_normalize = XMVector3Normalize(cross_vec_rec);
			XMStoreFloat3(&vec_cross, cross_vec_normalize);
			
			//合并至法向量
			now_frame_data._Ptr->point_data[index_triangle_0].normal.x += vec_cross.x;
			now_frame_data._Ptr->point_data[index_triangle_0].normal.y += vec_cross.y;
			now_frame_data._Ptr->point_data[index_triangle_0].normal.z += vec_cross.z;

			now_frame_data._Ptr->point_data[index_triangle_1].normal.x += vec_cross.x;
			now_frame_data._Ptr->point_data[index_triangle_1].normal.y += vec_cross.y;
			now_frame_data._Ptr->point_data[index_triangle_1].normal.z += vec_cross.z;

			now_frame_data._Ptr->point_data[index_triangle_2].normal.x += vec_cross.x;
			now_frame_data._Ptr->point_data[index_triangle_2].normal.y += vec_cross.y;
			now_frame_data._Ptr->point_data[index_triangle_2].normal.z += vec_cross.z;
			
			//求面法线
			XMFLOAT3 face_normal;
			face_normal.x += now_frame_data._Ptr->point_data[index_triangle_0].normal.x;
			face_normal.y += now_frame_data._Ptr->point_data[index_triangle_0].normal.y;
			face_normal.z += now_frame_data._Ptr->point_data[index_triangle_0].normal.z;

			face_normal.x += now_frame_data._Ptr->point_data[index_triangle_1].normal.x;
			face_normal.y += now_frame_data._Ptr->point_data[index_triangle_1].normal.y;
			face_normal.z += now_frame_data._Ptr->point_data[index_triangle_1].normal.z;

			face_normal.x += now_frame_data._Ptr->point_data[index_triangle_2].normal.x;
			face_normal.y += now_frame_data._Ptr->point_data[index_triangle_2].normal.y;
			face_normal.z += now_frame_data._Ptr->point_data[index_triangle_2].normal.z;

			XMStoreFloat3(&face_normal, XMVector3Normalize(XMLoadFloat3(&face_normal)));

			//还原点法线
			new_normal[index_triangle_0].x += face_normal.x;
			new_normal[index_triangle_0].y += face_normal.y;
			new_normal[index_triangle_0].z += face_normal.z;

			new_normal[index_triangle_1].x += face_normal.x;
			new_normal[index_triangle_1].y += face_normal.y;
			new_normal[index_triangle_1].z += face_normal.z;

			new_normal[index_triangle_2].x += face_normal.x;
			new_normal[index_triangle_2].y += face_normal.y;
			new_normal[index_triangle_2].z += face_normal.z;

		}
		
		for (int i = 0; i <  now_frame_data._Ptr->point_num; ++i)
		{
			//法向量归一化
			//XMFLOAT3 vec_normal = now_frame_data._Ptr->point_data[i].normal;
			XMFLOAT3 vec_normal = new_normal[i];
			//XMFLOAT3 vec_normal = normals[i];
			//XMFLOAT3 vec_normal = now_frame_data._Ptr->point_data[i].normal;
			XMFLOAT3 vec_normal_normalize;
			XMStoreFloat3(&vec_normal_normalize, XMVector3Normalize(XMLoadFloat3(&vec_normal)));
			now_frame_data._Ptr->point_data[i].normal = vec_normal_normalize;
		}
		*/
		int a = 0;
	}
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

	for (int i = 0; i < 100; ++i)
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
		mesh_need[i].point_buffer = new mesh_model<point_skincommon>(point_need, index_need, paiMesh->mNumVertices, paiMesh->mNumFaces * 3, if_adj);
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
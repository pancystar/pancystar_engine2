#pragma once
#include"pancystar_engine_basic.h"
#include"pancy_d3d11_basic.h"
#include"geometry.h"
#include"shader_pancy.h"
#include <assimp/Importer.hpp>      // 导入器在该头文件中定义
#include <assimp/scene.h>           // 读取到的模型数据都放在scene中
#include <assimp/postprocess.h>     // 该头文件中包含后处理的标志位定义
#include <assimp/matrix4x4.h>
#include <assimp/matrix3x3.h>


struct material_list
{
	char                       texture_diffuse[512];       //漫反射纹理地址
	char                       texture_normal[512];        //法线贴图纹理地址
	char                       texture_specular[512];      //法线贴图纹理地址
	ID3D11ShaderResourceView   *tex_diffuse_resource;      //漫反射纹理
	ID3D11ShaderResourceView   *texture_normal_resource;   //法线贴图纹理
	ID3D11ShaderResourceView   *texture_specular_resource; //高光贴图纹理
	material_list()
	{
		texture_diffuse[0] = '\0';
		texture_normal[0] = '\0';
		texture_specular[0] = '\0';
		tex_diffuse_resource = NULL;
		texture_normal_resource = NULL;
		texture_specular_resource = NULL;
	}
};
struct meshview_list
{
	Geometry_basic *point_buffer;
	int material_use;
	meshview_list()
	{
		point_buffer = NULL;
		material_use = 0;
	}
};
class assimp_basic
{
protected:
	meshview_list *mesh_need;        //网格表
	ID3DX11EffectTechnique *teque_pancy;       //绘制路径
	std::string filename;        //模型文件名
	char rec_texpath[128];       //纹理路径
	Assimp::Importer importer;   //模型导入器
	const aiScene *model_need;   //模型存储类
	material_list *matlist_need; //材质表
	int material_optimization;
	int mesh_optimization;
	std::vector<std::string> meshpart_name;
public:
	assimp_basic(const char* filename, const char* texture_path);
	engine_basic::engine_fail_reason model_create(bool if_adj,int alpha_partnum, int* alpha_part);
	int get_meshnum();
	int get_texnum() { return material_optimization; };
	virtual void get_texture(material_list *texture_need, int i);
	virtual void get_texture_byindex(material_list *texture_need, int index);
	bool check_if_anim();
	virtual void release();
	virtual void draw_part(int i);
	std::string get_mesh_name_bypart(int mesh_id);
	HRESULT get_technique(ID3DX11EffectTechnique *teque_need);
protected:
	virtual engine_basic::engine_fail_reason init_mesh(bool if_adj) = 0;
	engine_basic::engine_fail_reason init_texture();
	void remove_texture_path(char rec[]);
	void change_texturedesc_2dds(char rec[]);
};

template<typename T>
class model_reader_assimp : public assimp_basic
{
protected:
	T *point_pack_list;
	UINT *index_pack_list;
	int vertex_final_num;
	int index_pack_num;
public:
	model_reader_assimp(const char* filename, const char* texture_path);
	void get_model_pack_num(int &vertex_num,int &index_num);
	void get_model_pack_data(T *point_data, UINT *index_data);
protected:
	virtual engine_basic::engine_fail_reason init_mesh(bool if_adj);
};
template<typename T>
model_reader_assimp<T>::model_reader_assimp(const char* pFile, const char *texture_path) : assimp_basic(pFile, texture_path)
{
	point_pack_list = NULL;
	index_pack_list = NULL;
	vertex_final_num = 0;
	index_pack_num = 0;
}
template<typename T>
engine_basic::engine_fail_reason model_reader_assimp<T>::init_mesh(bool if_adj)
{

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
				count = i+1;
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
	point_pack_list = (T*)malloc(vertex_final_num * sizeof(T));
	index_pack_list = (unsigned int*)malloc(index_pack_num * sizeof(unsigned int));
	int count_point_pack = 0;
	int count_index_pack = 0;
	T *point_need;
	unsigned int *index_need;
	//创建网格记录表
	mesh_need = new meshview_list[model_need->mNumMeshes];
	mesh_optimization = model_need->mNumMeshes;
	int now_index_start = count_point_pack;
	for (int i = 0; i < model_need->mNumMeshes; i++)
	{
		//获取模型的第i个模块
		const aiMesh* paiMesh = model_need->mMeshes[i];
		//获取模型的材质编号
		mesh_need[i].material_use = paiMesh->mMaterialIndex;
		point_need = (T*)malloc(paiMesh->mNumVertices * sizeof(T));
		index_need = (unsigned int*)malloc(paiMesh->mNumFaces * 3 * sizeof(unsigned int));
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
			point_pack_list[count_point_pack] = point_need[j];
			point_pack_list[count_point_pack].tex_id.x = i;
			count_point_pack += 1;
		}
		//索引缓存区
		int count_index = 0;
		for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
		{
			if (paiMesh->mFaces[j].mNumIndices == 3)
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
				engine_basic::engine_fail_reason fail_message("model" + filename + "find no triangle face");
				return fail_message;
			}
		}
		//模型的第i个模块的顶点及索引信息
		mesh_need[i].point_buffer = new mesh_model<T>(point_need, index_need,paiMesh->mNumVertices, paiMesh->mNumFaces * 3,if_adj);
		//根据内存信息创建显存区
		engine_basic::engine_fail_reason check_fail = mesh_need[i].point_buffer->create_object();
		if (!check_fail.check_if_failed())
		{
			return check_fail;
		}
		now_index_start = count_point_pack;
		//释放内存
		free(point_need);
		point_need = NULL;
		free(index_need);
		index_need = NULL;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

template<typename T>
void model_reader_assimp<T>::get_model_pack_num(int &vertex_num, int &index_num)
{
	vertex_num = vertex_final_num;
	index_num = index_pack_num;
}
template<typename T>
void model_reader_assimp<T>::get_model_pack_data(T *point_data, UINT *index_data)
{
	for (int i = 0; i < vertex_final_num; ++i) 
	{
		point_data[i] = point_pack_list[i];
	}
	for (int i = 0; i < index_pack_num; ++i)
	{
		index_data[i] = index_pack_list[i];
	}
}


struct skin_tree
{
	char bone_ID[128];
	int bone_number;
	XMFLOAT4X4 basic_matrix;
	XMFLOAT4X4 animation_matrix;
	XMFLOAT4X4 now_matrix;
	skin_tree *brother;
	skin_tree *son;
	skin_tree()
	{
		bone_ID[0] = '\0';
		bone_number = -1;
		brother = NULL;
		son = NULL;
		XMStoreFloat4x4(&basic_matrix, XMMatrixIdentity());
		XMStoreFloat4x4(&animation_matrix, XMMatrixIdentity());
		XMStoreFloat4x4(&now_matrix, XMMatrixIdentity());
	}
};
//变换向量
struct vector_animation
{
	float time;               //帧时间
	float main_key[3];        //帧数据
};
//变换四元数
struct quaternion_animation
{
	float time;               //帧时间
	float main_key[4];        //帧数据
};
//变换矩阵
struct matrix_animation
{
	float time;               //帧时间
	float main_key[16];       //帧数据
};
struct animation_data
{
	char bone_name[128];                                //本次变换数据对应的骨骼名称
	skin_tree *bone_point;                              //本次变换数据对应的骨骼的指针

	DWORD number_translation;                           //平移变换的数量
	vector_animation *translation_key;                  //各个平移变换数据

	DWORD number_scaling;                               //放缩变换的数量
	vector_animation *scaling_key;                      //各个放缩变换数据

	DWORD number_rotation;                              //旋转变换的数量
	quaternion_animation *rotation_key;                 //各个旋转变换的数据

	DWORD number_transform;                             //混合变换的数量
	matrix_animation *transform_key;                    //各个混合变换的数据	

	struct animation_data *next;                        //下一个变换数据
};
struct animation_set
{
	char  animation_name[128];                          //该动画的名字
	float animation_length;                             //动画的长度
	DWORD number_animation;                             //动画包含的变换数量
	animation_data *head_animition;                     //该动画的数据
	animation_set *next;                                //指向下一个动画的指针
};
class model_reader_skin :public model_reader_assimp<point_skincommon> 
{
	skin_tree *root_skin;
	animation_set *first_animation;
	float time_all;
	int bone_num;
	XMFLOAT4X4 bone_matrix_array[100];
	XMFLOAT4X4 offset_matrix_array[100];
	XMFLOAT4X4 final_matrix_array[100];
	int tree_node_num[100][100];
	int hand_matrix_number;
public:
	model_reader_skin(const char* filename, const char* texture_path);
	void update_root(skin_tree *root, XMFLOAT4X4 matrix_parent);
	void update_mesh_offset(int i);
	void update_mesh_offset();
	void update_animation(float delta_time);
	void specify_animation_time(float animation_time);
	XMFLOAT4X4* get_bone_matrix(int i, int &num_bone);
	XMFLOAT4X4* get_bone_matrix();
	XMFLOAT4X4 get_hand_matrix();
	XMFLOAT4X4* get_offset_mat() { return offset_matrix_array; };
	skin_tree* get_bone_tree() { return root_skin; };
	animation_set* get_animation_data() { return first_animation; };
	float get_animation_length() { return first_animation->animation_length; };
	int get_bone_num() { return bone_num; };
	void release_all();
private:
	engine_basic::engine_fail_reason init_mesh(bool if_adj);
	aiNode *find_skinroot(aiNode *now_node, char root_name[]);
	HRESULT build_skintree(aiNode *now_node, skin_tree *now_root);
	HRESULT build_animation_list();
	bool check_ifsame(char a[], char b[]);
	void set_matrix(XMFLOAT4X4 &out, aiMatrix4x4 *in);
	skin_tree* find_tree(skin_tree* p, char name[]);
	skin_tree* find_tree(skin_tree* p, int num);
	void free_tree(skin_tree *now);
	void update_anim_data(animation_data *now);
	void find_anim_sted(int &st, int &ed, quaternion_animation *input, int num_animation);
	void find_anim_sted(int &st, int &ed, vector_animation *input, int num_animation);
	void Interpolate(quaternion_animation& pOut, quaternion_animation pStart, quaternion_animation pEnd, float pFactor);
	void Interpolate(vector_animation& pOut, vector_animation pStart, vector_animation pEnd, float pFactor);
	void Get_quatMatrix(XMFLOAT4X4 &resMatrix, quaternion_animation& pOut);
	int find_min(float x1, float x2, float x3, float x4);
};
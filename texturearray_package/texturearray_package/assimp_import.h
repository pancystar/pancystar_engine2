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
	char                       texture_diffuse[128];       //漫反射纹理地址
	char                       texture_normal[128];        //法线贴图纹理地址
	char                       texture_specular[128];      //法线贴图纹理地址
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
public:
	assimp_basic(char* filename, char* texture_path);
	engine_basic::engine_fail_reason model_create(bool if_adj,int alpha_partnum, int* alpha_part);
	int get_meshnum();
	int get_texnum() { return material_optimization; };
	virtual void get_texture(material_list *texture_need, int i);
	virtual void get_texture_byindex(material_list *texture_need, int index);

	virtual void release();
	virtual void draw_part(int i);
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
	T *point_pack_list;
	UINT *index_pack_list;
	int vertex_final_num;
	int index_pack_num;
public:
	model_reader_assimp(char* filename, char* texture_path);
	void get_model_pack_num(int &vertex_num,int &index_num);
	void get_model_pack_data(T *point_data, UINT *index_data);
protected:
	virtual engine_basic::engine_fail_reason init_mesh(bool if_adj);
};
template<typename T>
model_reader_assimp<T>::model_reader_assimp(char* pFile, char *texture_path) : assimp_basic(pFile, texture_path)
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
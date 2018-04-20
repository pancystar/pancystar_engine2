#pragma once
#include<windows.h>
#include<string.h>
#include<iostream>
#include<D3D11.h>
#include<assert.h>
#include<directxmath.h>
#include<d3dx11effect.h>
#include<DDSTextureLoader.h>
#include<DirectXMesh.h>
#include"pancystar_engine_basic.h"
#include"pancy_d3d11_basic.h"
using namespace DirectX;
struct fire_point
{
	XMFLOAT3 position;
	XMFLOAT3 speed;
	XMFLOAT2 Size;
	float Age;
	unsigned int Type;
};
struct sakura_point
{
	XMFLOAT3 position;
	XMFLOAT3 speed;
	XMFLOAT2 Size;
	float Age;
	unsigned int Type;
	unsigned int texnum;
};
struct point_common
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMUINT4  tex_id;
	XMFLOAT4 tex;
	XMFLOAT4 tex2;
};
struct point_output
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT3 tex_UVI;//UV&ID
	XMFLOAT4 tex_range;
};
struct point_skincommon
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMUINT4  tex_id;
	XMFLOAT4 tex;
	XMFLOAT4 tex2;
	XMUINT4  bone_id;
	XMFLOAT4 bone_weight;
};
struct point_skinoutput
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT3 tex_UVI;//UV&ID
	XMFLOAT4 tex_range;
	XMUINT4  bone_id;
	XMFLOAT4 bone_weight;
};
struct point_terrain
{
	XMFLOAT3 position;
	XMUINT4  tex_id;
	XMFLOAT2 tex_height;
	XMFLOAT2 tex_diffuse;
};
struct point_2D
{
	XMFLOAT3 position;
	XMFLOAT2 texcoord;
};
//�����������
class Geometry_basic
{
protected:
	ID3D11Buffer            *vertex_need;       //���㻺����������
	ID3D11Buffer	        *index_need;        //��������������
	ID3D11Buffer	        *indexadj_need;     //�ڽ���������������
	ID3DX11EffectTechnique  *teque_pancy;       //����·��
	int                     all_vertex;         //������Ķ������
	int                     all_index;          //���������������
	bool                    if_init_adj;
	UINT stride_vertex;
public:
	Geometry_basic(bool if_adj);
	virtual engine_basic::engine_fail_reason create_object() = 0;
	void get_point_num(int &vertex_number, int &index_number) { vertex_number = all_vertex; index_number = all_index; };
	void get_teque(ID3DX11EffectTechnique *teque_need);
	virtual void show_mesh();
	virtual void show_mesh_adj();
	virtual void show_mesh_instance(int copy_num);
	void release();
};
//�����崴������
template<typename T>
class Geometry : public Geometry_basic
{
public:
	Geometry(bool if_adj);
	//ֱ�Ӵ��������壬���������ú���init_point()���ɡ�
	virtual engine_basic::engine_fail_reason create_object();
	//��ȡ������������
	virtual engine_basic::engine_fail_reason get_bufferdata(T *vertex, UINT *index);
protected:
	//�����ڴ���Ķ�����Ϣ���Դ��￪��һ�ݿ���
	virtual engine_basic::engine_fail_reason init_point(T *vertex, UINT *index);
	//�������ɺ�����
	virtual engine_basic::engine_fail_reason find_point(T *vertex, UINT *index, int &num_vertex, int &num_index) = 0;
	//ת��������Դʹ��GPU��Դ����map��CPU
	ID3D11Buffer* CreateAndCopyToDebugBuf(ID3D11Buffer* pGBuffer);
};
template<typename T>
Geometry<T>::Geometry(bool if_adj) :Geometry_basic(if_adj)
{
	vertex_need = NULL;
	index_need = NULL;
	indexadj_need = NULL;
	all_vertex = 0;
	all_index = 0;
	stride_vertex = sizeof(T);
}
template<typename T>
engine_basic::engine_fail_reason Geometry<T>::init_point(T *vertex, UINT *index)
{
	D3D11_BUFFER_DESC point_buffer;
	point_buffer.Usage = D3D11_USAGE_IMMUTABLE;            //������gpuֻ����
	point_buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;         //��������Ϊ���㻺��
	point_buffer.ByteWidth = all_vertex * sizeof(T); //���㻺��Ĵ�С
	point_buffer.CPUAccessFlags = 0;
	point_buffer.MiscFlags = 0;
	point_buffer.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA resource_vertex;
	resource_vertex.pSysMem = vertex;//ָ���������ݵĵ�ַ
									 //�������㻺����
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&point_buffer, &resource_vertex, &vertex_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason fail_message(hr, "create vertex buffer error");
		return fail_message;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~��������������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//��������ʽ
	D3D11_BUFFER_DESC index_buffer;
	index_buffer.ByteWidth = all_index*sizeof(UINT);
	index_buffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer.Usage = D3D11_USAGE_IMMUTABLE;
	index_buffer.CPUAccessFlags = 0;
	index_buffer.MiscFlags = 0;
	index_buffer.StructureByteStride = 0;
	//Ȼ���������
	D3D11_SUBRESOURCE_DATA resource_index = { 0 };
	resource_index.pSysMem = index;
	//�������������ݴ�����������
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&index_buffer, &resource_index, &index_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason fail_message(hr, "create index buffer error");
		return fail_message;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~�����ڽ�����������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if (if_init_adj == true)
	{
		//��ȡ�ڽ�����
		XMFLOAT3 *positions = (XMFLOAT3*)malloc(all_vertex*sizeof(XMFLOAT3));
		for (int i = 0; i < all_vertex; ++i)
		{
			positions[i] = vertex[i].position;
		}
		UINT *rec_adj, *rec_pointrep, *adj_index;
		rec_adj = (UINT*)malloc(all_index * sizeof(UINT));
		rec_pointrep = (UINT*)malloc(all_vertex * sizeof(UINT));
		adj_index = (UINT*)malloc(2 * all_index * sizeof(UINT));
		GenerateAdjacencyAndPointReps(index, all_index / 3, positions, all_vertex, 0.01f, rec_pointrep, rec_adj);
		GenerateGSAdjacency(index, all_index / 3, rec_pointrep, rec_adj, all_vertex, adj_index);
		//������
		D3D11_SUBRESOURCE_DATA resource_index_adj = { 0 };
		resource_index_adj.pSysMem = adj_index;
		//����������
		index_buffer.ByteWidth = 2 * all_index*sizeof(UINT);
		hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&index_buffer, &resource_index_adj, &indexadj_need);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason fail_message(hr, "create adj index buffer error");
			return fail_message;
		}
		free(positions);
		free(rec_pointrep);
		free(rec_adj);
		free(adj_index);
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
template<typename T>
engine_basic::engine_fail_reason Geometry<T>::create_object()
{
	if (all_vertex == 0)
	{
		engine_basic::engine_fail_reason fail_message("vertex number is zero");
		return fail_message;
	}
	T *vertex_use = new T[all_vertex + 100];
	UINT   *index_use = new UINT[all_index + 100];  //��������
	engine_basic::engine_fail_reason check_faile;
	check_faile = find_point(vertex_use, index_use, all_vertex, all_index);
	if (!check_faile.check_if_failed())
	{
		return check_faile;
	}
	check_faile = init_point(vertex_use, index_use);
	if (!check_faile.check_if_failed())
	{
		return check_faile;
	}
	delete[] vertex_use;
	delete[] index_use;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
template<typename T>
engine_basic::engine_fail_reason Geometry<T>::get_bufferdata(T *vertex, UINT *index)
{
	if (vertex_need == NULL || index_need == NULL)
	{
		engine_basic::engine_fail_reason fail_message("get vertex/index buffer data empty");
		return fail_message;
	}
	ID3D11Buffer* vertex_rec = NULL;
	ID3D11Buffer* index_rec = NULL;
	vertex_rec = CreateAndCopyToDebugBuf(vertex_need);
	index_rec = CreateAndCopyToDebugBuf(index_need);

	D3D11_MAPPED_SUBRESOURCE vertex_resource;
	D3D11_MAPPED_SUBRESOURCE index_resource;
	HRESULT hr;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Map(vertex_rec, 0, D3D11_MAP_READ, 0, &vertex_resource);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason fail_message(hr, "get vertex buffer map error");
		return fail_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Map(index_rec, 0, D3D11_MAP_READ, 0, &index_resource);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason fail_message(hr, "get index buffer map error");
		return fail_message;
	}
	memcpy(static_cast<void*>(vertex), vertex_resource.pData, all_vertex * sizeof(T));
	memcpy(static_cast<void*>(index), index_resource.pData, all_index * sizeof(UINT));
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Unmap(vertex_rec, 0);
	vertex_rec->Release();
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Unmap(index_rec, 0);
	index_rec->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
template<typename T>
ID3D11Buffer* Geometry<T>::CreateAndCopyToDebugBuf(ID3D11Buffer* pGBuffer)
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	pGBuffer->GetDesc(&bufferDesc);
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.BindFlags = 0;
	bufferDesc.MiscFlags = 0;
	ID3D11Buffer* pDebugBuffer = NULL;
	if (SUCCEEDED(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&bufferDesc, NULL, &pDebugBuffer)))
	{
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->CopyResource(pDebugBuffer, pGBuffer);
	}
	return pDebugBuffer;
}



class mesh_cube : public Geometry<point_common>
{
public:
	mesh_cube(bool if_adj);
private:
	engine_basic::engine_fail_reason find_point(point_common *vertex, UINT *index, int &num_vertex, int &num_index);
};
class mesh_square : public Geometry<point_2D>
{
public:
	mesh_square(bool if_adj);
private:
	engine_basic::engine_fail_reason find_point(point_2D *vertex, UINT *index, int &num_vertex, int &num_index);
};
class mesh_ball : public Geometry<point_common>
{
	int circle_num;
	int vertex_percircle;
public:
	mesh_ball(bool if_adj,int circle_num_need, int vertex_percircle_need);
private:
	engine_basic::engine_fail_reason find_point(point_common *vertex, UINT *index, int &num_vertex, int &num_index);
};
template<typename T>
class mesh_model : public Geometry<T> 
{
	T* point_data;
	UINT* index_data;
public:
	mesh_model(T* point_input, UINT* index_input,int vertex_num_need,int index_num_need, bool if_adj);
private:
	engine_basic::engine_fail_reason find_point(T *vertex, UINT *index, int &num_vertex, int &num_index);
};
template<typename T>
mesh_model<T>::mesh_model(T* point_input, UINT* index_input, int vertex_num_need, int index_num_need, bool if_adj) : Geometry<T>(if_adj)
{
	//point_data = point_input;
	//index_data = index_input;
	point_data = new T[vertex_num_need];
	index_data = new UINT[index_num_need];
	for (int i = 0; i < vertex_num_need; ++i)
	{
		point_data[i] = point_input[i];
	}
	for (int i = 0; i < index_num_need; ++i)
	{
		index_data[i] = index_input[i];
	}
	all_vertex = vertex_num_need;
	all_index = index_num_need;
}
template<typename T>
engine_basic::engine_fail_reason mesh_model<T>::find_point(T *vertex, UINT *index, int &num_vertex, int &num_index)
{
	if (!point_data) 
	{
		engine_basic::engine_fail_reason check_error("vertex pointer is NULL");
		return check_error;
	}
	if (!index_data)
	{
		engine_basic::engine_fail_reason check_error("index pointer is NULL");
		return check_error;
	}
	for (int i = 0; i < all_vertex; ++i) 
	{
		vertex[i] = point_data[i];
	}
	for (int i = 0; i < all_index; ++i)
	{
		index[i] = index_data[i];
	}
	num_vertex = all_vertex;
	num_index = all_index;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
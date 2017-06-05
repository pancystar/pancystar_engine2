#include<math.h>
#include"geometry.h"
//几何体访问父类
Geometry_basic::Geometry_basic(bool if_adj)
{
	vertex_need = NULL;
	index_need = NULL;
	indexadj_need = NULL;
	all_vertex = 0;
	all_index = 0;
	if_init_adj = if_adj;
}
void Geometry_basic::get_teque(ID3DX11EffectTechnique *teque_need)
{
	teque_pancy = teque_need;
}
void Geometry_basic::show_mesh()
{
	UINT offset_need = 0;                       //顶点结构的首地址偏移
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetVertexBuffers(0, 1, &vertex_need, &stride_vertex, &offset_need);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetIndexBuffer(index_need, DXGI_FORMAT_R32_UINT, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//选定绘制路径
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_pancy->GetDesc(&techDesc);
	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		teque_pancy->GetPassByIndex(i)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->DrawIndexed(all_index, 0, 0);
	}
}
void Geometry_basic::show_mesh_adj()
{
	UINT offset_need = 0;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetVertexBuffers(0, 1, &vertex_need, &stride_vertex, &offset_need);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetIndexBuffer(indexadj_need, DXGI_FORMAT_R32_UINT, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ);
	//选定绘制路径
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_pancy->GetDesc(&techDesc);
	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		teque_pancy->GetPassByIndex(i)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->DrawIndexed(all_index * 2, 0, 0);
	}
}
void Geometry_basic::show_mesh_instance(int copy_num)
{
	UINT offset_need = 0;                       //顶点结构的首地址偏移
												//顶点缓存，索引缓存，绘图格式
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetVertexBuffers(0, 1, &vertex_need, &stride_vertex, &offset_need);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetIndexBuffer(index_need, DXGI_FORMAT_R32_UINT, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//选定绘制路径
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_pancy->GetDesc(&techDesc);
	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		teque_pancy->GetPassByIndex(i)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->DrawIndexedInstanced(all_index, copy_num, 0, 0, 0);
	}
}
void Geometry_basic::release()
{
	if (vertex_need != NULL)
	{
		vertex_need->Release();
	}
	if (index_need != NULL)
	{
		index_need->Release();
	}
	if (indexadj_need != NULL)
	{
		indexadj_need->Release();
	}
}

//立方体
mesh_cube::mesh_cube(bool if_adj):Geometry<point_common>(if_adj)
{
	all_vertex = 24;
	all_index = 24 * 6;
}
engine_basic::engine_fail_reason mesh_cube::find_point(point_common *vertex,UINT *index,int &num_vertex,int &num_index)
{
	point_common square_test[] =
	{
		{ XMFLOAT3(-1.0, -1.0, -1.0), XMFLOAT3(0.0, 0.0, -1.0), XMFLOAT3(1.0, 0.0, 0.0),XMUINT4(0,0,0,0) , XMFLOAT4(0.0, 1.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0, 1.0, -1.0), XMFLOAT3(0.0, 0.0, -1.0), XMFLOAT3(1.0, 0.0, 0.0),XMUINT4(0,0,0,0), XMFLOAT4(0.0, 0.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0, 1.0, -1.0), XMFLOAT3(0.0, 0.0, -1.0), XMFLOAT3(1.0, 0.0, 0.0), XMUINT4(0,0,0,0),XMFLOAT4(1.0, 0.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0, -1.0, -1.0), XMFLOAT3(0.0, 0.0, -1.0), XMFLOAT3(1.0, 0.0, 0.0),XMUINT4(0,0,0,0), XMFLOAT4(1.0, 1.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0, -1.0, 1.0), XMFLOAT3(-1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, -1.0), XMUINT4(0,0,0,0),XMFLOAT4(0.0, 1.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0, 1.0, 1.0), XMFLOAT3(-1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, -1.0), XMUINT4(0,0,0,0),XMFLOAT4(0.0, 0.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0, 1.0, -1.0), XMFLOAT3(-1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, -1.0),XMUINT4(0,0,0,0), XMFLOAT4(1.0, 0.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0, -1.0, -1.0), XMFLOAT3(-1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, -1.0),XMUINT4(0,0,0,0), XMFLOAT4(1.0, 1.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0, -1.0, 1.0), XMFLOAT3(0.0, 0.0, 1.0), XMFLOAT3(-1.0, 0.0, 0.0),XMUINT4(0,0,0,0), XMFLOAT4(0.0, 1.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0, 1.0, 1.0), XMFLOAT3(0.0, 0.0, 1.0), XMFLOAT3(-1.0, 0.0, 0.0),XMUINT4(0,0,0,0), XMFLOAT4(0.0, 0.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0, 1.0, 1.0), XMFLOAT3(0.0, 0.0, 1.0), XMFLOAT3(-1.0, 0.0, 0.0),XMUINT4(0,0,0,0), XMFLOAT4(1.0, 0.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0, -1.0, 1.0), XMFLOAT3(0.0, 0.0, 1.0), XMFLOAT3(-1.0, 0.0, 0.0), XMUINT4(0,0,0,0),XMFLOAT4(1.0, 1.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0, -1.0, -1.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, 1.0),XMUINT4(0,0,0,0), XMFLOAT4(0.0, 1.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0, 1.0, -1.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, 1.0),XMUINT4(0,0,0,0), XMFLOAT4(0.0, 0.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0, 1.0, 1.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, 1.0), XMUINT4(0,0,0,0),XMFLOAT4(1.0, 0.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0, -1.0, 1.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, 1.0),XMUINT4(0,0,0,0), XMFLOAT4(1.0, 1.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0, 1.0, -1.0), XMFLOAT3(0.0, 1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0),XMUINT4(0,0,0,0), XMFLOAT4(0.0, 1.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0, 1.0, 1.0), XMFLOAT3(0.0, 1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0),XMUINT4(0,0,0,0), XMFLOAT4(0.0, 0.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0, 1.0, 1.0), XMFLOAT3(0.0, 1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0),XMUINT4(0,0,0,0), XMFLOAT4(1.0, 0.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0, 1.0, -1.0), XMFLOAT3(0.0, 1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0),XMUINT4(0,0,0,0), XMFLOAT4(1.0, 1.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0, -1.0, 1.0), XMFLOAT3(0.0, -1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0),XMUINT4(0,0,0,0), XMFLOAT4(0.0, 1.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0, -1.0, -1.0), XMFLOAT3(0.0, -1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0),XMUINT4(0,0,0,0), XMFLOAT4(0.0, 0.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0, -1.0, -1.0), XMFLOAT3(0.0, -1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0),XMUINT4(0,0,0,0), XMFLOAT4(1.0, 0.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0, -1.0, 1.0), XMFLOAT3(0.0, -1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0),XMUINT4(0,0,0,0), XMFLOAT4(1.0, 1.0,0,0),XMFLOAT4(0.0f,0.0f,0.0f,0.0f) }
	};
	//创建索引数组。
	num_vertex = sizeof(square_test) / sizeof(point_common);
	for (int i = 0; i < num_vertex; ++i)
	{
		vertex[i] = square_test[i];
	}
	UINT indices[] = { 0,1,2, 0,2,3, 4,5,6, 4,6,7, 8,9,10, 8,10,11, 12,13,14, 12,14,15, 16,17,18, 16,18,19, 20,21,22, 20,22,23 };
	num_index = sizeof(indices) / sizeof(UINT);
	for (int i = 0; i < num_index; ++i)
	{
		index[i] = indices[i];
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

//平面
mesh_square::mesh_square(bool if_adj) :Geometry<point_2D>(if_adj)
{
	all_vertex = 4;
	all_index = 6;
}
engine_basic::engine_fail_reason mesh_square::find_point(point_2D *vertex, UINT *index, int &num_vertex, int &num_index)
{
	point_2D square_test[4];
	square_test[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	square_test[1].position = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	square_test[2].position = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	square_test[3].position = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	square_test[0].texcoord = XMFLOAT2(0.0f, 1.0f);
	square_test[1].texcoord = XMFLOAT2(0.0f, 0.0f);
	square_test[2].texcoord = XMFLOAT2(1.0f, 0.0f);
	square_test[3].texcoord = XMFLOAT2(1.0f, 1.0f);
	//创建索引数组。
	num_vertex = sizeof(square_test) / sizeof(point_2D);
	for (int i = 0; i < num_vertex; ++i)
	{
		vertex[i] = square_test[i];
	}
	UINT indices[] = { 0,1,2, 0,2,3};
	num_index = sizeof(indices) / sizeof(UINT);
	for (int i = 0; i < num_index; ++i)
	{
		index[i] = indices[i];
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
//AO平面
mesh_aosquare::mesh_aosquare(bool if_adj) :Geometry<point_ssao>(if_adj)
{
	all_vertex = 4;
	all_index = 6;
}
engine_basic::engine_fail_reason mesh_aosquare::find_point(point_ssao *vertex, UINT *index, int &num_vertex, int &num_index)
{
	point_ssao v[4];

	v[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	v[1].position = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	v[2].position = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	v[3].position = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	// Store far plane frustum corner indices in Normal.x slot.
	v[0].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[1].normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
	v[2].normal = XMFLOAT3(2.0f, 0.0f, 0.0f);
	v[3].normal = XMFLOAT3(3.0f, 0.0f, 0.0f);

	v[0].tex = XMFLOAT2(0.0f, 1.0f);
	v[1].tex = XMFLOAT2(0.0f, 0.0f);
	v[2].tex = XMFLOAT2(1.0f, 0.0f);
	v[3].tex = XMFLOAT2(1.0f, 1.0f);
	//创建索引数组。
	num_vertex = sizeof(v) / sizeof(point_ssao);
	for (int i = 0; i < num_vertex; ++i)
	{
		vertex[i] = v[i];
	}
	UINT indices[] = { 0,1,2, 0,2,3 };
	num_index = sizeof(indices) / sizeof(UINT);
	for (int i = 0; i < num_index; ++i)
	{
		index[i] = indices[i];
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
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
	for (int i = 0; i < 24; ++i) 
	{
		square_test[i].tex.x *= 100.0f;
		square_test[i].tex.y *= 100.0f;
	}
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

//球面
mesh_ball::mesh_ball(bool if_adj, int circle_num_need, int vertex_percircle_need) :Geometry<point_common>(if_adj)
{
	circle_num = circle_num_need;
	vertex_percircle = vertex_percircle_need;
	all_vertex = circle_num_need*vertex_percircle_need + 2;
	all_index = circle_num_need*vertex_percircle_need * 6;
}
engine_basic::engine_fail_reason mesh_ball::find_point(point_common *vertex, UINT *index, int &num_vertex, int &num_index)
{
	num_vertex = 0;
	num_index = 0;
	float radius = 1.0;
	//填充顶点的位置信息
	for (int i = 0; i < circle_num; ++i)
	{
		float phy = XM_PI * static_cast<float>(i + 1) / static_cast<float>(circle_num + 1); //球面角
		float tmpRadius = radius * sin(phy);//球面每个平面所截的圆半径
		for (int j = 0; j < vertex_percircle; ++j)
		{
			float theta = 2 * XM_PI * static_cast<float>(j) / static_cast<float>(vertex_percircle);//圆面角  

			float x = tmpRadius*cos(theta);
			float y = radius*cos(phy);
			float z = tmpRadius*sin(theta);
			//位置坐标
			vertex[num_vertex].position = XMFLOAT3(x, y, z);
			XMVECTOR N = XMVectorSet(x, y, z, 0.f);
			XMStoreFloat3(&vertex[num_vertex++].normal, XMVector3Normalize(N));
		}
	}
	//填充顶点的索引信息
	for (int i = 0; i < circle_num - 1; ++i)
	{
		for (int j = 0; j < vertex_percircle; ++j)
		{
			if (j == vertex_percircle - 1)
			{
				index[num_index++] = i * vertex_percircle + j;
				index[num_index++] = (i + 1) * vertex_percircle;
				index[num_index++] = (i + 1) * vertex_percircle + j;
				index[num_index++] = i * vertex_percircle + j;
				index[num_index++] = i * vertex_percircle;
				index[num_index++] = (i + 1) * vertex_percircle;
			}
			else
			{
				index[num_index++] = i * vertex_percircle + j;
				index[num_index++] = (i + 1) * vertex_percircle + j + 1;
				index[num_index++] = (i + 1) * vertex_percircle + j;
				index[num_index++] = i * vertex_percircle + j;
				index[num_index++] = i * vertex_percircle + j + 1;
				index[num_index++] = (i + 1) * vertex_percircle + j + 1;
			}
		}
	}
	//加上上下盖信息
	vertex[num_vertex].position = XMFLOAT3(0, radius, 0);
	vertex[num_vertex++].normal = XMFLOAT3(0, 1, 0);
	for (int j = 0; j < vertex_percircle; ++j)
	{
		if (j == vertex_percircle - 1)
		{
			index[num_index++] = num_vertex - 1;
			index[num_index++] = 0;
			index[num_index++] = j;
		}
		else
		{
			index[num_index++] = num_vertex - 1;
			index[num_index++] = j + 1;
			index[num_index++] = j;
		}
	}
	vertex[num_vertex].position = XMFLOAT3(0, -radius, 0);
	vertex[num_vertex++].normal = XMFLOAT3(0, -1, 0);
	for (int j = 0; j < vertex_percircle; ++j)
	{
		if (j == vertex_percircle - 1)
		{
			index[num_index++] = num_vertex - 1;
			index[num_index++] = (circle_num - 1) * vertex_percircle + j;
			index[num_index++] = (circle_num - 1) * vertex_percircle;
		}
		else
		{
			index[num_index++] = num_vertex - 1;
			index[num_index++] = (circle_num - 1) * vertex_percircle + j;
			index[num_index++] = (circle_num - 1) * vertex_percircle + j + 1;
		}
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
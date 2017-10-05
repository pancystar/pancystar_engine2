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
void Geometry_basic::show_mesh_pass(int pass)
{
	UINT offset_need = 0;                       //顶点结构的首地址偏移
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetVertexBuffers(0, 1, &vertex_need, &stride_vertex, &offset_need);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetIndexBuffer(index_need, DXGI_FORMAT_R32_UINT, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//选定绘制路径
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_pancy->GetDesc(&techDesc);
	teque_pancy->GetPassByIndex(pass)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->DrawIndexed(all_index, 0, 0);
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
mesh_cube::mesh_cube(bool if_adj) :Geometry<point_common>(if_adj)
{
	all_vertex = 24;
	all_index = 24 * 6;
}
engine_basic::engine_fail_reason mesh_cube::find_point(point_common *vertex, UINT *index, int &num_vertex, int &num_index)
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
	UINT indices[] = { 0,1,2, 0,2,3 };
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
//曲面细分平面
template <>   // 对point_UV 型特例化  
engine_basic::engine_fail_reason Geometry<point_UV>::init_point(point_UV *vertex, UINT *index)
{
	D3D11_BUFFER_DESC point_buffer;
	point_buffer.Usage = D3D11_USAGE_IMMUTABLE;            //顶点是gpu只读型
	point_buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;         //缓存类型为顶点缓存
	point_buffer.ByteWidth = all_vertex * sizeof(point_UV); //顶点缓存的大小
	point_buffer.CPUAccessFlags = 0;
	point_buffer.MiscFlags = 0;
	point_buffer.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA resource_vertex;
	resource_vertex.pSysMem = vertex;//指定顶点数据的地址
									 //创建顶点缓冲区
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&point_buffer, &resource_vertex, &vertex_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason fail_message(hr, "create vertex buffer error");
		return fail_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
mesh_square_tessellation::mesh_square_tessellation(bool if_adj) :Geometry(if_adj)
{
	all_vertex = 4;
	all_index = 6;
}
engine_basic::engine_fail_reason mesh_square_tessellation::find_point(point_UV *vertex, UINT *index, int &num_vertex, int &num_index)
{
	point_UV square_test[] =
	{
		{ XMFLOAT2(0.0f,0.0f) },
		{ XMFLOAT2(1.0f,0.0f) },
		{ XMFLOAT2(0.0f,1.0f) },
		{ XMFLOAT2(1.0f,1.0f) },

	};
	for (int i = 0; i < 4; ++i)
	{
		vertex[i] = square_test[i];
	}
	num_vertex = 4;
	num_index = 0;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void mesh_square_tessellation::show_mesh()
{
	UINT stride_need = sizeof(point_UV);     //顶点结构的位宽
	UINT offset_need = 0;                       //顶点结构的首地址偏移
												//顶点缓存，索引缓存，绘图格式
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetVertexBuffers(0, 1, &vertex_need, &stride_need, &offset_need);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	//选定绘制路径
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_pancy->GetDesc(&techDesc);
	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		teque_pancy->GetPassByIndex(i)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Draw(4, 0);
	}
}


mesh_multisquare_tessellation::mesh_multisquare_tessellation(bool if_adj, int divide_level_in) :Geometry(if_adj)
{
	divide_level = divide_level_in;
	all_vertex = 4 * divide_level*divide_level;
	all_index = 6;
}
engine_basic::engine_fail_reason mesh_multisquare_tessellation::find_point(point_UV *vertex, UINT *index, int &num_vertex, int &num_index)
{
	point_UV square_test[] =
	{
		{ XMFLOAT2(0.0f,0.0f) },
		{ XMFLOAT2(1.0f,0.0f) },
		{ XMFLOAT2(0.0f,1.0f) },
		{ XMFLOAT2(1.0f,1.0f) },

	};
	int now_count = 0;
	XMFLOAT2 uv_scal = XMFLOAT2(1.0f / 1, 1.0f / 1);
	for (int i = 0; i < divide_level; ++i)
	{
		for (int j = 0; j < divide_level; ++j)
		{
			XMFLOAT2 uv_offset = XMFLOAT2(uv_scal.x * i, uv_scal.y * j);
			for (int k = 0; k < 4; ++k)
			{
				XMFLOAT2 scal_vec = engine_basic::engine_mathmatic::vec2_mul(square_test[k].texcoord, uv_scal);
				vertex[now_count++].texcoord = engine_basic::engine_mathmatic::vec2_plus(scal_vec,uv_offset);
			}
		}
	}

	num_vertex = 4 * divide_level*divide_level;
	num_index = 0;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void mesh_multisquare_tessellation::show_mesh()
{
	UINT stride_need = sizeof(point_UV);     //顶点结构的位宽
	UINT offset_need = 0;                       //顶点结构的首地址偏移
												//顶点缓存，索引缓存，绘图格式
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetVertexBuffers(0, 1, &vertex_need, &stride_need, &offset_need);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	//选定绘制路径
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_pancy->GetDesc(&techDesc);
	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		teque_pancy->GetPassByIndex(i)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Draw(all_vertex, 0);
	}
}
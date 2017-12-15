#pragma once
#include"geometry.h"
#include"pancystar_engine_basic.h"
#include"pancy_d3d11_basic.h"
#include"shader_pancy.h"
#include<math.h>
using namespace std;
struct terrain_file_path
{
	string height_rawdata_name;
	string blend_texdata_name;
	string normal_texdata_name;
	string tangent_texdata_name;
	string color_albe_texdata_name[4];
	string color_norm_texdata_name[4];
};
class pancy_terrain_part
{
	//地形实际信息
	float terrain_width;                    //地形的真实宽度
	float Terrain_HeightScal;              //地形高度放大
	XMFLOAT2 terrain_offset;                //地形偏移
	int terrain_divide;                     //地形的细分级别
											//纹理信息
	float Terrain_ColorTexScal;            //地形的细节放大等级
	int TexHeight_width;                    //地形高度图的分辨率
											//地形数据
	std::vector<float> terrain_height_data; //地形高度数据
	terrain_file_path        terrain_file;
	Geometry_basic           *terrain_renderbuffer;
	ID3D11ShaderResourceView *terrain_height_tex;
	ID3D11ShaderResourceView *terrain_normal_tex;
	ID3D11ShaderResourceView *terrain_tangent_tex;
	ID3D11ShaderResourceView *terrain_blend_tex;
	ID3D11ShaderResourceView *terrain_color_albe_tex;
	ID3D11ShaderResourceView *terrain_color_norm_tex;
public:
	pancy_terrain_part(float terrain_width_in, int terrain_divide_in, float Terrain_ColorTexScal_in, float Terrain_HeightScal_in, XMFLOAT2 terrain_offset_in, terrain_file_path file_name);
	void render_terrain(XMFLOAT3 view_pos, XMFLOAT4X4 view_mat, XMFLOAT4X4 proj_mat);
	engine_basic::engine_fail_reason create();
	void release();
private:
	engine_basic::engine_fail_reason load_terrain_height();
	engine_basic::engine_fail_reason load_terrain_normal();
	engine_basic::engine_fail_reason load_terrain_tangent();
	engine_basic::engine_fail_reason load_terrain_blend();
	engine_basic::engine_fail_reason load_terrain_color();
	engine_basic::engine_fail_reason load_tex_array(string texdata_name[4], ID3D11ShaderResourceView **tex_array);
};
/*
struct terrain_part 
{
	pancy_terrain_part *now_terrain;
	pancy_terrain_part *nature_terrain[8];

};
class pancy_terrain_control 
{
	std::vector
};
*/
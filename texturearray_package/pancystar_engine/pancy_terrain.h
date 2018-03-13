#pragma once
#include"geometry.h"
#include"pancystar_engine_basic.h"
#include"pancy_d3d11_basic.h"
#include"shader_pancy.h"
#include"pancy_physx.h"
#include<math.h>
#include<queue>
using namespace std;
enum terrain_mat_quality
{
	terrain_material_low = 0,
	terrain_material_mid = 1,
	terrain_material_high = 2
};
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
	string terrain_file_name;
	std::vector<short> terrain_height_data; //地形高度数据
	terrain_file_path        terrain_file;
	Geometry_basic           *terrain_renderbuffer;
	ID3D11ShaderResourceView *terrain_height_tex;
	ID3D11ShaderResourceView *terrain_normal_tex;
	ID3D11ShaderResourceView *terrain_tangent_tex;
	ID3D11ShaderResourceView *terrain_blend_tex;
	/*
	ID3D11ShaderResourceView *terrain_color_albe_tex;
	ID3D11ShaderResourceView *terrain_color_norm_tex;
	*/
	//terrain_color_resource terrain_color_tex[4];
	string terrain_color_tex[4];
public:
	pancy_terrain_part(
		float terrain_width_in,
		int terrain_divide_in,
		float Terrain_ColorTexScal_in,
		float Terrain_HeightScal_in,
		XMFLOAT2 terrain_offset_in,
		string file_name
		);
	void render_terrain(XMFLOAT3 view_pos, XMFLOAT4X4 view_mat, XMFLOAT4X4 proj_mat,std::unordered_map<std::string, terrain_color_resource> material_list);
	void render_terrain_gbuffer(XMFLOAT3 view_pos, XMFLOAT4X4 view_mat, XMFLOAT4X4 proj_mat, std::unordered_map<std::string, terrain_color_resource> material_list);
	engine_basic::engine_fail_reason create();
	void release();
	int get_terrain_height_width() { return TexHeight_width; };
	std::vector<short> get_terrain_height_data() { return terrain_height_data; };
private:
	engine_basic::engine_fail_reason load_terrain_file();
	string find_path(string input);
	string move_string_space(string input);
	engine_basic::engine_fail_reason load_terrain_height();
	engine_basic::engine_fail_reason load_terrain_normal();
	engine_basic::engine_fail_reason load_terrain_tangent();
	engine_basic::engine_fail_reason load_terrain_blend();
	//engine_basic::engine_fail_reason load_terrain_color();
	//engine_basic::engine_fail_reason load_tex_array(string texdata_name[4], ID3D11ShaderResourceView **tex_array);
};
class terrain_part_resource
{
	int self_ID;
	bool if_loaded;         //是否已经加载
	//地形构造信息
	float terrain_width;
	int terrain_divide;
	float Terrain_ColorTexScal;
	float Terrain_HeightScal;
	XMFLOAT2 terrain_offset;
	string file_name;
	//地形结构信息
	pancy_terrain_part *now_terrain;
	int neighbour_up_ID;
	int neighbour_down_ID;
	int neighbour_left_ID;
	int neighbour_right_ID;
	int neighbour_upleft_ID;
	int neighbour_upright_ID;
	int neighbour_downleft_ID;
	int neighbour_downright_ID;
	//地形物理信息
	bool if_loaded_physics;           //是否加载了物理信息
	bool if_wakeup_physics;           //是否唤醒了物理效果
	unsigned __int64 terrain_physx_ID;//物理模型ID
public:
	terrain_part_resource(
		int self_ID_in,
		float terrain_width_in,
		int terrain_divide_in,
		float Terrain_ColorTexScal_in,
		float Terrain_HeightScal_in,
		XMFLOAT2 terrain_offset_in,
		string file_name_in
		);
	bool check_if_loaded() { return if_loaded; };
	void set_neighbour_up_ID(int ID_in) { neighbour_up_ID = ID_in; };
	void set_neighbour_down_ID(int ID_in) { neighbour_down_ID = ID_in; };
	void set_neighbour_left_ID(int ID_in) { neighbour_left_ID = ID_in; };
	void set_neighbour_right_ID(int ID_in) { neighbour_right_ID = ID_in; };
	void set_neighbour_upleft_ID(int ID_in) { neighbour_upleft_ID = ID_in; };
	void set_neighbour_upright_ID(int ID_in) { neighbour_upright_ID = ID_in; };
	void set_neighbour_downleft_ID(int ID_in) { neighbour_downleft_ID = ID_in; };
	void set_neighbour_downright_ID(int ID_in) { neighbour_downright_ID = ID_in; };

	int get_self_ID() { return self_ID; };
	int get_neighbour_up_ID() { return neighbour_up_ID; };
	int get_neighbour_down_ID() { return neighbour_down_ID; };
	int get_neighbour_left_ID() { return neighbour_left_ID; };
	int get_neighbour_right_ID() { return neighbour_right_ID; };
	int get_neighbour_upleft_ID() { return neighbour_upleft_ID; };
	int get_neighbour_upright_ID() { return neighbour_upright_ID; };
	int get_neighbour_downleft_ID() { return neighbour_downleft_ID; };
	int get_neighbour_downright_ID() { return neighbour_downright_ID; };
	void get_all_neighbour(int neighbour_ID[9]);
	void set_offset(XMFLOAT2 terrain_offset_in) { terrain_offset = terrain_offset_in; };
	XMFLOAT2 get_offset() { return terrain_offset; }
	engine_basic::engine_fail_reason build_resource(pancy_physx_scene *physic_scene);
	void release_resource(pancy_physx_scene *physic_scene);
	void display(XMFLOAT3 view_pos, XMFLOAT4X4 view_mat, XMFLOAT4X4 proj_mat, std::unordered_map<std::string, terrain_color_resource> material_list);
	void display_gbuffer(XMFLOAT3 view_pos, XMFLOAT4X4 view_mat, XMFLOAT4X4 proj_mat, std::unordered_map<std::string, terrain_color_resource> material_list);
private:
	engine_basic::engine_fail_reason build_physic(pancy_physx_scene *physic_scene);
};
class pancy_terrain_control
{
	//地形材质包
	std::unordered_map<std::string, terrain_color_resource> terrain_material_map;
	//地形物理信息
	pancy_physx_scene *physic_scene;
	string terrain_list_file;
	int now_center_terrain = 0;
	//地形基本信息
	float terrain_width;
	int terrain_divide;
	float Terrain_ColorTexScal;
	float Terrain_HeightScal;
	//地形组合信息
	std::unordered_map<int, terrain_part_resource> terrain_data_list;
	/*
	 —————————
	| rebuild↑distance|
	|     ————     |
	|    | do not |  →→rebuild place
	|    | rebuild|    |
	|     ————     |
	|                  |
	 —————————
	 when the camera get into rebuild part,
	 the terrain structure should be rebuild.
	 a smaller distance will reduce the object in the view when player near the edge.
	 a farther distance will increase the frequency of the rebuild time.
	*/
	float rebuild_distance_quality;  //边缘重建距离系数distance = rebuild_distance_quality∈(0-1) * map_width + map_width / 2.0f;
	//状态信息
	XMFLOAT3 now_view_pos; //视点位置
	XMFLOAT4X4 view_matrix;//取景变换
	XMFLOAT4X4 proj_matrix;//投影变换
	bool if_created;
public:
	pancy_terrain_control(
		pancy_physx_scene *physc_in,
		string terrain_list,
		float terrain_width_in,
		int terrain_divide_in,
		float Terrain_ColorTexScal_in,
		float Terrain_HeightScal_in,
		float rebuild_dis
		);
	engine_basic::engine_fail_reason create();
	engine_basic::engine_fail_reason load_terrain_material(string mat_file_name, terrain_mat_quality material_quality);
	void update(XMFLOAT3 view_pos, XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix);
	void release();
	void display();
	void display_gbuffer();
private:
	string move_string_space(string input);
	string find_path(string input);
	engine_basic::engine_fail_reason build_neighbour(int node_ID);
	engine_basic::engine_fail_reason build_terrain_tree();
	engine_basic::engine_fail_reason load_a_terrain(int node_ID);
	engine_basic::engine_fail_reason unload_a_terrain(int node_ID);
};


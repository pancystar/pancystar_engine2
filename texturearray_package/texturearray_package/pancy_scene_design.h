#pragma once
#include"geometry.h"
#include"pancystar_engine_basic.h"
#include"pancy_d3d11_basic.h"
#include"shader_pancy.h"
#include"assimp_import.h"
#include"PancyCamera.h"
#include"PancyInput.h"
#include<map>
class scene_root
{
protected:
	float                     time_game;
public:
	scene_root();
	virtual engine_basic::engine_fail_reason create() = 0;
	virtual void display() = 0;
	virtual void display_nopost() = 0;
	virtual void update(float delta_time) = 0;
	virtual void release() = 0;
protected:
};

struct save_picture_place
{
	int x_st;
	int y_st;
	int pic_index;
};
struct texture_rebuild_data 
{
	save_picture_place place_data;
	int pic_width;
	int pic_height;
	int now_index;
};
struct int_list
{
	int data_num;
	save_picture_place data[100];
};
class texture_combine
{
	int picture_type_width;
	int picture_type_height;
	int all_pic;
	bool check_if_use[1000];
	int width[1000];
	int height[1000];
	std::vector<int_list> picture_combine_list;
	ID3D11ShaderResourceView *SRV_array;
	std::vector<ID3D11RenderTargetView *> SRV_list;
public:
	texture_combine(int input_pic_num, int *input_width_list, int *input_height_list, int out_pic_width, int out_pic_height);
	engine_basic::engine_fail_reason create();
	int get_texture_num() { return picture_combine_list.size(); };
	int_list get_texture_data(int index);
	void get_texture_range(int &width_out, int &height_out, int index) { width_out = width[index]; height_out = height[index]; };
	ID3D11ShaderResourceView *get_SRV_texarray();
	ID3D11RenderTargetView *get_RTV_texarray(int index);
	texture_rebuild_data get_diffusetexture_data_byID(int ID_tex);
	void releae();
private:
	engine_basic::engine_fail_reason init_testure();
	int_list count_dfs(int now_st_x, int now_st_y, int now_width, int now_height);
};

class scene_test_square : public scene_root 
{
	int picture_type_width;
	int picture_type_height;
	float rec;
	mesh_cube *mesh_need;
	mesh_square *picture_buf;
	mesh_model<point_common> *model_out_test;
	texture_combine *texture_deal;
	model_reader_assimp<point_common> *mesh_model_need;

	std::map<std::string, ID3D11ShaderResourceView*> rec_texture_packmap;
	std::vector<string> picture_namelist;
public:
	scene_test_square();
	engine_basic::engine_fail_reason create();
	void display();
	void display_nopost() {};
	void update(float delta_time);
	void release();
private:
	void show_model();
	void show_model_single();
	void show_square();
	void change_model_texcoord(point_common *vertex_need, int point_num);
};



class pancy_scene_control
{
	int scene_now_show;
	std::vector<scene_root*> scene_list;
public:
	pancy_scene_control();
	void update(float delta_time);
	void display();
	engine_basic::engine_fail_reason add_a_scene(scene_root* scene_in);
	engine_basic::engine_fail_reason change_now_scene(int scene_ID);
	void release();
};
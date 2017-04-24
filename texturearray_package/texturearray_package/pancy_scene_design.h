#pragma once
#include"geometry.h"
#include"pancystar_engine_basic.h"
#include"pancy_d3d11_basic.h"
#include"shader_pancy.h"
#include"assimp_import.h"
#include"PancyCamera.h"
#include"PancyInput.h"
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
class scene_test_square : public scene_root 
{
	float rec;
	mesh_cube *mesh_need;
	mesh_square *picture_buf;
	model_reader_assimp<point_common> *mesh_model;
	ID3D11RenderTargetView *rendertarget_picture;
public:
	scene_test_square();
	engine_basic::engine_fail_reason create();
	void display();
	void display_nopost() {};
	void update(float delta_time);
	void release();
private:
	void show_model();
	void show_square();
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
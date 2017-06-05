#pragma once
#include"geometry.h"
#include"pancystar_engine_basic.h"
#include"pancy_d3d11_basic.h"
#include"shader_pancy.h"
#include"PancyCamera.h"
#include"PancyInput.h"
#include"pancy_model_control.h"
#include"pancy_pretreatment.h"
#include"pancy_ssao.h"
#include"pancy_lighting.h"
#include<map>
#include <Shlobj.h>  
#include <tchar.h>  
#include <Commctrl.h> 
#pragma comment(lib, "comctl32.lib")  
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
	pancy_model_ID ID_model_castel;
	int model_ID_castel;
	pancy_model_ID ID_model_floor;
	int model_ID_floor;
public:
	scene_test_square();
	engine_basic::engine_fail_reason create();
	void display();
	void display_nopost() {};
	void update(float delta_time);
	void release();
private:
	void show_model_single();
	void show_floor_single();
};











class pancy_scene_control
{
	int scene_now_show;
	Pretreatment_gbuffer *pretreat_render;
	ssao_pancy *ssao_render;
	std::vector<scene_root*> scene_list;
public:
	pancy_scene_control();
	void update(float delta_time);
	void display();
	engine_basic::engine_fail_reason create();
	engine_basic::engine_fail_reason add_a_scene(scene_root* scene_in);
	engine_basic::engine_fail_reason change_now_scene(int scene_ID);
	void release();
};
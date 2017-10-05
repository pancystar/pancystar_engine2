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
#include"pancy_posttreatment.h"
#include"pancy_atmosphere.h"
#include"pancy_FFT_ocean.h"
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
	virtual void display_environment(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix) = 0;
	virtual void update(float delta_time) = 0;
	virtual void release() = 0;
protected:
};

class scene_test_square : public scene_root
{
	float all_time_need = 0;
	pancy_model_ID ID_model_castel;
	int model_ID_castel;
	pancy_model_ID ID_model_floor;
	int model_ID_floor;
	pancy_model_ID ID_model_ball[30];
	int model_ID_ball;
	pancy_model_ID ID_model_sky;
	int model_ID_sky;
	pancy_model_ID ID_model_pbrtest;
	int model_ID_pbrtest;
	ID3D11ShaderResourceView *tex_cubesky;
	OceanSimulator *simulate_ocean;
	FFT_ocean      *render_ocean;
public:
	scene_test_square();
	engine_basic::engine_fail_reason create();
	void display();
	void display_nopost() {};
	void display_environment(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix);
	void update(float delta_time);
	void release();
private:
	void show_model_single(string tech_name, XMFLOAT4X4 *view_matrix = NULL, XMFLOAT4X4 *proj_matrix = NULL);
	void show_floor_single();
	void show_sky_single();
	void show_pbr_test(string tech_name, XMFLOAT4X4 *view_matrix = NULL, XMFLOAT4X4 *proj_matrix = NULL);
};
class scene_test_environment : public scene_root
{
	Geometry_basic           *fullscreen_buffer;
	XMFLOAT3                 up_cube_reflect[6];
	XMFLOAT3                 look_cube_reflect[6];
	float quality_reflect;
	pancy_model_ID ID_model_sky;
	int model_ID_sky;
	ID3D11ShaderResourceView *tex_cubesky;
	ID3D11ShaderResourceView *SRV_cube;
	ID3D11RenderTargetView *RTV_cube[7*6];

	ID3D11ShaderResourceView *SRV_diffusecube;
	ID3D11RenderTargetView *RTV_diffusecube[6];

	ID3D11ShaderResourceView *SRV_singlecube;
	ID3D11RenderTargetView *RTV_singlecube[6];

	ID3D11DepthStencilView   *reflect_cube_DSV;
	Pretreatment_gbuffer *pretreat_render;
	render_posttreatment_HDR  *HDR_tonemapping;

public:
	scene_test_environment(Pretreatment_gbuffer *pretreat_in, render_posttreatment_HDR  *HDR_in);
	engine_basic::engine_fail_reason create();
	void display();
	void display_nopost() {};
	void display_environment(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix);
	void update(float delta_time);
	void release();
private:
	void show_sky_single();
	void show_sky_cube();
	engine_basic::engine_fail_reason create_cubemap();
};


class pancy_scene_control
{
	int scene_now_show;
	XMFLOAT3 sundir;
	ID3D11ShaderResourceView *brdf_pic;
	ID3D11RenderTargetView *brdf_target;
	Pretreatment_gbuffer *pretreat_render;
	ssao_pancy *ssao_render;
	render_posttreatment_RTGR *globel_reflect;
	render_posttreatment_HDR  *HDR_tonemapping;
	std::vector<scene_root*> scene_list;
	atmosphere_pretreatment *atmosphere_texture;
	mesh_square *picture_buf;
	float quality_reflect;
	float delta_time_now;
	float update_time;

	ID3D11ShaderResourceView *reflect_cube_SRV;           //´æ´¢¾²Ì¬cubemappingµÄÎÆÀí×ÊÔ´
	ID3D11RenderTargetView   *reflect_cube_RTV[6];        //´æ´¢¾²Ì¬cubemappingµÄäÖÈ¾Ä¿±ê
	ID3D11DepthStencilView   *reflect_cube_DSV;

	ID3D11RenderTargetView   *reflect_cube_RTV_backbuffer[6];        //´æ´¢¾²Ì¬cubemappingµÄäÖÈ¾Ä¿±ê
	ID3D11ShaderResourceView   *reflect_cube_SRV_backbuffer;

	

	XMFLOAT3                 up_cube_reflect[6];
	XMFLOAT3                 look_cube_reflect[6];
	XMFLOAT3                 center_position;
public:
	pancy_scene_control();
	void update(float delta_time);
	void display();
	engine_basic::engine_fail_reason init_cube_map();
	engine_basic::engine_fail_reason create();
	engine_basic::engine_fail_reason add_a_scene(scene_root* scene_in);
	engine_basic::engine_fail_reason change_now_scene(int scene_ID);

	Pretreatment_gbuffer* get_pretreat() { return pretreat_render; };
	render_posttreatment_HDR* get_post_hdr(){ return HDR_tonemapping; };
	void release();
private:
	void render_environment();
	void render_brdf_texture();
	engine_basic::engine_fail_reason build_brdf_texture();
};
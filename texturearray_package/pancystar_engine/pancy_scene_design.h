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
#include"pancy_terrain.h"
#include"pancy_particle.h"
#include"pancy_environment.h"
#include<map>
#include <Shlobj.h>  
#include <tchar.h>  
#include <Commctrl.h> 
#pragma comment(lib, "comctl32.lib")  
class scene_root
{
protected:
	XMFLOAT3                  scene_center_pos;
	float                     time_game;
	pancy_geometry_control    *geometry_buffer;     //几何体存储
	gbuffer_out_message       *gbuffer_texture_data;//预处理纹理存储
	postRTGR_out_message      *post_buffer_target;  //后处理的渲染目标
	postHDR_out_message       *HDR_buffer_target;   //HDR渲染目标
public:
	scene_root();
	virtual engine_basic::engine_fail_reason create() = 0;
	virtual void display() = 0;
	virtual void display_nopost() = 0;
	virtual void display_environment(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix) = 0;
	virtual void update(float delta_time) = 0;
	virtual void release() = 0;
	gbuffer_render_target*  get_gbuffer_renderdata() { return gbuffer_texture_data->get_gbuffer(); };
	postRTGR_render_target *get_postbuffer_data() { return post_buffer_target->get_gbuffer(); };
	HDR_render_target      get_HDRbuffer_data() { return HDR_buffer_target->get_gbuffer(); };
	pancy_geometry_control   *get_geometry_buffer() { return geometry_buffer; };
	XMFLOAT3               get_scene_center() { return scene_center_pos; };
protected:
	engine_basic::engine_fail_reason create_basic();
	engine_basic::engine_fail_reason release_basic();
};
class real_time_environment
{
	D3D11_VIEWPORT           environment_VP;
	Geometry_basic           *fullscreen_buffer;          //全屏幕平面
	engine_basic::extra_perspective_message *scene_perspective;
	//渲染方向
	XMFLOAT3                 up_cube_reflect[6];
	XMFLOAT3                 look_cube_reflect[6];
	XMFLOAT3                 center_position;
	XMFLOAT3                 front_scene_center;
	int now_render_face;     //当前渲染面
	bool gbuffer_render_turn;//本次是否为gbuffer渲染批次
	float quality_environment;
	//gbuffer处理纹理
	gbuffer_out_message       *gbuffer_texture_data;
	//深度&朝向记录纹理
	ID3D11ShaderResourceView *cube_depthstencil_SRV;
	ID3D11RenderTargetView   *cube_depthstencil_RTV[6];

	ID3D11ShaderResourceView *cube_depthstencil_backSRV;
	ID3D11RenderTargetView   *cube_depthstencil_backRTV[6];
	//颜色记录纹理
	ID3D11ShaderResourceView *cube_rendercolor_SRV;
	ID3D11RenderTargetView   *cube_rendercolor_RTV[6];

	ID3D11ShaderResourceView *cube_rendercolor_backSRV;
	ID3D11RenderTargetView   *cube_rendercolor_backRTV[6];
	ID3D11DepthStencilView   *reflect_cube_DSV;
public:
	//环境反射质量(0-1)
	real_time_environment(float quality_environment_in);
	engine_basic::engine_fail_reason create();
	void display_a_turn(scene_root *environment_scene);
	void release();
	ID3D11ShaderResourceView * get_env_depth_texture() { return cube_depthstencil_SRV; };
	ID3D11ShaderResourceView * get_env_color_texture() { return cube_rendercolor_SRV; };
	XMFLOAT3 get_scene_center() { return front_scene_center; };
private:
	engine_basic::engine_fail_reason init_texture();
	engine_basic::engine_fail_reason init_cube_texture(DXGI_FORMAT tex_format, ID3D11ShaderResourceView **SRV_in, ID3D11RenderTargetView *RTV_in[6], string texture_name);
	void display_backbuffer(scene_root *environment_scene);
	void display_environment(scene_root *environment_scene);
	void get_ViewMatrix(XMFLOAT4X4 *view_matrix, XMFLOAT4X4 *invview_matrix);
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
	void show_sky_single(XMFLOAT4X4 view_matrix, XMFLOAT4X4 *proj_matrix);
	void show_pbr_test(string tech_name, XMFLOAT4X4 *view_matrix = NULL, XMFLOAT4X4 *proj_matrix = NULL);
};
class scene_test_environment : public scene_root
{
	
	particle_looping<point_ParticleBasic>          *particle_fire;
	//pancy_terrain_part        *terrain_need;
	pancy_terrain_control *terrain_test;
	bool if_finish = false;
	float time_need = 0;
	pancy_model_ID ID_model_skin;
	pancy_model_ID ID_model_skin2;
	int model_ID_skin;
	int animation_id;
	model_reader_pancymesh   *test_model;
	environment_IBL_control  *test_IBL;
	pancy_model_ID ID_model_sky;
	int model_ID_sky;
	/*
	gbuffer_out_message      *environment_texture_data;
	Geometry_basic           *fullscreen_buffer;
	XMFLOAT3                 up_cube_reflect[6];
	XMFLOAT3                 look_cube_reflect[6];
	float quality_reflect;
	
	ID3D11ShaderResourceView *tex_cubesky;
	ID3D11ShaderResourceView *SRV_cube;
	ID3D11RenderTargetView *RTV_cube[7*6];

	ID3D11ShaderResourceView *SRV_diffusecube;
	ID3D11RenderTargetView *RTV_diffusecube[6];

	ID3D11ShaderResourceView *SRV_singlecube;
	ID3D11RenderTargetView *RTV_singlecube[6];

	ID3D11DepthStencilView   *reflect_cube_DSV;
	*/
public:
	scene_test_environment();
	engine_basic::engine_fail_reason create();
	void display();
	void display_nopost() {};
	void display_environment(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix);
	void update(float delta_time);
	void release();
private:
	void show_sky_single();
	void show_sky_cube();
	void show_animation_test();
	void show_terrain();
	void show_particle();
	//engine_basic::engine_fail_reason create_cubemap();
};
class scene_test_plant : public scene_root
{
	pancy_model_ID ID_model_sky;
	int model_ID_sky;
	pancy_model_ID ID_model_bone[100];
	int model_ID_bone;
	skin_tree *root_skin;
	std::ifstream skin_instream;
	pancy_model_ID ID_model_skin;
	int model_ID_skin;
	int animation_id;
	int bone_num;
	XMFLOAT4X4 bone_matrix_array[100];
	XMFLOAT4X4 offset_matrix_array[100];
	XMFLOAT4X4 final_matrix_array[100];
	int parent_ID[100];
public:
	scene_test_plant();
	engine_basic::engine_fail_reason create();
	void display();
	void display_nopost() {};
	void display_environment(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix);
	void update(float delta_time);
	void release();
private:
	void show_sky_single();
	void show_bone();
	engine_basic::engine_fail_reason load_skintree(string filename);
	void read_bone_tree(skin_tree *now);
	void update_root(skin_tree *root, XMFLOAT4X4 matrix_parent, int ID_parent);
	void scene_test_plant::get_bone_matrix();
	void show_skinmesh();
};


class scene_game_build : public scene_root
{
};


class pancy_scene_control
{
	float time_count = 0;
	int scene_now_show;
	XMFLOAT3 sundir;
	ID3D11ShaderResourceView *brdf_pic;
	ID3D11RenderTargetView *brdf_target;
	//Pretreatment_gbuffer *pretreat_render;
	ssao_pancy *ssao_render;
	//render_posttreatment_RTGR *globel_reflect;
	//render_posttreatment_HDR  *HDR_tonemapping;
	std::vector<scene_root*> scene_list;
	atmosphere_pretreatment *atmosphere_texture;
	real_time_environment *environment_map_list;




	mesh_square *picture_buf;
	float quality_reflect;
	float delta_time_now;
	float update_time;

	ID3D11ShaderResourceView *reflect_cube_SRV;           //存储静态cubemapping的纹理资源
	ID3D11RenderTargetView   *reflect_cube_RTV[6];        //存储静态cubemapping的渲染目标
	ID3D11DepthStencilView   *reflect_cube_DSV;

	ID3D11RenderTargetView   *reflect_cube_RTV_backbuffer[6];        //存储静态cubemapping的渲染目标
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

	//Pretreatment_gbuffer* get_pretreat() { return pretreat_render; };
	//render_posttreatment_HDR* get_post_hdr(){ return HDR_tonemapping; };
	void release();
private:
	void render_environment();
	void render_brdf_texture();
	engine_basic::engine_fail_reason build_brdf_texture();
};
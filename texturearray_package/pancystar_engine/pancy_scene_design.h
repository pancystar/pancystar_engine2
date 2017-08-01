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
class atmosphere_pretreatment 
{
	Geometry_basic           *fullscreen_buffer;          //全屏幕平面
	ID3D11ShaderResourceView *transmittance_SRV;
	ID3D11RenderTargetView *transmittance_RTV;

	ID3D11ShaderResourceView *Irradiance_SRV;
	ID3D11RenderTargetView *Irradiance_RTV;

	ID3D11ShaderResourceView *SinglMieScattering_SRV;
	std::vector<ID3D11RenderTargetView*>SingleMieScattering_RTV;

	ID3D11ShaderResourceView *Scattering_SRV;
	std::vector<ID3D11RenderTargetView*>Scattering_RTV;

	
	ID3D11ShaderResourceView *DirectIrradiance_SRV;
	ID3D11RenderTargetView *DirectIrradiance_RTV;

	ID3D11ShaderResourceView *IndirectIrradiance_SRV;
	ID3D11RenderTargetView *IndirectIrradiance_RTV;

	ID3D11ShaderResourceView *MultipleScattering_SRV;
	ID3D11RenderTargetView *MultipleScattering_RTV;


	ID3D11ShaderResourceView *delta_Irradiance_SRV;
	ID3D11RenderTargetView *delta_Irradiance_RTV;

	ID3D11ShaderResourceView *delta_MieScattering_SRV;
	std::vector<ID3D11RenderTargetView*>delta_MieScattering_RTV;

	ID3D11ShaderResourceView *delta_rayleigh_scattering_SRV;
	std::vector<ID3D11RenderTargetView*> delta_rayleigh_scattering_RTV;

	ID3D11ShaderResourceView *delta_scattering_density_SRV;
	std::vector<ID3D11RenderTargetView*> delta_scattering_density_RTV;

	ID3D11ShaderResourceView *delta_multi_scattering_SRV;
	std::vector<ID3D11RenderTargetView*> delta_multi_scattering_RTV;
	ID3D11BlendState *add_blend;
public:
	atmosphere_pretreatment();
	engine_basic::engine_fail_reason create();
	void build_atomosphere_texture();
	void display(XMFLOAT3 sundir);
	void release();
private:
	engine_basic::engine_fail_reason init_texture_2D(int width_tex,int height_tex, ID3D11ShaderResourceView **SRV_input, ID3D11RenderTargetView **RTV_input);
	engine_basic::engine_fail_reason init_texture_3D(int width_tex, int height_tex, int depth_tex, ID3D11ShaderResourceView **SRV_input, std::vector<ID3D11RenderTargetView*> &RTV_input);
	engine_basic::engine_fail_reason init_texture();
	void draw_transmittance();
	void draw_irradiance();
	void draw_SingleScattering();
	void draw_Scattering_density(int layer_scattering);
	void draw_indirect_irradiance(int layer_scattering);
	void draw_multi_scattering(int layer_scattering);
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
	void release();
private:
	void render_environment();
	void render_brdf_texture();
	engine_basic::engine_fail_reason build_brdf_texture();
};
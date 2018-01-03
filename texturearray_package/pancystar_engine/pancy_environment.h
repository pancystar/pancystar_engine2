#pragma once
#include"geometry.h"
#include"pancystar_engine_basic.h"
#include"pancy_d3d11_basic.h"
#include"shader_pancy.h"
#include"pancy_model_control.h"
#include"pancy_pretreatment.h"
#include"pancy_lighting.h"
#include"pancy_posttreatment.h"
#include"pancy_atmosphere.h"

class environment_IBL_data
{
	float quality_reflect;
	XMFLOAT3 sun_dir;
	XMFLOAT3                 up_cube_reflect[6];
	XMFLOAT3                 look_cube_reflect[6];
	gbuffer_out_message       *environment_texture_data;
	//ID3D11ShaderResourceView *tex_cubesky;
	ID3D11ShaderResourceView *SRV_cube;
	ID3D11RenderTargetView *RTV_cube[7 * 6];

	ID3D11ShaderResourceView *SRV_diffusecube;
	ID3D11RenderTargetView *RTV_diffusecube[6];

	ID3D11ShaderResourceView *SRV_singlecube;
	ID3D11RenderTargetView *RTV_singlecube[6];

	ID3D11DepthStencilView   *reflect_cube_DSV;
public:
	environment_IBL_data(float quality_texture, XMFLOAT3 sun_dir_in);
	engine_basic::engine_fail_reason create();
	void display_environment(
		int model_ID_sky,
		pancy_geometry_control *geometry_buffer,
		Geometry_basic *fullscreen_buffer
		);
	ID3D11ShaderResourceView *get_SRV_sky() { return SRV_singlecube; };
	ID3D11ShaderResourceView *get_SRV_spec() { return SRV_cube; };
	ID3D11ShaderResourceView *get_SRV_diff() { return SRV_diffusecube; };
	void release();
private:
	engine_basic::engine_fail_reason create_cubemap();
};
class environment_IBL_control
{
	//XMFLOAT2 time_range;
	int environment_num;
	int divide_num_mid;   //太阳处于中间阶段的分段数量
	int divide_num_up;    //太阳处于上空阶段的分段数量
	float quality_sky;    //IBL质量
	int display_count = 0;
	Geometry_basic           *fullscreen_buffer;
	pancy_geometry_control   *geometry_sky;
	std::unordered_map<int, environment_IBL_data> IBL_list;
	std::vector<float> time_list;
	//std::vector<environment_IBL_data> IBL_list;
	pancy_model_ID ID_model_sky;
	int model_ID_sky;
public:
	environment_IBL_control(int divide_num_mid_in, int divide_num_up_in, float quality_sky_in);
	engine_basic::engine_fail_reason create();
	engine_basic::engine_fail_reason add_an_IBL_data(int time, float quality, XMFLOAT3 sun_dir);
	bool display_an_IBL_data();
	void update_sky_data(float delta_time);
	environment_IBL_data *get_IBL_data_by_time(float time);
	void release();
private:
	int   Transform_FloatTime_ToInt(float float_time);
	float Transform_IntTime_ToFloat(int int_time);
};

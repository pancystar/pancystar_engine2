#pragma once
#include"pancy_d3d11_basic.h"
#include"geometry.h"
#include"pancy_model_control.h"
#include"shader_pancy.h"
class ssao_pancy : public engine_basic::window_size_observer
{
	int                      map_width;
	int                      map_height;
	Geometry_basic           *fullscreen_buffer;          //全屏幕平面
	ID3D11ShaderResourceView *randomtex;           //随机纹理资源

	ID3D11ShaderResourceView *normaldepth_tex;     //存储法线和深度的纹理资源
	ID3D11ShaderResourceView *depth_tex;     //存储法线和深度的纹理资源

	ID3D11RenderTargetView   *ambient_target;     //存储ssao的渲染目标
	ID3D11ShaderResourceView *ambient_tex;        //存储ssao的纹理资源

	ID3D11RenderTargetView   *ambient_target_blur1;     //存储用于交换的ssao的渲染目标
	ID3D11ShaderResourceView *ambient_tex_blur1;        //存储用于交换的ssao的纹理资源
	ID3D11RenderTargetView   *ambient_target_blur2;     //存储用于交换的ssao的渲染目标
	ID3D11ShaderResourceView *ambient_tex_blur2;        //存储用于交换的ssao的纹理资源

	XMFLOAT4                 random_Offsets[14];   //十四个用于计算ao的随机采样点
public:
	ssao_pancy(int width, int height);
	void update_windowsize(int wind_width_need, int wind_height_need);
	engine_basic::engine_fail_reason basic_create();
	void compute_ssaomap();
	void get_normaldepthmap(ID3D11ShaderResourceView *normalspec_need, ID3D11ShaderResourceView *depth_need);
	ID3D11ShaderResourceView* get_aomap();
	void blur_ssaomap();
	void check_ssaomap();
	void release();
private:
	engine_basic::engine_fail_reason build_texture();
	void build_offset_vector();
	engine_basic::engine_fail_reason build_randomtex();
	void basic_blur(ID3D11ShaderResourceView *texin, ID3D11RenderTargetView *texout, bool if_row);
	void release_texture();
	template<class T>
	void safe_release(T t)
	{
		if (t != NULL)
		{
			t->Release();
			t = 0;
		}
	}
};
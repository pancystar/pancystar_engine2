#include"pancy_d3d11_basic.h"
#include"geometry.h"
#include"pancy_model_control.h"
#include"shader_pancy.h"
#include"pancy_lighting.h"
#pragma once
class Pretreatment_gbuffer : public engine_basic::window_size_observer
{
	int                      map_width;
	int                      map_height;
	Geometry_basic           *fullscreen_buffer;          //全屏幕平面
	Geometry_basic           *fullscreen_Lbuffer;         //全屏幕光照平面
	ID3D11ShaderResourceView *depthmap_tex;               //保存深度信息的纹理资源
	ID3D11DepthStencilView   *depthmap_target;            //用作渲染目标的缓冲区资源

	ID3D11RenderTargetView   *normalspec_target;          //存储法线和镜面反射系数的渲染目标
	ID3D11ShaderResourceView *normalspec_tex;             //存储法线和镜面反射系数的纹理资源

	ID3D11RenderTargetView   *gbuffer_diffuse_target;     //存储漫反射光照效果的渲染目标
	ID3D11ShaderResourceView *gbuffer_diffuse_tex;        //存储漫反射光照效果的纹理资源

	ID3D11RenderTargetView   *gbuffer_specular_target;    //存储漫反射光照效果的渲染目标
	ID3D11ShaderResourceView *gbuffer_specular_tex;       //存储漫反射光照效果的纹理资源

	ID3D11RenderTargetView   *depthmap_single_target;     //存储深度msaa采样后信息的渲染目标
	ID3D11ShaderResourceView *depthmap_single_tex;        //存储深度msaa采样后信息的纹理资源

	D3D11_VIEWPORT           render_viewport;             //视口信息
public:
	Pretreatment_gbuffer(int width_need, int height_need);
	void update_windowsize(int wind_width_need, int wind_height_need);
	engine_basic::engine_fail_reason create();
	void display();
	void display_lbuffer(bool if_shadow);
	void release();
	ID3D11ShaderResourceView *get_gbuffer_normalspec() { return normalspec_tex; };
	ID3D11ShaderResourceView *get_gbuffer_depth() { return depthmap_single_tex; };
	ID3D11ShaderResourceView *get_gbuffer_difusse() { return gbuffer_diffuse_tex; };
	ID3D11ShaderResourceView *get_gbuffer_specular() { return gbuffer_specular_tex; };
private:
	engine_basic::engine_fail_reason init_texture();
	void set_normalspecdepth_target();
	void set_multirender_target();
	void set_resolvdepth_target();
	void render_gbuffer();
	void render_lbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 invview_matrix, bool if_shadow);
	void resolve_depth_render(ID3DX11EffectTechnique* tech);
	void light_buffer_render(ID3DX11EffectTechnique* tech);
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

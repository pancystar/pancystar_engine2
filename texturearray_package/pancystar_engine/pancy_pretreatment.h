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
	int                      last_reflect_render_face;
	XMFLOAT3                 environment_map_place;
	XMFLOAT3                 environment_map_renderplace;
	XMFLOAT3                 up_cube_reflect[6];
	XMFLOAT3                 look_cube_reflect[6];
	int                      now_reflect_render_face;
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


	ID3D11ShaderResourceView *reflect_cubenormal_SRV;   //存储立方法线贴图的纹理资源
	ID3D11RenderTargetView   *reflect_cubenormal_RTV;   //存储立方法线贴图的渲染目标

	ID3D11DepthStencilView   *reflect_DSV;              //深度缓冲区目标
	ID3D11ShaderResourceView *reflect_depthcube_SRV;    //深度立方贴图

	ID3D11RenderTargetView   *reflect_diffuse_target;     //存储反射贴图漫反射光照效果的渲染目标
	ID3D11ShaderResourceView *reflect_diffuse_tex;        //存储反射贴图漫反射光照效果的纹理资源

	ID3D11RenderTargetView   *reflect_specular_target;    //存储反射贴图漫反射光照效果的渲染目标
	ID3D11ShaderResourceView *reflect_specular_tex;       //存储反射贴图漫反射光照效果的纹理资源

	ID3D11ShaderResourceView *reflect_cubestencil_SRV;    //存储静态cubemapping的纹理资源
	ID3D11RenderTargetView   *reflect_cubestencil_RTV[6]; //存储静态cubemapping的渲染目标

	ID3D11ShaderResourceView *reflect_cubestencil_SRV_backbuffer;    //存储静态cubemapping的纹理资源缓存
	ID3D11RenderTargetView   *reflect_cubestencil_RTV_backbuffer[6]; //存储静态cubemapping的渲染目标缓存

	ID3D11RenderTargetView   *posttreatment_RTV;           //存储用于后处理的纹理资源
	ID3D11RenderTargetView   *reflectmask_RTV;             //存储用于标记反射区域的纹理资源

	float quality_reflect;
	D3D11_VIEWPORT           render_viewport;             //视口信息
public:
	Pretreatment_gbuffer(int width_need, int height_need,float quality_reflect_need);
	void update_windowsize(int wind_width_need, int wind_height_need);
	engine_basic::engine_fail_reason create();
	void display();
	void display_lbuffer(bool if_shadow);
	void release();
	ID3D11ShaderResourceView *get_gbuffer_normalspec() { return normalspec_tex; };
	ID3D11ShaderResourceView *get_gbuffer_depth() { return depthmap_single_tex; };
	ID3D11ShaderResourceView *get_gbuffer_difusse() { return gbuffer_diffuse_tex; };
	ID3D11ShaderResourceView *get_gbuffer_specular() { return gbuffer_specular_tex; };	

	ID3D11ShaderResourceView *get_reflect_difusse() { return reflect_diffuse_tex; };
	ID3D11ShaderResourceView *get_reflect_specular() { return reflect_specular_tex; };

	ID3D11RenderTargetView *get_posttreat_color_map() { return posttreatment_RTV; };
	ID3D11RenderTargetView *get_posttreat_mask_map() { return reflectmask_RTV; };

	ID3D11ShaderResourceView *get_reflect_mask_map() { return reflect_cubestencil_SRV; };
	XMFLOAT3 get_environment_map_place() { return environment_map_place; };
	XMFLOAT3 get_environment_map_renderplace() { return environment_map_renderplace; };
	int  get_now_reflect_render_face() { return now_reflect_render_face; };
	void upadte_reflect_render_face();
	
	void set_posttreat_input_target();
private:
	engine_basic::engine_fail_reason init_texture();
	engine_basic::engine_fail_reason init_reflect_texture();
	void set_normalspecdepth_target();
	void set_reflect_normaldepth_target();
	void set_reflect_savedepth_target(int count);
	void set_multirender_target();
	void set_reflect_multirender_target();
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

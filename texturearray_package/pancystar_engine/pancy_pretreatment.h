#include"pancy_d3d11_basic.h"
#include"geometry.h"
#include"pancy_model_control.h"
#include"shader_pancy.h"
#include"pancy_lighting.h"
#pragma once
struct gbuffer_render_target
{
	bool IF_MSAA;
	D3D11_VIEWPORT           render_viewport;             //视口信息
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~gbuffer阶段输入~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11ShaderResourceView *depthmap_tex;               //保存深度信息的纹理资源
	ID3D11DepthStencilView   *depthmap_target;            //用作渲染目标的缓冲区资源

	ID3D11RenderTargetView   *normalspec_target;          //存储法线和镜面反射系数的渲染目标
	ID3D11ShaderResourceView *normalspec_tex;             //存储法线和镜面反射系数的纹理资源

	ID3D11RenderTargetView   *AtmosphereMask_target;      //存储大气光照掩码的渲染目标
	ID3D11ShaderResourceView *AtmosphereMask_tex;         //存储大气光照掩码的纹理资源

	ID3D11RenderTargetView   *specroughness_target;       //存储法线和镜面反射系数的渲染目标
	ID3D11ShaderResourceView *specroughness_tex;          //存储法线和镜面反射系数的纹理资源
	//MSAA深度纹理
	ID3D11RenderTargetView   *depthmap_single_target;     //存储深度msaa采样后信息的渲染目标
	ID3D11ShaderResourceView *depthmap_single_tex;        //存储深度msaa采样后信息的纹理资源
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~lbuffer阶段输入~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11RenderTargetView   *gbuffer_diffuse_target;     //存储漫反射光照效果的渲染目标
	ID3D11ShaderResourceView   *gbuffer_diffuse_tex;      //存储漫反射光照效果的渲染纹理

	ID3D11RenderTargetView   *gbuffer_specular_target;    //存储漫镜面反射光照效果的渲染目标
	ID3D11ShaderResourceView   *gbuffer_specular_tex;     //存储漫镜面反射光照效果的渲染纹理

	ID3D11RenderTargetView   *gbuffer_atmosphere_target;  //存储大气光照效果的渲染目标
	ID3D11ShaderResourceView   *gbuffer_atmosphere_tex;   //存储大气光照效果的渲染纹理
};
class gbuffer_out_message 
{
	gbuffer_render_target buffer_data;
public:
	gbuffer_out_message(int width_in, int height_in, bool if_MSAA);
	gbuffer_render_target *get_gbuffer() { return &buffer_data; };
	engine_basic::engine_fail_reason create();
	void release();
private:
	engine_basic::engine_fail_reason init_texture();
	engine_basic::engine_fail_reason init_texture_same_resource(DXGI_FORMAT tex_format, ID3D11ShaderResourceView **SRV_in, ID3D11RenderTargetView **RTV_in, string texture_name);
	engine_basic::engine_fail_reason init_texture_diffrent_resource(DXGI_FORMAT tex_format, ID3D11ShaderResourceView **SRV_in, ID3D11RenderTargetView **RTV_in, string texture_name);

};

class Pretreatment_gbuffer
{
	Geometry_basic           *fullscreen_buffer;          //全屏幕平面
	Geometry_basic           *fullscreen_Lbuffer;         //全屏幕光照平面								 
private:
	Pretreatment_gbuffer();
public:
	static Pretreatment_gbuffer* get_instance()
	{
		static Pretreatment_gbuffer* this_instance;
		if (this_instance == NULL)
		{
			this_instance = new Pretreatment_gbuffer();
		}
		return this_instance;
	}
	engine_basic::engine_fail_reason create();
	void render_gbuffer(
		pancy_geometry_control *geometry_list, 
		gbuffer_render_target *render_target_out, 
		XMFLOAT4X4 view_matrix, 
		engine_basic::extra_perspective_message *perspective_message, 
		bool if_static,
		bool if_post
		);
	void render_lbuffer(
		gbuffer_render_target *render_target_out,
		XMFLOAT3 view_position, 
		XMFLOAT4X4 view_matrix, 
		XMFLOAT4X4 invview_matrix, 
		engine_basic::extra_perspective_message *perspective_message, 
		bool if_shadow
		);
	void release();
private:
	void set_normalspecdepth_target(gbuffer_render_target render_target_out,bool if_clear);
	void set_multirender_target(gbuffer_render_target render_target_out);
	void set_resolvdepth_target(gbuffer_render_target render_target_out);
	void resolve_depth_render(ID3DX11EffectTechnique* tech);
	void light_buffer_render(ID3DX11EffectTechnique* tech);


	
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

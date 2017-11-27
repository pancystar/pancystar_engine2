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
	/*
	int                      map_width;
	int                      map_height;
	int                      last_reflect_render_face;
	XMFLOAT3                 environment_map_place;
	XMFLOAT3                 environment_map_renderplace;
	XMFLOAT3                 up_cube_reflect[6];
	XMFLOAT3                 look_cube_reflect[6];
	int                      now_reflect_render_face;
	*/
	Geometry_basic           *fullscreen_buffer;          //全屏幕平面
	Geometry_basic           *fullscreen_Lbuffer;         //全屏幕光照平面								 
	/*
	//gbuffer_render_target    render_target_out;           //渲染目标
	ID3D11ShaderResourceView *depthmap_tex;               //保存深度信息的纹理资源
	ID3D11DepthStencilView   *depthmap_target;            //用作渲染目标的缓冲区资源

	ID3D11RenderTargetView   *normalspec_target;          //存储法线和镜面反射系数的渲染目标
	ID3D11ShaderResourceView *normalspec_tex;             //存储法线和镜面反射系数的纹理资源

	ID3D11RenderTargetView   *AtmosphereMask_target;          //存储大气光照掩码的渲染目标
	ID3D11ShaderResourceView *AtmosphereMask_tex;             //存储大气光照掩码的纹理资源

	ID3D11RenderTargetView   *specroughness_target;       //存储法线和镜面反射系数的渲染目标
	ID3D11ShaderResourceView *specroughness_tex;          //存储法线和镜面反射系数的纹理资源

	ID3D11RenderTargetView   *gbuffer_diffuse_target;     //存储漫反射光照效果的渲染目标
	ID3D11ShaderResourceView *gbuffer_diffuse_tex;        //存储漫反射光照效果的纹理资源

	ID3D11RenderTargetView   *gbuffer_specular_target;    //存储漫镜面反射光照效果的渲染目标
	ID3D11ShaderResourceView *gbuffer_specular_tex;       //存储漫镜面光照效果的纹理资源

	ID3D11RenderTargetView   *gbuffer_atmosphere_target;    //存储大气光照效果的渲染目标
	ID3D11ShaderResourceView *gbuffer_atmosphere_tex;       //存储大气光照效果的纹理资源

	ID3D11RenderTargetView   *depthmap_single_target;     //存储深度msaa采样后信息的渲染目标
	ID3D11ShaderResourceView *depthmap_single_tex;        //存储深度msaa采样后信息的纹理资源


	ID3D11ShaderResourceView *reflect_cubenormal_SRV;   //存储立方法线贴图的纹理资源
	ID3D11RenderTargetView   *reflect_cubenormal_RTV;   //存储立方法线贴图的渲染目标

	ID3D11ShaderResourceView *reflect_cubeSpecRough_SRV;   //存储立方法线贴图的纹理资源
	ID3D11RenderTargetView   *reflect_cubeSpecRough_RTV;   //存储立方法线贴图的渲染目标

	ID3D11ShaderResourceView *reflect_AtmosphereMask_SRV;   //存储立方大气贴图的纹理资源
	ID3D11RenderTargetView   *reflect_AtmosphereMask_RTV;   //存储立方大气贴图的渲染目标

	ID3D11DepthStencilView   *reflect_DSV;              //深度缓冲区目标
	ID3D11ShaderResourceView *reflect_depthcube_SRV;    //深度立方贴图

	ID3D11RenderTargetView   *reflect_diffuse_target;     //存储反射贴图漫反射光照效果的渲染目标
	ID3D11ShaderResourceView *reflect_diffuse_tex;        //存储反射贴图漫反射光照效果的纹理资源

	ID3D11RenderTargetView   *reflect_specular_target;    //存储反射贴图漫反射光照效果的渲染目标
	ID3D11ShaderResourceView *reflect_specular_tex;       //存储反射贴图漫反射光照效果的纹理资源

	ID3D11RenderTargetView   *reflect_atmosphere_target;    //存储反射贴图大气光照效果的渲染目标
	ID3D11ShaderResourceView *reflect_atmosphere_tex;       //存储反射贴图大气射光照效果的纹理资源

	ID3D11ShaderResourceView *reflect_cubestencil_SRV;    //存储静态cubemapping的纹理资源
	ID3D11RenderTargetView   *reflect_cubestencil_RTV[6]; //存储静态cubemapping的渲染目标

	ID3D11ShaderResourceView *reflect_cubestencil_SRV_backbuffer;    //存储静态cubemapping的纹理资源缓存
	ID3D11RenderTargetView   *reflect_cubestencil_RTV_backbuffer[6]; //存储静态cubemapping的渲染目标缓存

	ID3D11RenderTargetView   *posttreatment_RTV;           //存储用于后处理的纹理资源
	ID3D11RenderTargetView   *reflectmask_RTV;             //存储用于标记反射区域的纹理资源

	float quality_reflect;
	D3D11_VIEWPORT           render_viewport;             //视口信息
	*/
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
	//void set_render_target(gbuffer_render_target render_target_in) { render_target_out = render_target_in; };
	void render_gbuffer(
		pancy_geometry_control *geometry_list, 
		gbuffer_render_target *render_target_out, 
		XMFLOAT4X4 view_matrix, 
		engine_basic::extra_perspective_message *perspective_message, 
		bool if_static
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
	/*
	void display();
	void display_lbuffer(bool if_shadow);

	void display(XMFLOAT4X4 view_matrix);
	void display_lbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 invview_matrix, bool if_shadow);
	ID3D11ShaderResourceView *get_gbuffer_normalspec() { return normalspec_tex; };
	ID3D11ShaderResourceView *get_gbuffer_specrough() { return specroughness_tex; };
	ID3D11ShaderResourceView *get_gbuffer_depth() { return depthmap_single_tex; };
	ID3D11ShaderResourceView *get_gbuffer_difusse() { return gbuffer_diffuse_tex; };
	ID3D11ShaderResourceView *get_gbuffer_specular() { return gbuffer_specular_tex; };

	ID3D11ShaderResourceView *get_reflect_difusse() { return reflect_diffuse_tex; };
	ID3D11ShaderResourceView *get_reflect_specular() { return reflect_specular_tex; };
	ID3D11ShaderResourceView *get_reflect_atmosphere() { return reflect_atmosphere_tex; };

	ID3D11RenderTargetView *get_posttreat_color_map() { return posttreatment_RTV; };
	ID3D11RenderTargetView *get_posttreat_mask_map() { return reflectmask_RTV; };

	ID3D11ShaderResourceView *get_reflect_mask_map() { return reflect_cubestencil_SRV; };

	XMFLOAT3 get_environment_map_place() { return environment_map_place; };
	XMFLOAT3 get_environment_map_renderplace() { return environment_map_renderplace; };
	int  get_now_reflect_render_face() { return now_reflect_render_face; };
	void upadte_reflect_render_face();

	void set_posttreat_input_target();
	*/
private:
	void set_normalspecdepth_target(gbuffer_render_target render_target_out);
	void set_multirender_target(gbuffer_render_target render_target_out);
	void set_resolvdepth_target(gbuffer_render_target render_target_out);
	/*
	engine_basic::engine_fail_reason init_texture();
	engine_basic::engine_fail_reason init_reflect_texture();
	void render_gbuffer();
	void render_gbuffer(XMFLOAT4X4 view_matrix);
	void render_lbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 invview_matrix, bool if_shadow);

	void render_lbuffer_cube(XMFLOAT4X4 view_matrix, XMFLOAT4X4 invview_matrix, bool if_shadow);

	void set_reflect_normaldepth_target();
	void set_reflect_savedepth_target(int count);
	void set_reflect_multirender_target();
	void release_texture();
	*/
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

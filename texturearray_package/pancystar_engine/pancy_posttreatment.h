#pragma once
#include"pancy_model_control.h"
#include"shader_pancy.h"
class render_posttreatment_RTGR : public engine_basic::window_size_observer
{
	Geometry_basic           *fullscreen_buffer;         //全屏幕平面
														 //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~反射计算的输入~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11ShaderResourceView *color_tex;                  //存储渲染结果的纹理资源
	ID3D11ShaderResourceView *input_mask_tex;             //存储渲染结果的纹理资源
														  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~反射计算的中间信息存储~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11RenderTargetView   *reflect_target;             //存储动态屏幕空间反射的渲染目标
	ID3D11ShaderResourceView *reflect_tex;                //存储动态屏幕空间反射的纹理资源

	ID3D11RenderTargetView   *mask_target;                //存储动态屏幕空间反射掩码的渲染目标
	ID3D11ShaderResourceView *mask_tex;                   //存储动态屏幕空间反射掩码的纹理资源

	ID3D11RenderTargetView   *blur_reflect_target;        //存储模糊中间结果的渲染目标
	ID3D11ShaderResourceView *blur_reflect_tex;           //存储模糊中间结果的纹理资源

	ID3D11RenderTargetView   *blur_reflect_target2[5];    //存储最终模糊结果的渲染目标
	ID3D11ShaderResourceView *blur_reflect_texarray;      //存储最终模糊结果的纹理数组资源

	//ID3D11RenderTargetView   *blur_reflect_target2;        //存储动态屏幕空间反射的渲染目标
	//ID3D11ShaderResourceView *blur_reflect_tex2;           //存储动态屏幕空间反射的纹理资源

	ID3D11RenderTargetView   *final_reflect_target;       //存储动态屏幕空间反射的渲染目标
	ID3D11ShaderResourceView *final_reflect_tex;          //存储动态屏幕空间反射的纹理资源
	//ID3D11RenderTargetView   *final_reflectmip_target[5]; //存储反射GI的mipmap渲染目标

	ID3D11RenderTargetView   *reflectpass_outRTV;            //存储反射pass结束后的渲染目标
	ID3D11ShaderResourceView *reflectpass_outSRV;            //存储反射pass结束后的纹理资源


	D3D11_VIEWPORT           render_viewport;             //视口信息
	D3D11_VIEWPORT           half_render_viewport;        //视口信息

	XMFLOAT4X4               static_cube_view_matrix[6];  //立方贴图的六个方向的取景变换

	float  width_static_cube;
public:
	render_posttreatment_RTGR();
	void update_windowsize(int wind_width_need, int wind_height_need);
	engine_basic::engine_fail_reason create();
	ID3D11ShaderResourceView* get_output_tex() { return reflectpass_outSRV; };
	void draw_reflect(XMFLOAT3 center_position, ID3D11RenderTargetView *rendertarget_input, ID3D11RenderTargetView *mask_target_input, ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex, ID3D11ShaderResourceView *reflect_cube_SRV, ID3D11ShaderResourceView *reflect_cubestencil_SRV);
	void draw_to_posttarget(ID3D11ShaderResourceView *ao_in, ID3D11ShaderResourceView *metallic_in, ID3D11ShaderResourceView *specrough_in, ID3D11ShaderResourceView *brdf_in);
	void release();
private:
	engine_basic::engine_fail_reason build_texture();
	void release_texture();
	void build_reflect_map(XMFLOAT3 center_position, ID3D11RenderTargetView *rendertarget_input, ID3D11RenderTargetView *mask_target_input, ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex, ID3D11ShaderResourceView *reflect_cube_SRV, ID3D11ShaderResourceView *reflect_cubestencil_SRV);
	void blur_map(ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex);
	void basic_blur(ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex, ID3D11ShaderResourceView *mask, ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz);
	void basic_blur_mipmap(ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex, ID3D11ShaderResourceView *mask, ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz, int mip_level);

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


class render_posttreatment_HDR : public engine_basic::window_size_observer
{
	//全屏四边形
	mesh_square                 *HDR_fullscreen;
	int                         width, height;       //屏幕宽高
	//内部变量
	ID3D11UnorderedAccessView   *UAV_HDR_mid;        //HDR的缓冲区，用于中间计算
	ID3D11UnorderedAccessView   *UAV_HDR_final;      //HDR的缓冲区，用于存储结果
	ID3D11ShaderResourceView    *SRV_HDR_map;        //HDR的缓冲区，用于存储map结果
	ID3D11ShaderResourceView    *SRV_HDR_use;        //HDR输入部分，要把屏幕像素转换成非抗锯齿的纹理

	ID3D11ShaderResourceView    *SRV_HDR_save;       //HDR高光存储部分渲染资源，将高光进行存储。
	ID3D11RenderTargetView      *RTV_HDR_save;       //HDR高光存储部分渲染目标，将高光进行存储。

	ID3D11ShaderResourceView    *SRV_HDR_blur1;       //HDR高光模糊渲染资源。
	ID3D11RenderTargetView      *RTV_HDR_blur1;       //HDR高光模糊渲染目标。
	ID3D11ShaderResourceView    *SRV_HDR_blur2;       //HDR高光模糊渲染资源。
	ID3D11RenderTargetView      *RTV_HDR_blur2;       //HDR高光模糊渲染目标。

	D3D11_VIEWPORT              render_viewport;      //视口信息
	ID3D11Buffer*               CPU_read_buffer;
	float                       average_light;
	float                       average_light_last;
	int width_rec, height_rec, buffer_num, map_num;
public:
	render_posttreatment_HDR(int width_need, int height_need);
	void update_windowsize(int wind_width_need, int wind_height_need);
	engine_basic::engine_fail_reason create();
	void release();
	engine_basic::engine_fail_reason display(ID3D11ShaderResourceView *SRV_HDR_use_in);
private:
	engine_basic::engine_fail_reason init_buffer();
	engine_basic::engine_fail_reason init_texture();
	engine_basic::engine_fail_reason CreateCPUaccessBuf(int size_need);
	void basic_blur(ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz);
	engine_basic::engine_fail_reason count_average_light(ID3D11ShaderResourceView *SRV_HDR_use_in);
	engine_basic::engine_fail_reason build_preblur_map();
	engine_basic::engine_fail_reason blur_map();
	engine_basic::engine_fail_reason HDR_map();
	void release_basic();
};


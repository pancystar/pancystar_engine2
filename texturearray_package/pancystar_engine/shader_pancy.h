#pragma once
#include<windows.h>
#include<iostream>
#include<D3D11.h>
#include<assert.h>
#include<d3dx11effect.h>
//#include<d3dx11dbg.h>
#include<directxmath.h>
#include <sstream>
#include <fstream>
#include <vector>
#include<d3dcompiler.h>
#include<unordered_map>
#include<typeinfo>
#include<typeindex>
#include"pancystar_engine_basic.h"
#include"pancy_d3d11_basic.h"
using namespace DirectX;
enum light_type
{
	direction_light = 0,
	point_light = 1,
	spot_light = 2
};
enum shadow_type
{
	shadow_none = 0,
	shadow_map = 1,
	shadow_volume = 2
};
struct pancy_light_basic
{
	XMFLOAT4    ambient;
	XMFLOAT4    diffuse;
	XMFLOAT4    specular;

	XMFLOAT3    dir;
	float       spot;

	XMFLOAT3    position;
	float       theta;

	XMFLOAT3    decay;
	float       range;

	XMUINT4    light_type;
};
struct material_handle//为shader中材质相关的全局变量赋值的句柄集合
{
	ID3DX11EffectVariable *ambient;
	ID3DX11EffectVariable *diffuse;
	ID3DX11EffectVariable *specular;
};
struct pancy_material//材质结构
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	XMFLOAT4 reflect;
};

class shader_basic
{
protected:
	ID3D11InputLayout                     *input_need;          //定义了shader及其接受输入的格式
	ID3DX11Effect                         *fx_need;             //shader接口
	LPCWSTR                               shader_filename;      //shader文件名
	std::string                           shader_file_string;
public:
	shader_basic(LPCWSTR filename);				 //构造函数，输入shader文件的文件名
	engine_basic::engine_fail_reason shder_create();
	engine_basic::engine_fail_reason get_technique(ID3DX11EffectTechnique** tech_need, LPCSTR tech_name); //获取渲染路径
	engine_basic::engine_fail_reason get_technique(D3D11_INPUT_ELEMENT_DESC member_point[], UINT num_member, ID3DX11EffectTechnique** tech_need, LPCSTR tech_name); //获取特殊渲染路径
	virtual void release() = 0;
protected:
	engine_basic::engine_fail_reason combile_shader(LPCWSTR filename);		//shader编译接口
	virtual void init_handle() = 0;                 //注册全局变量句柄
	virtual void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member) = 0;
	engine_basic::engine_fail_reason set_matrix(ID3DX11EffectMatrixVariable *mat_handle, XMFLOAT4X4 *mat_need);
	bool WCharToMByte(LPCWSTR lpcwszStr, std::string &str);
	void release_basic();
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
class color_shader : public shader_basic 
{
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //全套几何变换句柄
public:
	color_shader(LPCWSTR filename);
	engine_basic::engine_fail_reason set_trans_all(XMFLOAT4X4 *mat_need);                            //设置总变换
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};

class picture_show_shader : public shader_basic
{
	ID3DX11EffectVariable                    *UI_scal_handle;
	ID3DX11EffectVariable                    *UI_position_handle;
	ID3DX11EffectShaderResourceVariable      *tex_color_input;      //shader中的纹理资源句柄
public:
	picture_show_shader(LPCWSTR filename);
	engine_basic::engine_fail_reason set_tex_color_resource(ID3D11ShaderResourceView *buffer_input);
	engine_basic::engine_fail_reason set_UI_scal(XMFLOAT4 scal_range);
	engine_basic::engine_fail_reason set_UI_position(XMFLOAT4 position_range);
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class virtual_light_shader : public shader_basic
{
	ID3DX11EffectMatrixVariable           *world_matrix_handle;      //世界变换句柄
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;      //法线变换句柄
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //全套几何变换句柄
	ID3DX11EffectMatrixVariable           *ssao_matrix_handle;       //ssao矩阵变换句柄

	ID3DX11EffectShaderResourceVariable   *texture_diffuse_handle;      //漫反射贴图句柄
	ID3DX11EffectShaderResourceVariable   *texture_normal_handle;       //法线贴图纹理
	ID3DX11EffectShaderResourceVariable   *texture_specular_handle;     //高光贴图句柄

	ID3DX11EffectShaderResourceVariable   *texture_diffusearray_handle;     //漫反射贴图句柄

	ID3DX11EffectShaderResourceVariable   *texture_ssao_handle;      //环境光纹理资源句柄
public:
	virtual_light_shader(LPCWSTR filename);
	engine_basic::engine_fail_reason set_trans_world(XMFLOAT4X4 *mat_world);//设置总变换
	engine_basic::engine_fail_reason set_trans_all(XMFLOAT4X4 *mat_need);//设置总变换
	engine_basic::engine_fail_reason set_trans_ssao(XMFLOAT4X4 *mat_need);                   //设置环境光变换

	engine_basic::engine_fail_reason set_tex_diffuse(ID3D11ShaderResourceView *tex_in);//设置漫反射纹理
	engine_basic::engine_fail_reason set_tex_normal(ID3D11ShaderResourceView *tex_in);//设置法线纹理
	engine_basic::engine_fail_reason set_tex_specular(ID3D11ShaderResourceView *tex_in);//设置高光纹理
	engine_basic::engine_fail_reason set_ssaotex(ID3D11ShaderResourceView *tex_in);			//设置ssaomap

	engine_basic::engine_fail_reason set_tex_diffuse_array(ID3D11ShaderResourceView *tex_in);//设置漫反射纹理数组
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_pretreat_gbuffer : public shader_basic
{
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //全套几何变换句柄
	ID3DX11EffectMatrixVariable           *world_matrix_handle;        //世界变换句柄
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;       //法线变换句柄
	ID3DX11EffectMatrixVariable           *world_matrix_array_handle;  //世界变换组矩阵
	ID3DX11EffectMatrixVariable           *normal_matrix_array_handle;  //法线变换组矩阵
	ID3DX11EffectMatrixVariable           *proj_matrix_handle;         //取景*投影变换矩阵

	ID3DX11EffectShaderResourceVariable   *texture_packarray_handle;     //贴图数组
public:
	shader_pretreat_gbuffer(LPCWSTR filename);
	engine_basic::engine_fail_reason set_trans_world(XMFLOAT4X4 *mat_world, XMFLOAT4X4 *mat_view);
	engine_basic::engine_fail_reason set_trans_all(XMFLOAT4X4 *mat_final);
	engine_basic::engine_fail_reason set_trans_proj(XMFLOAT4X4 *mat_need);               //设置取景*投影变换
	engine_basic::engine_fail_reason set_texturepack_array(ID3D11ShaderResourceView *tex_in);
	engine_basic::engine_fail_reason set_world_matrix_array(const XMFLOAT4X4* M, XMFLOAT4X4 mat_view, int cnt);	 //设置世界变换组矩阵
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_resolvedepth : public shader_basic
{
	ID3DX11EffectShaderResourceVariable   *texture_MSAA;
	ID3DX11EffectVariable   *window_size;            //窗口大小
	ID3DX11EffectVariable   *projmessage_handle;            //视点位置
public:
	shader_resolvedepth(LPCWSTR filename);
	engine_basic::engine_fail_reason set_texture_MSAA(ID3D11ShaderResourceView *tex_in);
	engine_basic::engine_fail_reason set_window_size(float width,float height);
	engine_basic::engine_fail_reason set_projmessage(XMFLOAT3 proj_message);
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_ssaomap : public shader_basic
{
	ID3DX11EffectMatrixVariable* ViewToTexSpace;
	ID3DX11EffectVectorVariable* OffsetVectors;
	ID3DX11EffectVectorVariable* FrustumCorners;
	ID3DX11EffectShaderResourceVariable* NormalDepthMap;
	ID3DX11EffectShaderResourceVariable* DepthMap;
	ID3DX11EffectShaderResourceVariable* RandomVecMap;
public:
	shader_ssaomap(LPCWSTR filename);

	engine_basic::engine_fail_reason set_ViewToTexSpace(XMFLOAT4X4 *mat);
	engine_basic::engine_fail_reason set_OffsetVectors(const XMFLOAT4 v[14]);
	engine_basic::engine_fail_reason set_FrustumCorners(const XMFLOAT4 v[4]);
	engine_basic::engine_fail_reason set_NormalDepthtex(ID3D11ShaderResourceView* srv);
	engine_basic::engine_fail_reason set_Depthtex(ID3D11ShaderResourceView* srv);
	engine_basic::engine_fail_reason set_randomtex(ID3D11ShaderResourceView* srv);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_ssaoblur : public shader_basic
{
	ID3DX11EffectScalarVariable* TexelWidth;
	ID3DX11EffectScalarVariable* TexelHeight;

	ID3DX11EffectShaderResourceVariable* NormalDepthMap;
	ID3DX11EffectShaderResourceVariable* DepthMap;
	ID3DX11EffectShaderResourceVariable* InputImage;
public:
	shader_ssaoblur(LPCWSTR filename);
	engine_basic::engine_fail_reason set_image_size(float width, float height);
	engine_basic::engine_fail_reason set_tex_resource(ID3D11ShaderResourceView* tex_normaldepth, ID3D11ShaderResourceView* tex_aomap);
	engine_basic::engine_fail_reason set_Depthtex(ID3D11ShaderResourceView* srv);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class light_shadow : public shader_basic
{
	ID3DX11EffectMatrixVariable           *project_matrix_handle; //全套几何变换句柄
	ID3DX11EffectShaderResourceVariable   *texture_need;
	ID3DX11EffectMatrixVariable           *world_matrix_array_handle;//世界变换组矩阵
	ID3DX11EffectMatrixVariable           *viewproj_matrix_handle;   //取景*投影变换矩阵
public:
	light_shadow(LPCWSTR filename);
	engine_basic::engine_fail_reason set_trans_all(XMFLOAT4X4 *mat_need);        //设置总变换
	engine_basic::engine_fail_reason set_texturepack_array(ID3D11ShaderResourceView *tex_in);
	engine_basic::engine_fail_reason set_world_matrix_array(const XMFLOAT4X4* M, int cnt);	 //设置世界变换组矩阵
	engine_basic::engine_fail_reason set_trans_viewproj(XMFLOAT4X4 *mat_need);               //设置取景*投影变换
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class light_defered_lightbuffer : public shader_basic
{
	//大气预计算贴图
	ID3DX11EffectShaderResourceVariable      *transmittance_texture;                   //透射率纹理
	ID3DX11EffectShaderResourceVariable      *scattering_texture;                      //散射
	ID3DX11EffectShaderResourceVariable      *single_mie_scattering_texture;           //单层米氏散射
	ID3DX11EffectShaderResourceVariable      *irradiance_texture;                      //亮度纹理
	ID3DX11EffectShaderResourceVariable      *mask_texture;                            //掩码图
	//一次设置参数
	ID3DX11EffectVariable                    *white_point;    //白平衡最亮点
	ID3DX11EffectVariable                    *earth_center;   //地心位置
	ID3DX11EffectVariable                    *sun_size;       //太阳大小
	ID3DX11EffectMatrixVariable              *view_from_clip; //反投影光线追踪
	//逐帧设置参数
	ID3DX11EffectVariable                    *camera;         //摄像机位置
	ID3DX11EffectVariable                    *exposure;       //曝光系数
	//光照信息
	ID3DX11EffectVariable                 *light_sun;                  //太阳光
	ID3DX11EffectVariable                 *sunlight_num;               //太阳光分级数量
	ID3DX11EffectVariable                 *depth_devide;               //每一级的深度
	ID3DX11EffectShaderResourceVariable   *suntexture_shadow;          //太阳光阴影纹理资源句柄
	ID3DX11EffectMatrixVariable           *sunshadow_matrix_handle;    //太阳光阴影图变换
	ID3DX11EffectVariable                 *projmessage_handle;            //投影信息
	ID3DX11EffectVariable                 *light_list;                 //灯光
	ID3DX11EffectVariable                 *light_num_handle;           //光源数量
	ID3DX11EffectVariable                 *shadow_num_handle;           //光源数量
	ID3DX11EffectMatrixVariable           *shadow_matrix_handle;       //阴影图变换
	ID3DX11EffectMatrixVariable           *view_matrix_handle;         //取景变换句柄
	ID3DX11EffectMatrixVariable           *invview_matrix_handle;      //取景变换逆变换句柄
	ID3DX11EffectVectorVariable           *FrustumCorners;             //3D还原角点
	ID3DX11EffectShaderResourceVariable   *NormalspecMap;             //法线镜面光纹理资源句柄
	ID3DX11EffectShaderResourceVariable   *SpecRoughnessMap;           //镜面光粗糙度纹理资源句柄
	ID3DX11EffectShaderResourceVariable   *DepthMap;                   //深度纹理资源句柄
	ID3DX11EffectShaderResourceVariable   *texture_shadow;             //阴影纹理资源句柄
public:
	light_defered_lightbuffer(LPCWSTR filename);
	//纹理设置
	engine_basic::engine_fail_reason set_tex_transmittance(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_scattering(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_single_mie_scattering(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_irradiance(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_mask(ID3D11ShaderResourceView *tex_input);
	//矩阵设置
	engine_basic::engine_fail_reason set_view_from_clip(XMFLOAT4X4 view_from_clip_in);
	//其他变量设置
	engine_basic::engine_fail_reason set_white_point(XMFLOAT3 white_point_in);
	engine_basic::engine_fail_reason set_earth_center(XMFLOAT3 earth_center_in);
	engine_basic::engine_fail_reason set_sun_size(XMFLOAT2 sun_size_in);
	engine_basic::engine_fail_reason set_camera(XMFLOAT3 camera_in);
	engine_basic::engine_fail_reason set_exposure(float exposure_in);
	//光照信息
	engine_basic::engine_fail_reason set_sunlight(pancy_light_basic light_need);             //设置太阳光源
	engine_basic::engine_fail_reason set_sunshadow_matrix(const XMFLOAT4X4* M, int cnt);     //设置太阳阴影图变换矩阵
	engine_basic::engine_fail_reason set_sunlight_num(XMUINT3 all_light_num);                //设置太阳光源数量
	engine_basic::engine_fail_reason set_sunshadow_tex(ID3D11ShaderResourceView *tex_in);	//设置阴影纹理
	engine_basic::engine_fail_reason set_depth_devide(XMFLOAT4 v);                           //设置太阳光分级

	engine_basic::engine_fail_reason set_light(pancy_light_basic light_need, int light_num); //设置一个光源
	engine_basic::engine_fail_reason set_light_num(XMUINT3 all_light_num);                   //设置光源数量
	engine_basic::engine_fail_reason set_shadow_num(XMUINT3 all_light_num);                  //设置光源数量
	engine_basic::engine_fail_reason set_FrustumCorners(const XMFLOAT4 v[4]);                //设置3D还原角点
	engine_basic::engine_fail_reason set_shadow_matrix(const XMFLOAT4X4* M, int cnt);		//设置阴影图变换矩阵
	engine_basic::engine_fail_reason set_view_matrix(XMFLOAT4X4 *mat_need);                  //设置取景变换
	engine_basic::engine_fail_reason set_invview_matrix(XMFLOAT4X4 *mat_need);                  //设置取景变换

	engine_basic::engine_fail_reason set_Normalspec_tex(ID3D11ShaderResourceView *tex_in);	//设置法线镜面光纹理
	engine_basic::engine_fail_reason set_SpecRoughness_tex(ID3D11ShaderResourceView *tex_in);	//设置镜面光粗糙度纹理
	engine_basic::engine_fail_reason set_DepthMap_tex(ID3D11ShaderResourceView *tex_in);		//设置深度纹理
	engine_basic::engine_fail_reason set_shadow_tex(ID3D11ShaderResourceView *tex_in);		//设置阴影纹理
	engine_basic::engine_fail_reason set_projmessage(XMFLOAT3 proj_message);
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class light_defered_draw : public shader_basic
{
	ID3DX11EffectVariable                 *material_need;            //材质
	ID3DX11EffectVariable                 *view_pos_handle;          //视点位置
	ID3DX11EffectMatrixVariable           *world_matrix_handle;      //世界变换句柄
	ID3DX11EffectMatrixVariable           *view_matrix_handle;       //取景变换句柄
	ID3DX11EffectMatrixVariable           *invview_matrix_handle;    //取景变换逆变换句柄
	ID3DX11EffectMatrixVariable           *final_matrix_handle;      //全套几何变换句柄
	ID3DX11EffectMatrixVariable           *ssao_matrix_handle;       //ssao矩阵变换句柄
	ID3DX11EffectMatrixVariable           *world_matrix_array_handle;//世界变换组矩阵
	ID3DX11EffectMatrixVariable           *viewproj_matrix_handle;   //取景*投影变换矩阵
	ID3DX11EffectShaderResourceVariable   *tex_light_diffuse_handle; //漫反射光纹理资源句柄
	ID3DX11EffectShaderResourceVariable   *tex_light_specular_handle;//镜面光纹理资源句柄
	ID3DX11EffectShaderResourceVariable   *texture_ssao_handle;      //环境光纹理资源句柄
	ID3DX11EffectShaderResourceVariable   *texture_diffuse_handle;   //漫反射纹理资源句柄
	ID3DX11EffectShaderResourceVariable   *texture_normal_handle;    //法线纹理资源句柄
	ID3DX11EffectShaderResourceVariable   *texture_ibl_handle;       //环境光纹理资源句柄
	ID3DX11EffectShaderResourceVariable   *tex_specroughness;        //镜面光&粗糙度
	ID3DX11EffectShaderResourceVariable   *tex_brdf_list;            //brdf表
public:
	light_defered_draw(LPCWSTR filename);
	engine_basic::engine_fail_reason set_view_pos(XMFLOAT3 eye_pos);
	engine_basic::engine_fail_reason set_trans_ssao(XMFLOAT4X4 *mat_need);                                   //设置环境光变换
	engine_basic::engine_fail_reason set_trans_world(XMFLOAT4X4 *mat_need);                                  //设置世界变换
	engine_basic::engine_fail_reason set_trans_view(XMFLOAT4X4 *mat_need);                                   //设置取景变换
	engine_basic::engine_fail_reason set_trans_invview(XMFLOAT4X4 *mat_need);                                //设置取景变换逆变换
	engine_basic::engine_fail_reason set_trans_all(XMFLOAT4X4 *mat_need);                                    //设置总变换
	engine_basic::engine_fail_reason set_trans_viewproj(XMFLOAT4X4 *mat_need);                               //设置取景*投影变换
	engine_basic::engine_fail_reason set_material(pancy_material material_in);				                 //设置材质
	engine_basic::engine_fail_reason set_ssaotex(ID3D11ShaderResourceView *tex_in);			                 //设置ssaomap
	engine_basic::engine_fail_reason set_tex_diffuse_array(ID3D11ShaderResourceView *tex_in);	             //设置漫反射纹理
	engine_basic::engine_fail_reason set_diffuse_light_tex(ID3D11ShaderResourceView *tex_in);                //设置漫反射光纹理
	engine_basic::engine_fail_reason set_specular_light_tex(ID3D11ShaderResourceView *tex_in);               //设置镜面反射光纹理
	engine_basic::engine_fail_reason set_normal_tex(ID3D11ShaderResourceView *tex_in);                       //设置法线纹理
	engine_basic::engine_fail_reason set_IBL_tex(ID3D11ShaderResourceView *tex_in);                          //设置环境光纹理
	engine_basic::engine_fail_reason set_tex_specroughness_resource(ID3D11ShaderResourceView *buffer_input); //设置镜面光&粗糙度纹理
	engine_basic::engine_fail_reason set_tex_brdflist_resource(ID3D11ShaderResourceView *buffer_input);      //设置brdf查找表
	engine_basic::engine_fail_reason set_world_matrix_array(const XMFLOAT4X4* M, int cnt);	                 //设置世界变换组矩阵
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_reflect_save_depth : public shader_basic
{
	ID3DX11EffectVariable         *cube_count_handle;
	ID3DX11EffectShaderResourceVariable   *depth_input;
public:
	shader_reflect_save_depth(LPCWSTR filename);
	engine_basic::engine_fail_reason set_depthtex_input(ID3D11ShaderResourceView *tex_in);
	engine_basic::engine_fail_reason set_cube_count(XMFLOAT3 cube_count);
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class rtgr_reflect : public shader_basic
{
	ID3DX11EffectVariable*       view_pos_handle;            //视点位置
	ID3DX11EffectVariable        *projmessage_handle;        //投影参数
	ID3DX11EffectMatrixVariable* ViewToTexSpace;
	ID3DX11EffectMatrixVariable* view_matrix_handle;         //取景变换句柄
	ID3DX11EffectMatrixVariable* invview_matrix_handle;      //取景变换逆变换句柄
	ID3DX11EffectMatrixVariable* cubeview_matrix_handle;     //cubemap的六个取景变换矩阵
	ID3DX11EffectVectorVariable* FrustumCorners;
	ID3DX11EffectVectorVariable* camera_positions;
	ID3DX11EffectShaderResourceVariable* NormalDepthMap;
	ID3DX11EffectShaderResourceVariable* DepthMap;
	ID3DX11EffectShaderResourceVariable* texture_diffuse_handle;
	ID3DX11EffectShaderResourceVariable* texture_cube_handle;
	ID3DX11EffectShaderResourceVariable* texture_stencilcube_handle;
	ID3DX11EffectShaderResourceVariable* texture_color_mask;
	ID3DX11EffectShaderResourceVariable* texture_color_ssr;
public:
	rtgr_reflect(LPCWSTR filename);
	engine_basic::engine_fail_reason set_projmessage(XMFLOAT3 proj_message);
	engine_basic::engine_fail_reason set_ViewToTexSpace(XMFLOAT4X4 *mat);
	engine_basic::engine_fail_reason set_FrustumCorners(const XMFLOAT4 v[4]);
	engine_basic::engine_fail_reason set_camera_positions(XMFLOAT3 v);
	engine_basic::engine_fail_reason set_NormalDepthtex(ID3D11ShaderResourceView* srv);
	engine_basic::engine_fail_reason set_Depthtex(ID3D11ShaderResourceView* srv);
	engine_basic::engine_fail_reason set_diffusetex(ID3D11ShaderResourceView* srv);
	engine_basic::engine_fail_reason set_enviroment_tex(ID3D11ShaderResourceView* srv);
	//HRESULT set_enviroment_depth(ID3D11ShaderResourceView* srv);
	engine_basic::engine_fail_reason set_enviroment_stencil(ID3D11ShaderResourceView* srv);
	engine_basic::engine_fail_reason set_color_mask_tex(ID3D11ShaderResourceView* srv);
	engine_basic::engine_fail_reason set_color_ssr_tex(ID3D11ShaderResourceView* srv);
	engine_basic::engine_fail_reason set_invview_matrix(XMFLOAT4X4 *mat_need);                  //设置取景逆变换
	engine_basic::engine_fail_reason set_view_matrix(XMFLOAT4X4 *mat_need);                     //设置取景变换
	engine_basic::engine_fail_reason set_cubeview_matrix(const XMFLOAT4X4* M, int cnt);	       //设置立方取景矩阵
	engine_basic::engine_fail_reason set_view_pos(XMFLOAT3 eye_pos);
	
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class rtgr_reflect_blur : public shader_basic
{
	ID3DX11EffectVariable*             Texelrange;
	ID3DX11EffectVariable*             sample_level_handle;
	ID3DX11EffectShaderResourceVariable      *tex_input;      //shader中的纹理资源句柄
	ID3DX11EffectShaderResourceVariable      *tex_input_array;      //shader中的纹理资源句柄
	ID3DX11EffectShaderResourceVariable      *tex_normal_input;      //shader中的纹理资源句柄
	ID3DX11EffectShaderResourceVariable      *tex_depth_input;      //shader中的纹理资源句柄
	ID3DX11EffectShaderResourceVariable      *tex_mask_input;      //shader中的纹理资源句柄
public:
	rtgr_reflect_blur(LPCWSTR filename);
	engine_basic::engine_fail_reason set_tex_resource(ID3D11ShaderResourceView *buffer_input);
	engine_basic::engine_fail_reason set_tex_resource_array(ID3D11ShaderResourceView *buffer_input);
	engine_basic::engine_fail_reason set_tex_normal_resource(ID3D11ShaderResourceView *buffer_input);
	engine_basic::engine_fail_reason set_tex_depth_resource(ID3D11ShaderResourceView *buffer_input);
	engine_basic::engine_fail_reason set_tex_mask_resource(ID3D11ShaderResourceView *buffer_input);
	engine_basic::engine_fail_reason set_image_size(XMFLOAT4 texel_range);
	engine_basic::engine_fail_reason set_sample_level(XMUINT4 sample_level);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class rtgr_reflect_final : public shader_basic
{
	ID3DX11EffectVariable                    *Texelrange;
	ID3DX11EffectShaderResourceVariable      *tex_color_input;      //直接光照下的颜色
	ID3DX11EffectShaderResourceVariable      *tex_reflect_input;    //环境光的颜色

	ID3DX11EffectShaderResourceVariable      *tex_color_ao;      //环境光遮蔽颜色
	ID3DX11EffectShaderResourceVariable      *tex_metallic;      //金属度
	ID3DX11EffectShaderResourceVariable      *tex_specroughness; //镜面光&粗糙度
	ID3DX11EffectShaderResourceVariable      *tex_brdf_list;     //brdf表
	ID3DX11EffectShaderResourceVariable      *tex_albedo_nov;    //albedo颜色&法线视线点积
public:
	rtgr_reflect_final(LPCWSTR filename);
	engine_basic::engine_fail_reason set_tex_color_resource(ID3D11ShaderResourceView *buffer_input);
	engine_basic::engine_fail_reason set_tex_reflect_resource(ID3D11ShaderResourceView *buffer_input);

	engine_basic::engine_fail_reason set_tex_ao_resource(ID3D11ShaderResourceView *buffer_input);
	engine_basic::engine_fail_reason set_tex_metallic_resource(ID3D11ShaderResourceView *buffer_input);
	engine_basic::engine_fail_reason set_tex_specroughness_resource(ID3D11ShaderResourceView *buffer_input);
	engine_basic::engine_fail_reason set_tex_brdflist_resource(ID3D11ShaderResourceView *buffer_input);
	engine_basic::engine_fail_reason set_tex_tex_albedonov_resource(ID3D11ShaderResourceView *buffer_input);

	engine_basic::engine_fail_reason set_image_size(XMFLOAT4 texel_range);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_skycube : public shader_basic
{
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //全套几何变换句柄
	ID3DX11EffectMatrixVariable           *world_matrix_handle;        //世界变换句柄
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;       //法线变换句柄
	ID3DX11EffectMatrixVariable           *texproj_matrix_handle;      //投影纹理变换句柄

	ID3DX11EffectVariable                 *view_pos_handle;            //视点位置
	ID3DX11EffectShaderResourceVariable   *cubemap_texture;            //立方贴图资源
	ID3DX11EffectShaderResourceVariable   *atomosphere_texture;        //大气贴图资源
public:
	shader_skycube(LPCWSTR filename);
	engine_basic::engine_fail_reason set_view_pos(XMFLOAT3 eye_pos);                                 //设置视点位置
	engine_basic::engine_fail_reason set_trans_world(XMFLOAT4X4 *mat_need);                          //设置世界变换
	engine_basic::engine_fail_reason set_trans_all(XMFLOAT4X4 *mat_need);                            //设置总变换
	engine_basic::engine_fail_reason set_trans_texproj(XMFLOAT4X4 *mat_need);                        //设置投影纹理变换
	engine_basic::engine_fail_reason set_tex_resource(ID3D11ShaderResourceView* tex_cube);           //设置立方纹理资源
	engine_basic::engine_fail_reason set_tex_atmosphere(ID3D11ShaderResourceView* tex_in);           //设置大气纹理资源
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class brdf_envpre_shader : public shader_basic
{
public:
	brdf_envpre_shader(LPCWSTR filename);
	void release();
private:
	void init_handle() {};                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class compute_averagelight : public shader_basic
{
	ID3DX11EffectShaderResourceVariable      *texture_input;      //shader中的纹理资源句柄
	ID3DX11EffectUnorderedAccessViewVariable *buffer_input;       //shader中的纹理资源句柄
	ID3DX11EffectUnorderedAccessViewVariable *buffer_output;	  //compute_shader计算完毕纹理资源
	ID3DX11EffectVariable                    *texture_range;      //输入纹理大小
public:
	compute_averagelight(LPCWSTR filename);
	engine_basic::engine_fail_reason set_compute_tex(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_piccturerange(int width_need, int height_need, int buffer_num, int bytewidth);
	engine_basic::engine_fail_reason set_compute_buffer(ID3D11UnorderedAccessView *buffer_input_need, ID3D11UnorderedAccessView *buffer_output_need);

	void release();
	void dispatch(int width_need, int height_need, int final_need, int map_need);
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_HDRpreblur : public shader_basic
{
	ID3DX11EffectShaderResourceVariable      *tex_input;       //shader中的纹理资源句柄
	ID3DX11EffectShaderResourceVariable      *buffer_input;    //shader中的纹理资源句柄
	ID3DX11EffectVariable                    *lum_message;     //亮度信息及参数
	ID3DX11EffectVariable                    *texture_range;   //输入纹理大小
	ID3DX11EffectMatrixVariable              *matrix_YUV2RGB;  //YUV2RGB变换句柄
	ID3DX11EffectMatrixVariable              *matrix_RGB2YUV;  //RGB2YUV变换句柄
public:
	shader_HDRpreblur(LPCWSTR filename);
	engine_basic::engine_fail_reason set_buffer_input(ID3D11ShaderResourceView *buffer_need, ID3D11ShaderResourceView *tex_need);
	//亮度信息(平均亮度，高光分界点，高光最大值，tonemapping参数)
	engine_basic::engine_fail_reason set_lum_message(float average_lum, float HighLight_divide, float HightLight_max, float key_tonemapping);
	engine_basic::engine_fail_reason set_piccturerange(int width_need, int height_need, int buffer_num, int bytewidth);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_HDRblur : public shader_basic
{
	ID3DX11EffectScalarVariable*             TexelWidth;
	ID3DX11EffectScalarVariable*             TexelHeight;
	ID3DX11EffectShaderResourceVariable      *tex_input;      //shader中的纹理资源句柄
public:
	shader_HDRblur(LPCWSTR filename);
	engine_basic::engine_fail_reason set_tex_resource(ID3D11ShaderResourceView *buffer_input);
	engine_basic::engine_fail_reason set_image_size(float width, float height);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_HDRfinal : public shader_basic
{
	ID3DX11EffectVariable                    *lum_message;    //亮度信息及参数
	ID3DX11EffectShaderResourceVariable      *tex_input;      //原始图像
	ID3DX11EffectShaderResourceVariable      *tex_bloom;      //高亮曝光图形
	ID3DX11EffectShaderResourceVariable      *buffer_input;   //平均亮度buffer
	ID3DX11EffectVariable                    *texture_range;   //输入纹理大小
	ID3DX11EffectMatrixVariable              *matrix_YUV2RGB; //YUV2RGB变换句柄
	ID3DX11EffectMatrixVariable              *matrix_RGB2YUV; //RGB2YUV变换句柄
public:
	shader_HDRfinal(LPCWSTR filename);
	engine_basic::engine_fail_reason set_tex_resource(ID3D11ShaderResourceView *tex_input, ID3D11ShaderResourceView *tex_bloom, ID3D11ShaderResourceView *buffer_need);
	//亮度信息(平均亮度，高光分界点，高光最大值，tonemapping参数)
	engine_basic::engine_fail_reason set_lum_message(float average_lum, float HighLight_divide, float HightLight_max, float key_tonemapping);
	engine_basic::engine_fail_reason set_piccturerange(int width_need, int height_need, int buffer_num, int bytewidth);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_atmosphere_pretreat : public shader_basic 
{
	ID3DX11EffectVariable                    *scattering_order;                        //散射层
	ID3DX11EffectVariable                    *layer;                                   //3d纹理层
	ID3DX11EffectMatrixVariable              *luminance_from_radiance;                 //球光照

	ID3DX11EffectShaderResourceVariable      *transmittance_texture;                   //透射率纹理
	ID3DX11EffectShaderResourceVariable      *single_rayleigh_scattering_texture;      //单层瑞利散射
	ID3DX11EffectShaderResourceVariable      *single_mie_scattering_texture;           //单层米氏散射
	ID3DX11EffectShaderResourceVariable      *multiple_scattering_texture;             //多层散射
	ID3DX11EffectShaderResourceVariable      *irradiance_texture;                      //亮度纹理
	ID3DX11EffectShaderResourceVariable      *scattering_density_texture;              //最终散射目标
public:
	shader_atmosphere_pretreat(LPCWSTR filename);
	engine_basic::engine_fail_reason set_tex_transmittance(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_single_rayleigh_scattering(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_single_mie_scattering(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_multiple_scattering(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_irradiance(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_scattering_density(ID3D11ShaderResourceView *tex_input);
	//层级信息
	engine_basic::engine_fail_reason set_scattering_order(unsigned int scattering_order_in);
	engine_basic::engine_fail_reason set_layer(unsigned int layer_in);
	engine_basic::engine_fail_reason set_luminance_from_radiance(XMFLOAT4X4 luminance_from_radiance_in);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_atmosphere_render : public shader_basic 
{
	//预计算贴图
	ID3DX11EffectShaderResourceVariable      *transmittance_texture;                   //透射率纹理
	ID3DX11EffectShaderResourceVariable      *scattering_texture;                      //散射
	ID3DX11EffectShaderResourceVariable      *single_mie_scattering_texture;           //单层米氏散射
	ID3DX11EffectShaderResourceVariable      *irradiance_texture;                      //亮度纹理
	ID3DX11EffectShaderResourceVariable      *mask_texture;                            //掩码图
	//一次设置参数
	ID3DX11EffectVariable                    *white_point;
	ID3DX11EffectVariable                    *earth_center;
	ID3DX11EffectVariable                    *sun_size;
	ID3DX11EffectMatrixVariable              *view_from_clip;
	//逐帧设置参数
	ID3DX11EffectVariable                    *camera;
	ID3DX11EffectVariable                    *exposure;
	ID3DX11EffectVariable                    *sun_direction;
	ID3DX11EffectMatrixVariable              *model_from_view;
	//深度法线图
	ID3DX11EffectShaderResourceVariable      *depth_texture;//深度图
	ID3DX11EffectShaderResourceVariable      *normal_texture;//法线图
	
	//远截面角点
	ID3DX11EffectVectorVariable* FrustumCorners;
public:
	shader_atmosphere_render(LPCWSTR filename);
	//纹理设置
	engine_basic::engine_fail_reason set_tex_transmittance(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_scattering(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_single_mie_scattering(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_irradiance(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_depth(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_normal(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_tex_mask(ID3D11ShaderResourceView *tex_input);
	//矩阵设置
	engine_basic::engine_fail_reason set_view_from_clip(XMFLOAT4X4 view_from_clip_in);
	engine_basic::engine_fail_reason set_model_from_view(XMFLOAT4X4 view_model_from_view);
	//其他变量设置
	engine_basic::engine_fail_reason set_white_point(XMFLOAT3 white_point_in);
	engine_basic::engine_fail_reason set_earth_center(XMFLOAT3 earth_center_in);
	engine_basic::engine_fail_reason set_sun_size(XMFLOAT2 sun_size_in);
	engine_basic::engine_fail_reason set_camera(XMFLOAT3 camera_in);
	engine_basic::engine_fail_reason set_exposure(float exposure_in);
	engine_basic::engine_fail_reason set_sun_direction(XMFLOAT3 sun_direction_in);
	engine_basic::engine_fail_reason set_FrustumCorners(const XMFLOAT4 v[4]);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class compute_FFT : public shader_basic
{
	ID3DX11EffectShaderResourceVariable      *SRV_input;
	ID3DX11EffectUnorderedAccessViewVariable *UAV_output;	  //compute_shader计算完毕纹理资源
	ID3DX11EffectConstantBuffer              *constent_buffer;    //常量缓冲区
public:
	compute_FFT(LPCWSTR filename);
	engine_basic::engine_fail_reason set_shader_resource(ID3D11ShaderResourceView *data_input);
	engine_basic::engine_fail_reason set_compute_UAV(ID3D11UnorderedAccessView *buffer_input_need);
	engine_basic::engine_fail_reason set_Constant_Buffer(ID3D11Buffer *buffer_input);

	void release();
	void dispatch(int grad, LPCSTR name);
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_ocean_simulateCS : public shader_basic
{
	ID3DX11EffectShaderResourceVariable      *SRV_input_h0;
	ID3DX11EffectShaderResourceVariable      *SRV_input_omega;
	ID3DX11EffectUnorderedAccessViewVariable *UAV_output;	  //compute_shader计算完毕纹理资源

	ID3DX11EffectConstantBuffer              *constent_buffer_Immutable;    //常量缓冲区
	ID3DX11EffectConstantBuffer              *constent_buffer_ChangePerFrame;    //常量缓冲区
public:
	shader_ocean_simulateCS(LPCWSTR filename);
	engine_basic::engine_fail_reason set_shader_resource_h0(ID3D11ShaderResourceView *data_input_h0);
	engine_basic::engine_fail_reason set_shader_resource_omega(ID3D11ShaderResourceView *data_input_omega);
	engine_basic::engine_fail_reason set_compute_UAV(ID3D11UnorderedAccessView *buffer_input_need);
	engine_basic::engine_fail_reason set_Constant_Buffer_Immutable(ID3D11Buffer *buffer_input);
	engine_basic::engine_fail_reason set_Constant_Buffer_ChangePerFrame(ID3D11Buffer *buffer_input);

	void release();
	void dispatch(int grad_x, int grad_y);
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_ocean_simulateVPS : public shader_basic
{
	ID3DX11EffectShaderResourceVariable      *SRV_input_tex;
	ID3DX11EffectShaderResourceVariable      *SRV_input_buffer;

	ID3DX11EffectConstantBuffer              *constent_buffer_Immutable;    //常量缓冲区
	ID3DX11EffectConstantBuffer              *constent_buffer_ChangePerFrame;    //常量缓冲区
public:
	shader_ocean_simulateVPS(LPCWSTR filename);
	engine_basic::engine_fail_reason set_shader_resource_texture(ID3D11ShaderResourceView *data_input);
	engine_basic::engine_fail_reason set_shader_resource_buffer(ID3D11ShaderResourceView *data_input);
	engine_basic::engine_fail_reason set_Constant_Buffer_Immutable(ID3D11Buffer *buffer_input);
	engine_basic::engine_fail_reason set_Constant_Buffer_ChangePerFrame(ID3D11Buffer *buffer_input);

	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_ocean_render : public shader_basic
{
	ID3DX11EffectShaderResourceVariable      *SRV_tex_displayment;
	ID3DX11EffectShaderResourceVariable      *SRV_tex_Perlin;
	ID3DX11EffectShaderResourceVariable      *SRV_tex_gradient;
	ID3DX11EffectShaderResourceVariable      *SRV_tex_Fresnel;
	ID3DX11EffectShaderResourceVariable      *SRV_tex_ReflectCube;

	ID3DX11EffectConstantBuffer              *constent_buffer_Shading;    //常量缓冲区
	ID3DX11EffectConstantBuffer              *constent_buffer_PerCall;    //常量缓冲区
public:
	shader_ocean_render(LPCWSTR filename);
	engine_basic::engine_fail_reason set_texture_displayment(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_texture_Perlin(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_texture_gradient(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_texture_Fresnel(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_texture_ReflectCube(ID3D11ShaderResourceView *tex_input);


	engine_basic::engine_fail_reason set_Constant_Buffer_Shading(ID3D11Buffer *buffer_input);
	engine_basic::engine_fail_reason set_Constant_Buffer_PerCall(ID3D11Buffer *buffer_input);

	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_ocean_draw : public shader_basic
{
	ID3DX11EffectVariable                 *view_pos_handle;          //视点位置
	ID3DX11EffectMatrixVariable              *final_mat_handle;
	ID3DX11EffectMatrixVariable              *scal_mat_handle;
	ID3DX11EffectMatrixVariable              *world_mat_handle;
	ID3DX11EffectMatrixVariable              *normal_mat_handle;

	ID3DX11EffectShaderResourceVariable      *SRV_tex_displayment;
	ID3DX11EffectShaderResourceVariable      *SRV_tex_Perlin;
	ID3DX11EffectShaderResourceVariable      *SRV_tex_gradient;
	ID3DX11EffectShaderResourceVariable      *SRV_tex_Fresnel;
	ID3DX11EffectShaderResourceVariable      *SRV_tex_ReflectCube;

	ID3DX11EffectConstantBuffer              *constent_buffer_Shading;    //常量缓冲区
	ID3DX11EffectConstantBuffer              *constent_buffer_PerCall;    //常量缓冲区
public:
	shader_ocean_draw(LPCWSTR filename);
	engine_basic::engine_fail_reason set_view_pos(XMFLOAT3 eye_pos);
	engine_basic::engine_fail_reason set_texture_displayment(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_texture_Perlin(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_texture_gradient(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_texture_Fresnel(ID3D11ShaderResourceView *tex_input);
	engine_basic::engine_fail_reason set_texture_ReflectCube(ID3D11ShaderResourceView *tex_input);

	engine_basic::engine_fail_reason set_trans_all(XMFLOAT4X4 *mat_need);
	engine_basic::engine_fail_reason set_trans_scal(XMFLOAT4X4 *mat_need);
	engine_basic::engine_fail_reason set_trans_world(XMFLOAT4X4 *mat_need);

	engine_basic::engine_fail_reason set_Constant_Buffer_Shading(ID3D11Buffer *buffer_input);
	engine_basic::engine_fail_reason set_Constant_Buffer_PerCall(ID3D11Buffer *buffer_input);

	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_IBL_specular : public shader_basic
{
	ID3DX11EffectVariable                    *HalfPixel_handle;
	ID3DX11EffectVariable                    *Face_handle;
	ID3DX11EffectVariable                    *MipIndex_handle;
	ID3DX11EffectShaderResourceVariable      *tex_cube_handle;
public:
	shader_IBL_specular(LPCWSTR filename);
	engine_basic::engine_fail_reason set_input_message(XMFLOAT2 HalfPixel, float face_cube, float mip_level);
	engine_basic::engine_fail_reason set_input_CubeTex(ID3D11ShaderResourceView *tex_input);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};

class shader_control
{
private:
	std::unordered_map<std::string, std::shared_ptr<shader_basic>> shader_list;
	shader_control();
public:
	static shader_control *shadercontrol_pInstance;
	static engine_basic::engine_fail_reason single_create()
	{
		if (shadercontrol_pInstance != NULL)
		{
			engine_basic::engine_fail_reason failed_reason("the shader contorller instance have been created before");
			return failed_reason;
		}
		else
		{
			shadercontrol_pInstance = new shader_control();
			engine_basic::engine_fail_reason check_failed = shadercontrol_pInstance->init();
			return check_failed;
		}
	}
	static shader_control * GetInstance()
	{
		return shadercontrol_pInstance;
	}
	engine_basic::engine_fail_reason init();
	//shader的添加与获取
	std::shared_ptr<shader_basic> get_shader_by_type(std::string type_name, engine_basic::engine_fail_reason &if_succeed);
	engine_basic::engine_fail_reason add_a_new_shader(std::type_index class_name, std::shared_ptr<shader_basic> shader_in);
	//引擎基本shader的获取
	std::shared_ptr<color_shader> get_shader_color(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<virtual_light_shader> get_shader_virtual_light(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<picture_show_shader> get_shader_picture(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_pretreat_gbuffer> get_shader_gbuffer(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_resolvedepth> get_shader_resolvedepth(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_ssaomap> get_shader_ssaodraw(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_ssaoblur> get_shader_ssaoblur(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<light_shadow> get_shader_shadowmap(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<light_defered_lightbuffer> get_shader_lightbuffer(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<light_defered_draw> get_shader_lightdeffered(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_reflect_save_depth> get_shader_reflect_savedepth(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<rtgr_reflect> get_shader_reflect_draw(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<rtgr_reflect_blur> get_shader_reflect_blur(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<rtgr_reflect_final> get_shader_reflect_final(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_skycube> get_shader_sky_draw(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<brdf_envpre_shader> get_shader_brdf_pre(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<compute_averagelight> get_shader_hdr_averagelight (engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_HDRpreblur> get_shader_hdr_preblur(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_HDRblur> get_shader_hdr_blur(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_HDRfinal> get_shader_hdr_final(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_atmosphere_pretreat> get_shader_atmosphere_pretreat(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_atmosphere_render> get_shader_atmosphere_render(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<compute_FFT> get_shader_compute_fft(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_ocean_simulateCS> get_shader_oceansimulate_cs(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_ocean_simulateVPS> get_shader_oceansimulate_vps(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_ocean_render> get_shader_oceanrender_vps(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_ocean_draw> get_shader_oceandraw_tess(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_IBL_specular> get_shader_IBL_specular(engine_basic::engine_fail_reason &if_succeed);

	void release();
private:
	engine_basic::engine_fail_reason init_basic();
};
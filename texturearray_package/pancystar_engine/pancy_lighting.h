#pragma once
#include"pancy_model_control.h"
#include"shader_pancy.h"
#include<map>
class basic_shadow_map
{
	int                      shadowmap_width;
	int                      shadowmap_height;
	ID3D11ShaderResourceView *depthmap_tex;     //保存深度信息的纹理资源
	ID3D11DepthStencilView   *depthmap_target;  //用作渲染目标的缓冲区资源(与上面的纹理资源指针共用一片texture资源以实现render depth to teture)

	D3D11_VIEWPORT           shadow_map_VP;     //视口渲染信息
	XMFLOAT4X4               shadow_build;      //生成shadowmap所需要的矩阵
	XMFLOAT4X4               shadow_rebuild;    //调用shadowmap所需要的矩阵
public:
	basic_shadow_map(int width_need, int height_need);
	engine_basic::engine_fail_reason set_renderstate_spot(XMFLOAT3 light_position, XMFLOAT3 light_dir);
	engine_basic::engine_fail_reason set_renderstate(XMFLOAT4X4 shadow_matrix);
	virtual engine_basic::engine_fail_reason create(ID3D11Texture2D* depthMap_array, int index_need);
	ID3D11ShaderResourceView* get_mapresource();
	XMFLOAT4X4 get_shadow_rebuild_matrix() { return shadow_rebuild; };
	XMFLOAT4X4 get_shadow_build_matrix() { return shadow_build; };
	void change_shadowsize(int width_need, int height_need);
	void release();
	engine_basic::engine_fail_reason reset_texture(ID3D11Texture2D* depthMap_array, int index_need);
private:
	engine_basic::engine_fail_reason init_texture(ID3D11Texture2D* depthMap_array, int index_need);
};
//点基光源
class basic_point_lighting
{
protected:
	light_type             light_source_type;
	shadow_type            shadow_source_type;
	pancy_light_basic      light_data;
public:
	basic_point_lighting(light_type type_need_light, shadow_type type_need_shadow);
	void set_light_ambient(float red, float green, float blue, float alpha);
	void set_light_diffuse(float red, float green, float blue, float alpha);
	void set_light_specular(float red, float green, float blue, float alpha);
	void set_light_position(float x, float y, float z);
	void set_light_dir(float x, float y, float z);
	void set_light_decay(float x0, float x1, float x2);
	void set_light_range(float range_need);
	void set_light_spottheata(float theta);
	void set_light_spotstrenth(float spot);
	shadow_type get_shadow_type() { return shadow_source_type; };
	light_type get_light_type() { return light_source_type; };
	//前向光渲染
	void set_frontlight(int light_num);
	//延迟光渲染
	void set_defferedlight(int light_num);
protected:
	void init_comman_dirlight(shadow_type type_need_shadow);
	void init_comman_pointlight(shadow_type type_need_shadow);
	void init_comman_spotlight(shadow_type type_need_shadow);
};

//聚光灯shadowmap
class spotlight_with_shadowmap : public basic_point_lighting
{
	basic_shadow_map *shadowmap_deal;
public:
	spotlight_with_shadowmap(int width_shadow, int height_shadow);
	//engine_basic::engine_fail_reason create(ID3D11Texture2D* depthMap_array, int index_need);
	engine_basic::engine_fail_reason reset_texture(ID3D11Texture2D* depthMap_array, int index_need);
	void draw_shadow(pancy_geometry_control *geometry_list);
	ID3D11ShaderResourceView* get_mapresource() { return shadowmap_deal->get_mapresource(); };
	XMFLOAT4X4 get_shadow_rebuild_matrix() { return shadowmap_deal->get_shadow_rebuild_matrix(); };
	void release();
};

//平行光shadowmap
class sunlight_with_shadowmap : public basic_point_lighting
{
	int shadow_width;
	int shadow_height;
	int                      shadow_devide;                //平行光阴影分级层数
	basic_shadow_map         *shadowmap_array[20];         //每层分级的阴影图
	XMFLOAT4X4               mat_sunlight_pssm[20];        //每层分级的变换矩阵
	ID3D11ShaderResourceView *sunlight_pssm_Shadowresource;//全套阴影图数组
	float                    sunlight_pssm_depthdevide[20];//每层分级的深度
	float                    sunlight_lamda_log;           //对数项的系数
public:
	sunlight_with_shadowmap(int width_need, int height_need, int shadow_divide_num);
	engine_basic::engine_fail_reason create();
	void draw_shadow(pancy_geometry_control *geometry_list);
	void update_view_space(XMFLOAT4X4 mat_sunlight_view, int count) { mat_sunlight_pssm[count] = mat_sunlight_view; };
	ID3D11ShaderResourceView* get_mapresource() { return sunlight_pssm_Shadowresource; };
	XMFLOAT4X4 get_shadow_rebuild_matrix(int count) { return shadowmap_array[count]->get_shadow_rebuild_matrix(); };

	void divide_view_frustum(float lamda_log, int divide_num);
	XMFLOAT4X4 build_matrix_sunlight(float near_plane, float far_plane, XMFLOAT3 light_dir);
	void build_AABB_box(XMFLOAT4 near_point[4], XMFLOAT4 far_point[4], XMFLOAT3 &min_pos, XMFLOAT3 &max_pos);
	void release();
private:
	void draw_shadow_basic(pancy_geometry_control *geometry_list,int count);
};

//光源控制器
class light_control_singleton
{
	int common_shadow_width;
	int common_shadow_height;
	int max_shadow_num;
	int sunlight_use;
	int sunlight_IDcount;
	ID3D11Texture2D          *ShadowTextureArray;      //阴影图数组资源
	ID3D11ShaderResourceView *shadow_map_resource;
	//无影光
	std::vector<basic_point_lighting>                nonshadow_light_list;
	//带阴影的聚光灯
	std::vector<spotlight_with_shadowmap>            shadowmap_light_list;
	//带阴影的大范围平行光
	std::unordered_map<int,sunlight_with_shadowmap>             sun_pssmshadow_light;
	light_control_singleton(int max_shadow_num_need, int common_shadow_width_need, int common_shadow_height_need);
public:
	//对外访问单例接口
	static light_control_singleton *light_control_pInstance;
	static engine_basic::engine_fail_reason single_create(int max_shadow_num_need, int common_shadow_width_need, int common_shadow_height_need)
	{
		if (light_control_pInstance != NULL)
		{
			engine_basic::engine_fail_reason failed_reason("the light control instance have been created before");
			return failed_reason;
		}
		else
		{
			light_control_pInstance = new light_control_singleton(max_shadow_num_need, common_shadow_width_need, common_shadow_height_need);
			engine_basic::engine_fail_reason check_failed = light_control_pInstance->create();
			return check_failed;
		}
	}
	static light_control_singleton * GetInstance()
	{
		return light_control_pInstance;
	}
	//添加光源
	void add_light_without_shadow(light_type type_need_light);
	void add_spotlight_with_shadow_map();
	engine_basic::engine_fail_reason add_sunlight_with_shadow_map(int width_shadow, int height_shadow, int shadow_num,int &sunlight_ID);
	void draw_shadow(pancy_geometry_control *geometry_list);
	void set_sunlight(int sunlight_ID) { sunlight_use = sunlight_ID; };
	engine_basic::engine_fail_reason create();
	void update_sunlight(XMFLOAT3 dir);
	void release();
private:
	void get_shadow_map_matrix(XMFLOAT4X4* mat_out, int &mat_num_out);
	void update_and_setlight();
};
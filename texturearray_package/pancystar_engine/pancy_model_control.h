#pragma once
#include"geometry.h"
#include"shader_pancy.h"
#include"PancyCamera.h"
#include<map>
using namespace std;
#define MAX_BONE_NUM 100
class model_reader_pancymesh
{
protected:
	ID3DX11EffectTechnique *teque_pancy;
	ID3D11ShaderResourceView *test_resource;
public:
	model_reader_pancymesh();
	virtual engine_basic::engine_fail_reason create() = 0;
	engine_basic::engine_fail_reason get_technique(ID3DX11EffectTechnique *teque_need);
	ID3D11ShaderResourceView *get_texture() { return test_resource; }
	virtual void draw_mesh() = 0;
	virtual void draw_mesh_instance(int copy_num) = 0;
	virtual void release() = 0;
protected:
	engine_basic::engine_fail_reason read_texture_from_file(std::vector<string> file_name_list);
};
template<typename T>
class model_reader_pancymesh_build : public model_reader_pancymesh
{
protected:
	string mesh_file_name;
	string mat_file_name;
	mesh_model<T> *model_out_test;
public:
	model_reader_pancymesh_build(string file_name_mesh, string file_name_mat);
	engine_basic::engine_fail_reason create();
	void draw_mesh();
	void draw_mesh_instance(int copy_num);
	virtual void release();
protected:
	virtual engine_basic::engine_fail_reason load_model();
	engine_basic::engine_fail_reason load_model_mesh();
};
template<typename T>
model_reader_pancymesh_build<T>::model_reader_pancymesh_build(string file_name_mesh, string file_name_mat)
{
	mesh_file_name = file_name_mesh;
	mat_file_name = file_name_mat;
}
template<typename T>
engine_basic::engine_fail_reason model_reader_pancymesh_build<T>::create()
{
	return load_model();
}
template<typename T>
engine_basic::engine_fail_reason model_reader_pancymesh_build<T>::load_model()
{
	return load_model_mesh();
}
template<typename T>
engine_basic::engine_fail_reason model_reader_pancymesh_build<T>::load_model_mesh()
{
	T *data_point_need;
	UINT *data_index_need;
	ifstream in_stream;
	in_stream.open(mesh_file_name, ios::binary);
	int vnum_rec = 0, inum_rec, tnum_rec;
	in_stream.read(reinterpret_cast<char*>(&vnum_rec), sizeof(vnum_rec));
	in_stream.read(reinterpret_cast<char*>(&inum_rec), sizeof(inum_rec));
	in_stream.read(reinterpret_cast<char*>(&tnum_rec), sizeof(tnum_rec));
	data_point_need = new T[vnum_rec];
	data_index_need = new UINT[inum_rec];
	in_stream.read(reinterpret_cast<char*>(data_point_need), vnum_rec * sizeof(data_point_need[0]));
	in_stream.read(reinterpret_cast<char*>(data_index_need), inum_rec * sizeof(data_index_need[0]));
	in_stream.close();
	model_out_test = new mesh_model<T>(data_point_need, data_index_need, vnum_rec, inum_rec, false);
	auto error_message = model_out_test->create_object();
	if (!error_message.check_if_failed())
	{
		return error_message;
	}
	std::vector<string> file_name_saving;

	in_stream.open(mat_file_name);
	for (int i = 0; i < tnum_rec; ++i)
	{
		string texture_name_now;
		getline(in_stream, texture_name_now);
		file_name_saving.push_back(texture_name_now);
	}
	error_message = read_texture_from_file(file_name_saving);
	if (!error_message.check_if_failed())
	{
		return error_message;
	}
	in_stream.close();
	delete[] data_point_need;
	delete[] data_index_need;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
template<typename T>
void model_reader_pancymesh_build<T>::draw_mesh()
{
	model_out_test->get_teque(teque_pancy);
	model_out_test->show_mesh();
}
template<typename T>
void model_reader_pancymesh_build<T>::draw_mesh_instance(int copy_num)
{
	model_out_test->get_teque(teque_pancy);
	model_out_test->show_mesh_instance(copy_num);
}
template<typename T>
void model_reader_pancymesh_build<T>::release()
{
	model_out_test->release();
	test_resource->Release();
}
//~~~~~~~~~~~~~~~~~~~~~~~~骨骼动画数据~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct skin_tree
{
	char bone_ID[128];
	int bone_number;
	XMFLOAT4X4 basic_matrix;
	XMFLOAT4X4 animation_matrix;
	XMFLOAT4X4 now_matrix;
	skin_tree *brother;
	skin_tree *son;
	skin_tree()
	{
		bone_ID[0] = '\0';
		bone_number = -1;
		brother = NULL;
		son = NULL;
		XMStoreFloat4x4(&basic_matrix, XMMatrixIdentity());
		XMStoreFloat4x4(&animation_matrix, XMMatrixIdentity());
		XMStoreFloat4x4(&now_matrix, XMMatrixIdentity());
	}
};
//变换向量
struct vector_animation
{
	float time;               //帧时间
	float main_key[3];        //帧数据
};
//变换四元数
struct quaternion_animation
{
	float time;               //帧时间
	float main_key[4];        //帧数据
};
//变换矩阵
struct matrix_animation
{
	float time;               //帧时间
	float main_key[16];       //帧数据
};
struct animation_data
{
	char bone_name[128];                                //本次变换数据对应的骨骼名称
	skin_tree *bone_point;                              //本次变换数据对应的骨骼的指针

	DWORD number_translation;                           //平移变换的数量
	vector_animation *translation_key;                  //各个平移变换数据

	DWORD number_scaling;                               //放缩变换的数量
	vector_animation *scaling_key;                      //各个放缩变换数据

	DWORD number_rotation;                              //旋转变换的数量
	quaternion_animation *rotation_key;                 //各个旋转变换的数据

	DWORD number_transform;                             //混合变换的数量
	matrix_animation *transform_key;                    //各个混合变换的数据	

	//struct animation_data *next;                        //下一个变换数据
};
struct animation_set
{
	char  animation_name[128];                          //该动画的名字
	float animation_length;                             //动画的长度
	DWORD number_animation;                             //动画包含的变换数量
	std::vector<animation_data> animation_datalist;     //该动画的数据
	//animation_data *head_animition;                     
	//animation_set *next;                                //指向下一个动画的指针
};
class model_reader_PancySkinMesh : public model_reader_pancymesh_build<point_skincommon>
{
	string bone_file_name;
	int now_animation_choose;
	//输入流
	ifstream skin_instream;
	//骨骼信息
	skin_tree *root_skin;
	std::vector<animation_set> animation_list;
	float time_all;
	int bone_num;
	XMFLOAT4X4 bone_matrix_array[MAX_BONE_NUM];
	XMFLOAT4X4 offset_matrix_array[MAX_BONE_NUM];
	XMFLOAT4X4 final_matrix_array[MAX_BONE_NUM];
	int hand_matrix_number;
public:
	engine_basic::engine_fail_reason load_model();
	engine_basic::engine_fail_reason set_animation_byname(string name);
	engine_basic::engine_fail_reason set_animation_byindex(int index);
	model_reader_PancySkinMesh(string file_name_mesh, string file_name_mat, string file_name_bone);
	void update_root(skin_tree *root, XMFLOAT4X4 matrix_parent);
	void update_animation(float delta_time);
	void specify_animation_time(float animation_time);
	XMFLOAT4X4* get_bone_matrix();
	XMFLOAT4X4* get_offset_mat() { return offset_matrix_array; };
	float get_animation_length() 
	{
		return animation_list[now_animation_choose].animation_length; 
	};
	int get_bone_num() { return bone_num; };
	engine_basic::engine_fail_reason load_animation_list(string file_name_animation, int &anim_ID);
	void release();
private:
	void free_animation();
	//加载骨骼及动画
	engine_basic::engine_fail_reason load_skintree(string filename);
	void read_bone_tree(skin_tree *now);
	
	bool check_ifsame(char a[], char b[]);
	//查找骨骼信息
	skin_tree* find_tree(skin_tree* p, char name[]);
	skin_tree* find_tree(skin_tree* p, int num);
	void free_tree(skin_tree *now);
	void update_anim_data();
	//寻找动画时间对应的关键帧时间
	void find_anim_sted(int &st, int &ed, quaternion_animation *input, int num_animation);
	void find_anim_sted(int &st, int &ed, vector_animation *input, int num_animation);
	//四元数与向量的插值函数
	void Interpolate(quaternion_animation& pOut, quaternion_animation pStart, quaternion_animation pEnd, float pFactor);
	void Interpolate(vector_animation& pOut, vector_animation pStart, vector_animation pEnd, float pFactor);
	//四元数转矩阵
	void Get_quatMatrix(XMFLOAT4X4 &resMatrix, quaternion_animation& pOut);
	
};

class geometry_instance_view
{
	bool if_show;
	bool if_skin;
	int instance_ID;
	float animation_time;
	int now_animation_use;
	model_reader_PancySkinMesh *skindata;
	XMFLOAT4X4 bone_matrix[MAX_BONE_NUM];
	XMFLOAT4X4 world_matrix;
public:
	geometry_instance_view(int ID_need);
	geometry_instance_view(int ID_need, model_reader_PancySkinMesh *skin_data_in);
	engine_basic::engine_fail_reason get_bone_matrix(XMFLOAT4X4** mat_out, int &bone_num);
	engine_basic::engine_fail_reason Set_AnimationUse_ByID(int anim_ID);
	XMFLOAT4X4 get_world_matrix() { return world_matrix; };
	bool check_if_show() { return if_show; };
	void sleep() { if_show = false; };
	void wakeup() { if_show = true; };
	void update(XMFLOAT4X4 mat_in, float delta_time);
};
class geometry_resource_view
{
	int resource_view_ID;
	int ID_instance_index;//自增ID号
	bool if_cull_front;
	bool if_dynamic;//是否在全局反射中显示
	bool if_skin;//是否拥有骨骼动画
	int max_instance_num;//最大实例数量
	model_reader_pancymesh *model_data;
	std::unordered_map<int, geometry_instance_view> instance_list;
	std::vector<XMFLOAT4X4> world_matrix_array;//实例变换矩阵簇
	//std::vector<XMFLOAT4X4> bone_matrix_array;//骨骼变换矩阵簇
	
	ID3D11ShaderResourceView *bone_matrix_buffer_SRV;
public:
	geometry_resource_view(model_reader_pancymesh *model_data_in,int ID_need,bool if_dynamic_in,bool if_skin_in);
	engine_basic::engine_fail_reason create(int max_instance_num_in);
	engine_basic::engine_fail_reason Load_Animation_FromFile(string file_mame, int &anim_ID);

	std::vector<XMFLOAT4X4> get_matrix_list();
	ID3D11ShaderResourceView * get_bone_matrix_list();
	void get_bonematrix_singledata(XMFLOAT4X4 **mat_in,int &bone_num);
	int get_bone_mat_num();
	int add_an_instance(XMFLOAT4X4 world_matrix);
	engine_basic::engine_fail_reason get_technique(ID3DX11EffectTechnique *teque_need) { return model_data->get_technique(teque_need); };
	engine_basic::engine_fail_reason delete_an_instance(int instance_ID);
	engine_basic::engine_fail_reason sleep_a_instance(int instance_ID);
	engine_basic::engine_fail_reason set_a_instance_anim(int instance_ID, int animation_ID);
	engine_basic::engine_fail_reason wakeup_a_instance(int instance_ID);
	engine_basic::engine_fail_reason update(int instance_ID, XMFLOAT4X4 mat_world, float delta_time);
	void set_cull_front() { if_cull_front = true; };
	bool check_if_cullfront() { return if_cull_front; };
	bool check_if_skin() { return if_skin; };
	ID3D11ShaderResourceView *get_texture() { return model_data->get_texture(); }
	void draw(bool if_static);
	void release();
private:
	engine_basic::engine_fail_reason create_buffer();
};

template<typename T>
class geometry_ResourceView_list
{
protected:
	int ID_reosurce_index;
	std::unordered_map<int, T> ModelResourceView_list;
public:
	int get_now_index() { return ID_reosurce_index; };
	geometry_ResourceView_list();
	int add_new_geometry(T data_input);
	engine_basic::engine_fail_reason delete_geometry_byindex(int resource_ID);
	T *get_geometry_byindex(int index_input);
	int get_geometry_num() { return ModelResourceView_list.size(); };
	//void render_gbuffer();
	void render_gbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix,bool if_static);
	void render_shadowmap(XMFLOAT4X4 shadow_matrix, bool if_static);
	void release();
};
template<typename T>
geometry_ResourceView_list<T>::geometry_ResourceView_list()
{
	ID_reosurce_index = 0;
}
template<typename T>
int geometry_ResourceView_list<T>::add_new_geometry(T data_input)
{
	std::pair<int, T> data_need(ID_reosurce_index, data_input);
	auto check_iferror = ModelResourceView_list.insert(data_need);
	if (!check_iferror.second)
	{
		return -1;
	}
	ID_reosurce_index += 1;
	return ID_reosurce_index - 1;
}
template<typename T>
engine_basic::engine_fail_reason geometry_ResourceView_list<T>::delete_geometry_byindex(int resource_ID)
{
	auto data_now = ModelResourceView_list.find(resource_ID);
	if (data_now == ModelResourceView_list.end())
	{
		stringstream stream;
		stream << resource_ID;
		string string_temp = stream.str();
		engine_basic::engine_fail_reason error_message(string("could not find the model") + "modelID:" + string_temp);
	}
	data_now->second.release();
	ModelResourceView_list.erase(data_now);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
template<typename T>
T* geometry_ResourceView_list<T>::get_geometry_byindex(int resource_ID)
{
	auto data_now = ModelResourceView_list.find(resource_ID);
	if (data_now == ModelResourceView_list.end())
	{
		return NULL;
	}
	return &data_now->second;
}
template<typename T>
void geometry_ResourceView_list<T>::render_gbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix, bool if_static)
{
	//绘制一种模型
	ID3DX11EffectTechnique *teque_need;
	for (auto data_need = ModelResourceView_list.begin(); data_need != ModelResourceView_list.end(); ++data_need)
	{
		engine_basic::engine_fail_reason check_error;
		auto shader_gbuffer = shader_control::GetInstance()->get_shader_gbuffer(check_error);
		//检测当前种类模型的渲染数量是否大于零
		if (data_need->second.get_matrix_list().size() <= 0) 
		{
			continue;
		}
		XMFLOAT4X4 final_mat,viewproj_mat;
		/*
		//从摄像机获取取景变换矩阵
		//pancy_camera::get_instance()->count_view_matrix(&view_mat);
		//从投影单例获得投影变换矩阵
		//XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
		*/
		XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
		//从模型访问器获得世界变换矩阵
		XMFLOAT4X4 world_matrix_rec = data_need->second.get_matrix_list()[0];
		XMMATRIX rec_world = XMLoadFloat4x4(&world_matrix_rec) * XMLoadFloat4x4(&view_matrix) * proj;
		XMStoreFloat4x4(&final_mat, rec_world);
		//设置单个instance的变换矩阵
		shader_gbuffer->set_trans_world(&world_matrix_rec, &view_matrix);
		shader_gbuffer->set_trans_all(&final_mat);
		shader_gbuffer->set_trans_proj(&proj_matrix);
		//设置所有instance的世界变换矩阵
		XMFLOAT4X4 *data_worldmat_array = new XMFLOAT4X4[data_need->second.get_matrix_list().size()];
		int count_num = 0;
		auto matrix_list = data_need->second.get_matrix_list();
		for (auto mat_need = matrix_list.begin(); mat_need != matrix_list.end(); ++mat_need)
		{
			data_worldmat_array[count_num++] = *mat_need._Ptr;
		}


		shader_gbuffer->set_world_matrix_array(data_worldmat_array, view_matrix, data_need->second.get_matrix_list().size());
		delete[] data_worldmat_array;
		//设置纹理数据
		shader_gbuffer->set_texturepack_array(data_need->second.get_texture());
		
		if (data_need->second.get_matrix_list().size() == 1)
		{
			if (data_need->second.check_if_cullfront()) 
			{
				shader_gbuffer->get_technique(&teque_need, "NormalDepth_CullFornt");
			}
			else 
			{
				shader_gbuffer->get_technique(&teque_need,"NormalDepth_withnormal");
			}
			
		}
		else 
		{
			shader_gbuffer->get_technique(&teque_need, "NormalDepth_withinstance_normal");
		}
		data_need->second.get_technique(teque_need);
		data_need->second.draw(if_static);
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetState(NULL);
	}
	//还原渲染状态
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, NULL_target, NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetState(NULL);
}
template<typename T>
void geometry_ResourceView_list<T>::render_shadowmap(XMFLOAT4X4 shadow_matrix, bool if_static)
{
	for (auto data_need = ModelResourceView_list.begin(); data_need != ModelResourceView_list.end(); ++data_need)
	{
		engine_basic::engine_fail_reason check_error;
		auto shader_shadow_map = shader_control::GetInstance()->get_shader_shadowmap(check_error);
		//检测当前种类模型的渲染数量是否大于零
		if (data_need->second.get_matrix_list().size() <= 0)
		{
			continue;
		}
		XMFLOAT4X4 viewproj_mat, final_mat;
		//从模型访问器获得世界变换矩阵
		XMFLOAT4X4 world_matrix_rec = data_need->second.get_matrix_list()[0];
		XMMATRIX rec_world = XMLoadFloat4x4(&world_matrix_rec) * XMLoadFloat4x4(&shadow_matrix);
		XMStoreFloat4x4(&final_mat, rec_world);
		XMStoreFloat4x4(&viewproj_mat, XMLoadFloat4x4(&shadow_matrix));
		//设置单个instance的变换矩阵
		shader_shadow_map->set_trans_all(&final_mat);
		shader_shadow_map->set_trans_viewproj(&viewproj_mat);
		//设置所有instance的世界变换矩阵
		XMFLOAT4X4 *data_worldmat_array = new XMFLOAT4X4[data_need->second.get_matrix_list().size()];
		int count_num = 0;
		auto matrix_list = data_need->second.get_matrix_list();
		for (auto mat_need = matrix_list.begin(); mat_need != matrix_list.end(); ++mat_need)
		{
			data_worldmat_array[count_num++] = *mat_need._Ptr;
		}
		shader_shadow_map->set_world_matrix_array(data_worldmat_array, data_need->second.get_matrix_list().size());
		delete[] data_worldmat_array;
		//设置纹理数据
		shader_shadow_map->set_texturepack_array(data_need->second.get_texture());
		//绘制一种模型
		ID3DX11EffectTechnique *teque_need;
		if (data_need->second.get_matrix_list().size() == 1)
		{
			shader_shadow_map->get_technique(&teque_need, "ShadowTech");
		}
		else
		{
			shader_shadow_map->get_technique(&teque_need, "ShadowTech_instance");
		}
		data_need->second.get_technique(teque_need);
		data_need->second.draw(if_static);
	}
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetState(0);
}
template<typename T>
void geometry_ResourceView_list<T>::release()
{
	for (auto data_need = ModelResourceView_list.begin(); data_need != ModelResourceView_list.end(); ++data_need)
	{
		data_need->second.release();
	}
	ModelResourceView_list.clear();
	for (auto data_GRV = ModelResourceView_list.begin(); data_GRV != ModelResourceView_list.end(); data_GRV++)
	{
		ModelResourceView_list.erase(data_GRV);
	}
}

struct pancy_model_ID
{
	int model_type;
	int model_instance;
};
class pancy_geometry_control
{
	geometry_ResourceView_list<geometry_resource_view> *model_view_list;
private:
	
public:
	/*
	//单例
	static pancy_geometry_control_singleton* get_instance()
	{
		static pancy_geometry_control_singleton* this_instance;
		if (this_instance == NULL)
		{
			this_instance = new pancy_geometry_control_singleton();
		}
		return this_instance;
	}
	*/
	pancy_geometry_control();
	//加载和删除一个模型种类
	engine_basic::engine_fail_reason load_a_model_type(string file_name_mesh, string file_name_mat, bool if_dynamic, int &model_type_ID);
	engine_basic::engine_fail_reason load_a_skinmodel_type(string file_name_mesh, string file_name_mat, string file_name_bone, bool if_dynamic, int &model_type_ID,int max_instance_num);
	engine_basic::engine_fail_reason load_a_skinmodel_animation(int model_type_ID, string file_name_animation, int& animition_ID);
	engine_basic::engine_fail_reason delete_a_model_type(int model_type_ID);
	//添加和删除一个模型实例
	engine_basic::engine_fail_reason add_a_model_instance(int model_type_ID, XMFLOAT4X4 world_Matrix,pancy_model_ID &model_ID);
	engine_basic::engine_fail_reason set_a_instance_animation(pancy_model_ID model_ID,int animation_ID);
	engine_basic::engine_fail_reason delete_a_model_instance(pancy_model_ID model_ID);
	engine_basic::engine_fail_reason update_a_model_instance(pancy_model_ID model_ID, XMFLOAT4X4 world_Matrix, float delta_time);
	engine_basic::engine_fail_reason sleep_a_model_instance(pancy_model_ID model_ID);
	engine_basic::engine_fail_reason wakeup_a_model_instance(pancy_model_ID model_ID);
	//绘制
	engine_basic::engine_fail_reason get_a_model_type(geometry_resource_view **data_out,int model_type_ID);
	void render_gbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix, bool if_static) { model_view_list->render_gbuffer(view_matrix, proj_matrix, if_static); };
	void render_shadowmap(XMFLOAT4X4 shadow_matrix, bool if_static) { model_view_list->render_shadowmap(shadow_matrix, if_static); };
	void release();
};

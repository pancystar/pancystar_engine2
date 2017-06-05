#pragma once
#include"geometry.h"
#include"shader_pancy.h"
#include"PancyCamera.h"
using namespace std;
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
	string mesh_file_name;
	string mat_file_name;
	mesh_model<T> *model_out_test;
public:
	model_reader_pancymesh_build(string file_name_mesh, string file_name_mat);
	engine_basic::engine_fail_reason create();
	void draw_mesh();
	void draw_mesh_instance(int copy_num);
	void release();
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
	T *data_point_need;
	UINT *data_index_need;
	ifstream in_stream;
	in_stream.open(mesh_file_name, ios::binary);
	int vnum_rec, inum_rec, tnum_rec;
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
	read_texture_from_file(file_name_saving);
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


class geometry_instance_view
{
	bool if_show;
	int instance_ID;
	XMFLOAT4X4 world_matrix;
public:
	geometry_instance_view(int ID_need);
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
	model_reader_pancymesh *model_data;
	std::unordered_map<int, geometry_instance_view> instance_list;
	std::vector<XMFLOAT4X4> world_matrix_array;
public:
	geometry_resource_view(model_reader_pancymesh *model_data_in,int ID_need);
	std::vector<XMFLOAT4X4> get_matrix_list();
	int add_an_instance(XMFLOAT4X4 world_matrix);
	engine_basic::engine_fail_reason get_technique(ID3DX11EffectTechnique *teque_need) { return model_data->get_technique(teque_need); };
	engine_basic::engine_fail_reason delete_an_instance(int instance_ID);
	engine_basic::engine_fail_reason sleep_a_instance(int instance_ID);
	engine_basic::engine_fail_reason wakeup_a_instance(int instance_ID);
	engine_basic::engine_fail_reason update(int instance_ID, XMFLOAT4X4 mat_world, float delta_time);
	ID3D11ShaderResourceView *get_texture() { return model_data->get_texture(); }
	void draw();
	void release();
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
	void render_gbuffer();
	void render_shadowmap(XMFLOAT4X4 shadow_matrix);
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
void geometry_ResourceView_list<T>::render_gbuffer()
{
	for (auto data_need = ModelResourceView_list.begin(); data_need != ModelResourceView_list.end(); ++data_need)
	{
		engine_basic::engine_fail_reason check_error;
		auto shader_gbuffer = shader_control::GetInstance()->get_shader_gbuffer(check_error);
		//检测当前种类模型的渲染数量是否大于零
		if (data_need->second.get_matrix_list().size() <= 0) 
		{
			continue;
		}
		XMFLOAT4X4 view_mat, final_mat,viewproj_mat;
		//从摄像机获取取景变换矩阵
		pancy_camera::get_instance()->count_view_matrix(&view_mat);
		//从投影单例获得投影变换矩阵
		XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
		//从模型访问器获得世界变换矩阵
		XMFLOAT4X4 world_matrix_rec = data_need->second.get_matrix_list()[0];
		XMMATRIX rec_world = XMLoadFloat4x4(&world_matrix_rec) * XMLoadFloat4x4(&view_mat) * proj;
		XMStoreFloat4x4(&final_mat, rec_world);
		XMStoreFloat4x4(&viewproj_mat, XMLoadFloat4x4(&view_mat) * proj);
		//设置单个instance的变换矩阵
		shader_gbuffer->set_trans_world(&world_matrix_rec, &view_mat);
		shader_gbuffer->set_trans_all(&final_mat);
		shader_gbuffer->set_trans_viewproj(&viewproj_mat);
		//设置所有instance的世界变换矩阵
		XMFLOAT4X4 *data_worldmat_array = new XMFLOAT4X4[data_need->second.get_matrix_list().size()];
		int count_num = 0;
		auto matrix_list = data_need->second.get_matrix_list();
		for (auto mat_need = matrix_list.begin(); mat_need != matrix_list.end(); ++mat_need)
		{
			data_worldmat_array[count_num++] = *mat_need._Ptr;
		}
		shader_gbuffer->set_world_matrix_array(data_worldmat_array, data_need->second.get_matrix_list().size());
		delete[] data_worldmat_array;
		//设置纹理数据
		shader_gbuffer->set_texturepack_array(data_need->second.get_texture());
		//绘制一种模型
		ID3DX11EffectTechnique *teque_need;
		if (data_need->second.get_matrix_list().size() == 1)
		{
			shader_gbuffer->get_technique(&teque_need,"NormalDepth_withnormal");
		}
		else 
		{
			shader_gbuffer->get_technique(&teque_need, "NormalDepth_withinstance_normal");
		}
		data_need->second.get_technique(teque_need);
		data_need->second.draw();
	}
}
template<typename T>
void geometry_ResourceView_list<T>::render_shadowmap(XMFLOAT4X4 shadow_matrix)
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
		data_need->second.draw();
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
class pancy_geometry_control_singleton
{
	geometry_ResourceView_list<geometry_resource_view> *model_view_list;
private:
	pancy_geometry_control_singleton();
public:
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
	//加载和删除一个模型种类
	engine_basic::engine_fail_reason load_a_model_type(string file_name_mesh, string file_name_mat, int &model_type_ID);
	engine_basic::engine_fail_reason delete_a_model_type(int model_type_ID);
	//添加和删除一个模型实例
	engine_basic::engine_fail_reason add_a_model_instance(int model_type_ID, XMFLOAT4X4 world_Matrix, pancy_model_ID &model_ID);
	engine_basic::engine_fail_reason delete_a_model_instance(pancy_model_ID model_ID);
	engine_basic::engine_fail_reason update_a_model_instance(pancy_model_ID model_ID, XMFLOAT4X4 world_Matrix, float delta_time);
	engine_basic::engine_fail_reason sleep_a_model_instance(pancy_model_ID model_ID);
	engine_basic::engine_fail_reason wakeup_a_model_instance(pancy_model_ID model_ID);
	//绘制
	engine_basic::engine_fail_reason get_a_model_type(geometry_resource_view **data_out,int model_type_ID);
	void render_gbuffer() { model_view_list->render_gbuffer(); };
	void render_shadowmap(XMFLOAT4X4 shadow_matrix) { model_view_list->render_shadowmap(shadow_matrix); };
	void release();
};

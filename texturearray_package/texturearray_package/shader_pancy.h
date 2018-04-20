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
struct material_handle//Ϊshader�в�����ص�ȫ�ֱ�����ֵ�ľ������
{
	ID3DX11EffectVariable *ambient;
	ID3DX11EffectVariable *diffuse;
	ID3DX11EffectVariable *specular;
};
struct pancy_material//���ʽṹ
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	XMFLOAT4 reflect;
};

class shader_basic
{
protected:
	ID3D11InputLayout                     *input_need;          //������shader�����������ĸ�ʽ
	ID3DX11Effect                         *fx_need;             //shader�ӿ�
	LPCWSTR                               shader_filename;      //shader�ļ���
	std::string                           shader_file_string;
public:
	shader_basic(LPCWSTR filename);				 //���캯��������shader�ļ����ļ���
	engine_basic::engine_fail_reason shder_create();
	engine_basic::engine_fail_reason get_technique(ID3DX11EffectTechnique** tech_need, LPCSTR tech_name); //��ȡ��Ⱦ·��
	engine_basic::engine_fail_reason get_technique(D3D11_INPUT_ELEMENT_DESC member_point[], UINT num_member, ID3DX11EffectTechnique** tech_need, LPCSTR tech_name); //��ȡ������Ⱦ·��
	virtual void release() = 0;
protected:
	engine_basic::engine_fail_reason combile_shader(LPCWSTR filename);		//shader����ӿ�
	virtual void init_handle() = 0;                 //ע��ȫ�ֱ������
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
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //ȫ�׼��α任���
public:
	color_shader(LPCWSTR filename);
	engine_basic::engine_fail_reason set_trans_all(XMFLOAT4X4 *mat_need);                            //�����ܱ任
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class virtual_light_shader : public shader_basic
{
	ID3DX11EffectVariable                 *view_pos_handle;            //�ӵ�λ��
	ID3DX11EffectMatrixVariable           *world_matrix_handle;      //����任���
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;      //���߱任���
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //ȫ�׼��α任���
	ID3DX11EffectMatrixVariable           *BoneTransforms;             //�����任����

	ID3DX11EffectShaderResourceVariable   *texture_diffuse_handle;      //��������ͼ���
	ID3DX11EffectShaderResourceVariable   *texture_normal_handle;       //������ͼ����
	ID3DX11EffectShaderResourceVariable   *texture_specular_handle;     //�߹���ͼ���

	ID3DX11EffectShaderResourceVariable   *texture_metallic_handle;       //��������ͼ����
	ID3DX11EffectShaderResourceVariable   *texture_roughness_handle;       //�ֲڶ���ͼ���
	ID3DX11EffectShaderResourceVariable   *texture_brdfluv_handle;       //brdfԤ������ͼ���

	ID3DX11EffectShaderResourceVariable   *texture_diffusearray_handle;     //��������ͼ���
	ID3DX11EffectShaderResourceVariable   *texture_normalarray_handle;     //��������ͼ���
	ID3DX11EffectShaderResourceVariable   *texture_metallicarray_handle;     //��������ͼ���
	ID3DX11EffectShaderResourceVariable   *texture_roughnessarray_handle;     //��������ͼ���
	ID3DX11EffectShaderResourceVariable   *cubemap_texture;                 //������ͼ��Դ

	ID3DX11EffectShaderResourceVariable   *animation_buffer;      //��������
	ID3DX11EffectVariable                 *point_offset_handle;   //��������ƫ��
public:
	virtual_light_shader(LPCWSTR filename);
	engine_basic::engine_fail_reason set_view_pos(XMFLOAT3 eye_pos);
	engine_basic::engine_fail_reason set_trans_world(XMFLOAT4X4 *mat_world);//�����ܱ任
	engine_basic::engine_fail_reason set_trans_all(XMFLOAT4X4 *mat_need);//�����ܱ任
	engine_basic::engine_fail_reason set_bone_matrix(const XMFLOAT4X4* M, int cnt);		     //���ù����任����

	engine_basic::engine_fail_reason set_tex_diffuse(ID3D11ShaderResourceView *tex_in);//��������������
	engine_basic::engine_fail_reason set_tex_normal(ID3D11ShaderResourceView *tex_in);//���÷�������
	engine_basic::engine_fail_reason set_tex_specular(ID3D11ShaderResourceView *tex_in);//���ø߹�����

	engine_basic::engine_fail_reason set_tex_metallic(ID3D11ShaderResourceView *tex_in);//���ý���������
	engine_basic::engine_fail_reason set_tex_roughness(ID3D11ShaderResourceView *tex_in);//���ôֲڶ�����
	engine_basic::engine_fail_reason set_tex_brdfluv(ID3D11ShaderResourceView *tex_in);//���ôֲڶ�����
	
	engine_basic::engine_fail_reason set_tex_diffuse_array(ID3D11ShaderResourceView *tex_in);//������������������
	engine_basic::engine_fail_reason set_tex_normal_array(ID3D11ShaderResourceView *tex_in);//���÷�����������
	engine_basic::engine_fail_reason set_tex_metallic_array(ID3D11ShaderResourceView *tex_in);//���ý�������������
	engine_basic::engine_fail_reason set_tex_roughness_array(ID3D11ShaderResourceView *tex_in);//���ôֲڶ���������

	engine_basic::engine_fail_reason set_tex_environment(ID3D11ShaderResourceView* tex_cube);           //����������Դ

	engine_basic::engine_fail_reason set_animation_buffer(ID3D11ShaderResourceView* buffer_in);
	engine_basic::engine_fail_reason set_animation_offset(XMUINT4 offset_data);
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class picture_show_shader : public shader_basic
{
	ID3DX11EffectVariable                    *UI_scal_handle;
	ID3DX11EffectVariable                    *UI_position_handle;
	ID3DX11EffectShaderResourceVariable      *tex_color_input;      //shader�е�������Դ���
public:
	picture_show_shader(LPCWSTR filename);
	engine_basic::engine_fail_reason set_tex_color_resource(ID3D11ShaderResourceView *buffer_input);
	engine_basic::engine_fail_reason set_UI_scal(XMFLOAT4 scal_range);
	engine_basic::engine_fail_reason set_UI_position(XMFLOAT4 position_range);
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_skycube : public shader_basic
{
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //ȫ�׼��α任���
	ID3DX11EffectMatrixVariable           *world_matrix_handle;        //����任���
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;       //���߱任���
	ID3DX11EffectVariable                 *view_pos_handle;            //�ӵ�λ��
	ID3DX11EffectShaderResourceVariable   *cubemap_texture;            //������ͼ��Դ
public:
	shader_skycube(LPCWSTR filename);
	engine_basic::engine_fail_reason set_view_pos(XMFLOAT3 eye_pos);                                 //�����ӵ�λ��
	engine_basic::engine_fail_reason set_trans_world(XMFLOAT4X4 *mat_need);                          //��������任
	engine_basic::engine_fail_reason set_trans_all(XMFLOAT4X4 *mat_need);                            //�����ܱ任
	engine_basic::engine_fail_reason set_tex_resource(ID3D11ShaderResourceView* tex_cube);           //����������Դ
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class brdf_envpre_shader : public shader_basic
{
public:
	brdf_envpre_shader(LPCWSTR filename);
	void release();
private:
	void init_handle() {};                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class find_clip : public shader_basic
{
	ID3DX11EffectVariable                 *partid_handle;            //����ID
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //ȫ�׼��α任���
public:
	find_clip(LPCWSTR filename);
	engine_basic::engine_fail_reason set_trans_all(XMFLOAT4X4 *mat_need);//�����ܱ任
	engine_basic::engine_fail_reason set_part_ID(XMUINT4 part_ID);
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
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
	std::shared_ptr<shader_basic> get_shader_by_type(std::string type_name, engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<color_shader> get_shader_color(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<virtual_light_shader> get_shader_virtual_light(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<picture_show_shader> get_shader_picture(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<shader_skycube> get_shader_sky_draw(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<brdf_envpre_shader> get_shader_brdf_pre(engine_basic::engine_fail_reason &if_succeed);
	std::shared_ptr<find_clip> get_shader_find_clip(engine_basic::engine_fail_reason &if_succeed);
	engine_basic::engine_fail_reason add_a_new_shader(std::type_index class_name, std::shared_ptr<shader_basic> shader_in);
	void release();
private:
	engine_basic::engine_fail_reason init_basic();
};
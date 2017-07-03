#pragma once
#include"pancy_model_control.h"
#include"shader_pancy.h"
class render_posttreatment_RTGR : public engine_basic::window_size_observer
{
	Geometry_basic           *fullscreen_buffer;         //ȫ��Ļƽ��
														 //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~������������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11ShaderResourceView *color_tex;                  //�洢��Ⱦ�����������Դ
	ID3D11ShaderResourceView *input_mask_tex;             //�洢��Ⱦ�����������Դ
														  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~���������м���Ϣ�洢~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11RenderTargetView   *reflect_target;             //�洢��̬��Ļ�ռ䷴�����ȾĿ��
	ID3D11ShaderResourceView *reflect_tex;                //�洢��̬��Ļ�ռ䷴���������Դ

	ID3D11RenderTargetView   *mask_target;                //�洢��̬��Ļ�ռ䷴���������ȾĿ��
	ID3D11ShaderResourceView *mask_tex;                   //�洢��̬��Ļ�ռ䷴�������������Դ

	ID3D11RenderTargetView   *blur_reflect_target;        //�洢��̬��Ļ�ռ䷴�����ȾĿ��
	ID3D11ShaderResourceView *blur_reflect_tex;           //�洢��̬��Ļ�ռ䷴���������Դ

	ID3D11RenderTargetView   *blur_reflect_target2;        //�洢��̬��Ļ�ռ䷴�����ȾĿ��
	ID3D11ShaderResourceView *blur_reflect_tex2;           //�洢��̬��Ļ�ռ䷴���������Դ

	ID3D11RenderTargetView   *final_reflect_target;       //�洢��̬��Ļ�ռ䷴�����ȾĿ��
	ID3D11ShaderResourceView *final_reflect_tex;          //�洢��̬��Ļ�ռ䷴���������Դ

	D3D11_VIEWPORT           render_viewport;             //�ӿ���Ϣ
	D3D11_VIEWPORT           half_render_viewport;        //�ӿ���Ϣ

	XMFLOAT4X4               static_cube_view_matrix[6];  //������ͼ�����������ȡ���任

	float  width_static_cube;
public:
	render_posttreatment_RTGR();
	void update_windowsize(int wind_width_need, int wind_height_need);
	engine_basic::engine_fail_reason create();
	void draw_reflect(XMFLOAT3 center_position, ID3D11RenderTargetView *rendertarget_input, ID3D11RenderTargetView *mask_target_input, ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex, ID3D11ShaderResourceView *reflect_cube_SRV, ID3D11ShaderResourceView *reflect_cubestencil_SRV);
	void release();
private:
	engine_basic::engine_fail_reason build_texture();
	void release_texture();
	void build_reflect_map(XMFLOAT3 center_position, ID3D11RenderTargetView *rendertarget_input, ID3D11RenderTargetView *mask_target_input, ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex, ID3D11ShaderResourceView *reflect_cube_SRV, ID3D11ShaderResourceView *reflect_cubestencil_SRV);
	void blur_map(ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex);
	void basic_blur(ID3D11ShaderResourceView *normaldepth_tex, ID3D11ShaderResourceView *depth_tex, ID3D11ShaderResourceView *mask, ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz);
	void draw_to_posttarget();
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
/*
class render_posttreatment_HDR
{
	//ȫ���ı���
	int                         width, height;       //��Ļ����
													 //�ڲ�����
	ID3D11UnorderedAccessView   *UAV_HDR_mid;        //HDR�Ļ������������м����
	ID3D11UnorderedAccessView   *UAV_HDR_final;      //HDR�Ļ����������ڴ洢���
	ID3D11ShaderResourceView    *SRV_HDR_map;        //HDR�Ļ����������ڴ洢map���
	ID3D11ShaderResourceView    *SRV_HDR_use;        //HDR���벿�֣�Ҫ����Ļ����ת���ɷǿ���ݵ�����

	ID3D11ShaderResourceView    *SRV_HDR_save;       //HDR�߹�洢������Ⱦ��Դ�����߹���д洢��
	ID3D11RenderTargetView      *RTV_HDR_save;       //HDR�߹�洢������ȾĿ�꣬���߹���д洢��

	ID3D11ShaderResourceView    *SRV_HDR_blur1;       //HDR�߹�ģ����Ⱦ��Դ��
	ID3D11RenderTargetView      *RTV_HDR_blur1;       //HDR�߹�ģ����ȾĿ�ꡣ
	ID3D11ShaderResourceView    *SRV_HDR_blur2;       //HDR�߹�ģ����Ⱦ��Դ��
	ID3D11RenderTargetView      *RTV_HDR_blur2;       //HDR�߹�ģ����ȾĿ�ꡣ

	D3D11_VIEWPORT              render_viewport;      //�ӿ���Ϣ
	ID3D11Buffer*               CPU_read_buffer;
	float                       average_light;
	float                       average_light_last;
	int width_rec, height_rec, buffer_num, map_num;
public:
	render_posttreatment_HDR(int width_need, int height_need);
	HRESULT create();
	void release();
	HRESULT display();
private:
	HRESULT init_buffer();
	HRESULT init_texture();
	HRESULT CreateCPUaccessBuf(int size_need);
	HRESULT build_fullscreen_picturebuff();
	void basic_blur(ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz);
	HRESULT count_average_light();
	HRESULT build_preblur_map();
	HRESULT blur_map();
	HRESULT HDR_map();

};
*/
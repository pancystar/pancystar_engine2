#pragma once
#include"pancystar_engine_basic.h"
using namespace DirectX;
#define window_width 1200
#define window_hight 800
class d3d_pancy_basic_singleton : public engine_basic::window_size_subject
{
	HWND wind_hwnd;
	UINT wind_width;
	UINT wind_height;
	UINT check_4x_msaa;
	ID3D11Device            *device_pancy;       //d3d设备
	ID3D11DeviceContext     *contex_pancy;       //设备描述表
	ID3D11RenderTargetView  *RTV_back_buffer;     //视图变量
	ID3D11DepthStencilView  *DSV_gbuffer;         //缓冲区信息
	D3D11_VIEWPORT          viewPort;             //视口信息
	IDXGISwapChain          *swapchain;          //交换链信息
	POINT point;
private:
	d3d_pancy_basic_singleton(HWND wind_hwnd_need, UINT wind_width_need, UINT wind_hight_need);
public:
	void attach(engine_basic::window_size_observer* observer_input);
	void detach(engine_basic::window_size_observer* observer_input);
	void notify(int wind_width_need, int wind_height_need);
	static d3d_pancy_basic_singleton *d3dbasic_pInstance;
	static engine_basic::engine_fail_reason single_create(HWND wind_hwnd, UINT wind_width, UINT wind_hight)
	{
		if (d3dbasic_pInstance != NULL)
		{
			engine_basic::engine_fail_reason failed_reason("the d3d basic instance have been created before");
			return failed_reason;
		}
		else
		{
			d3dbasic_pInstance = new d3d_pancy_basic_singleton(wind_hwnd, wind_width, wind_hight);
			engine_basic::engine_fail_reason check_failed = d3dbasic_pInstance->init(wind_hwnd, wind_width, wind_hight);
			return check_failed;
		}
	}
	static d3d_pancy_basic_singleton * GetInstance()
	{
		return d3dbasic_pInstance;
	}
	void release();
	POINT update_mouse();
	engine_basic::engine_fail_reason change_size(int width_need,int height_need);
	engine_basic::engine_fail_reason init(HWND wind_hwnd, UINT wind_width_need, UINT wind_height_need);
	ID3D11Device *get_d3d11_device() { return device_pancy; };
	ID3D11DeviceContext *get_d3d11_contex() { return contex_pancy; };
	engine_basic::engine_fail_reason set_render_target(ID3D11RenderTargetView  *render_target, ID3D11DepthStencilView *depthstencil_target);
	engine_basic::engine_fail_reason set_render_target(ID3D11RenderTargetView  *render_target);
	engine_basic::engine_fail_reason restore_render_target();
	engine_basic::engine_fail_reason clear_basicrender_target();
	engine_basic::engine_fail_reason clear_basicrender_target(D3D11_VIEWPORT viewPort_in);
	engine_basic::engine_fail_reason end_draw();
	engine_basic::engine_fail_reason save_texture(ID3D11Resource *resource_in,std::string name_file,int array_count = 0);
	IDXGISwapChain          *get_swapchain() { return swapchain; };
private:
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


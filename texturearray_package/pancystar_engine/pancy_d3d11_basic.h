#pragma once
#include"pancystar_engine_basic.h"
using namespace DirectX;
#define window_width 800
#define window_hight 600
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
	engine_basic::engine_fail_reason change_size(int width_need,int height_need);
	engine_basic::engine_fail_reason init(HWND wind_hwnd, UINT wind_width_need, UINT wind_height_need);
	ID3D11Device *get_d3d11_device() { return device_pancy; };
	ID3D11DeviceContext *get_d3d11_contex() { return contex_pancy; };
	engine_basic::engine_fail_reason set_render_target(ID3D11RenderTargetView  *render_target, ID3D11DepthStencilView *depthstencil_target);
	engine_basic::engine_fail_reason set_render_target(ID3D11RenderTargetView  *render_target);
	engine_basic::engine_fail_reason set_render_target(ID3D11RenderTargetView  **render_target,int size);
	ID3D11RenderTargetView *get_back_buffer() { return RTV_back_buffer; };
	engine_basic::engine_fail_reason restore_render_target();
	engine_basic::engine_fail_reason clear_basicrender_target();
	engine_basic::engine_fail_reason end_draw();
	IDXGISwapChain          *get_swapchain() { return swapchain; };
	UINT get_wind_width() { return wind_width; };
	UINT get_wind_height() { return wind_height; };
	void set_viewport(D3D11_VIEWPORT viewport_in) { contex_pancy->RSSetViewports(1, &viewport_in); };
	void reset_viewport() { contex_pancy->RSSetViewports(1, &viewPort); };;
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


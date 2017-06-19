#include"pancy_d3d11_basic.h"
d3d_pancy_basic_singleton *d3d_pancy_basic_singleton::d3dbasic_pInstance = NULL;
void d3d_pancy_basic_singleton::attach(engine_basic::window_size_observer* observer_input)
{
	observer_list.push_back(observer_input);
}
void d3d_pancy_basic_singleton::detach(engine_basic::window_size_observer* observer_input)
{
	for (auto data = observer_list.begin(); data != observer_list.end(); ++data)
	{
		if ((*data) == observer_input)
		{
			observer_list.erase(data);
		}
	}
}
void d3d_pancy_basic_singleton::notify(int wind_width_need, int wind_height_need)
{
	for (auto data = observer_list.begin(); data != observer_list.end(); ++data)
	{
		(*data)->update_windowsize(wind_width_need, wind_height_need);
	}
}
d3d_pancy_basic_singleton::d3d_pancy_basic_singleton(HWND wind_hwnd_need, UINT wind_width_need, UINT wind_hight_need)
{
	device_pancy = NULL;
	contex_pancy = NULL;       //设备描述表
	swapchain = NULL;          //交换链信息	
	RTV_back_buffer = NULL;     //视图变量
	DSV_gbuffer = NULL;         //缓冲区信息
	wind_hwnd = wind_hwnd_need;
	wind_width = wind_width_need;
	wind_height = wind_hight_need;
}
engine_basic::engine_fail_reason d3d_pancy_basic_singleton::init(HWND wind_hwnd, UINT wind_width_need, UINT wind_height_need)
{
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &device_pancy, &featureLevel, &contex_pancy);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_reason(hr, "D3D11CreateDevice Failed.");
		return failed_reason;
	}
	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		engine_basic::engine_fail_reason failed_reason(hr, "Direct3D Feature Level 11 unsupported.");
		return failed_reason;
	}
	//是否支持四倍抗锯齿
	hr = device_pancy->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &check_4x_msaa);
	if (check_4x_msaa <= 0)
	{
		engine_basic::engine_fail_reason failed_reason("4x msaa unsupported");
		return failed_reason;
	}
	//创建交换链
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = wind_width_need;
	sd.BufferDesc.Height = wind_height_need;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//是否开启四倍抗锯齿
	if (check_4x_msaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = check_4x_msaa - 1;
	}
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = wind_hwnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	IDXGIDevice *pDxgiDevice(NULL);
	hr = device_pancy->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDxgiDevice));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_reason(hr, "D3D11Create IDXGIDevice Failed.");
		return failed_reason;
	}
	IDXGIAdapter *pDxgiAdapter(NULL);
	hr = pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDxgiAdapter));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_reason(hr, "D3D11Create IDXGIAdapter Failed.");
		return failed_reason;
	}
	IDXGIFactory *pDxgiFactory(NULL);
	hr = pDxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pDxgiFactory));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_reason(hr, "D3D11Create IDXGIFactory Failed.");
		return failed_reason;
	}
	hr = pDxgiFactory->CreateSwapChain(device_pancy, &sd, &swapchain);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_reason(hr, "D3D11Create SwapChain Failed.");
		return failed_reason;
	}
	//释放接口  
	pDxgiFactory->Release();
	pDxgiAdapter->Release();
	pDxgiDevice->Release();
	engine_basic::engine_fail_reason check_failed = change_size(wind_width_need, wind_height_need);
	//engine_basic::engine_fail_reason succeed;
	return check_failed;
}
engine_basic::engine_fail_reason d3d_pancy_basic_singleton::change_size(int width_need, int height_need)
{
	wind_width = width_need;
	wind_height = height_need;
	//更新交换链的大小
	HRESULT hr = swapchain->ResizeBuffers(1, wind_width, wind_height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_reason(hr, "change swapchain size error");
		return failed_reason;
	}
	ID3D11Texture2D *backBuffer = NULL;
	//获取后缓冲区地址 
	hr;
	hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_reason(hr, "get swapchain buffer error");
		return failed_reason;
	}
	//更新后台缓冲区和基本深度缓冲区的大小
	safe_release(RTV_back_buffer);
	safe_release(DSV_gbuffer);
	hr = device_pancy->CreateRenderTargetView(backBuffer, 0, &RTV_back_buffer);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason failed_reason(hr, "create back buffer error");
		return failed_reason;
	}
	//释放后缓冲区引用  
	backBuffer->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~创建深度及模板缓冲区
	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.Width = wind_width;
	dsDesc.Height = wind_height;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	if (check_4x_msaa > 0)
	{
		dsDesc.SampleDesc.Count = 4;
		dsDesc.SampleDesc.Quality = check_4x_msaa - 1;
	}
	else
	{
		dsDesc.SampleDesc.Count = 1;
		dsDesc.SampleDesc.Quality = 0;
	}
	ID3D11Texture2D* depthStencilBuffer;
	device_pancy->CreateTexture2D(&dsDesc, 0, &depthStencilBuffer);
	device_pancy->CreateDepthStencilView(depthStencilBuffer, 0, &DSV_gbuffer);
	depthStencilBuffer->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~绑定视图信息到渲染管线
	contex_pancy->OMSetRenderTargets(1, &RTV_back_buffer, DSV_gbuffer);
	//修改视口大小
	viewPort.Width = static_cast<FLOAT>(wind_width);
	viewPort.Height = static_cast<FLOAT>(wind_height);
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	contex_pancy->RSSetViewports(1, &viewPort);
	//通知观察者修改窗口大小
	notify(wind_width, wind_height);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason d3d_pancy_basic_singleton::set_render_target(ID3D11RenderTargetView  *render_target, ID3D11DepthStencilView *depthstencil_target)
{
	if (render_target == NULL && depthstencil_target == NULL) 
	{
		engine_basic::engine_fail_reason failed_reason("set rendertarget error both RTV & DSV are empty");
		return failed_reason;
	}

	contex_pancy->OMSetRenderTargets(1, &render_target, depthstencil_target);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason d3d_pancy_basic_singleton::set_render_target(ID3D11RenderTargetView  *render_target)
{
	if (render_target == NULL)
	{
		engine_basic::engine_fail_reason failed_reason("set rendertarget error RTV is empty & DSV is default");
		return failed_reason;
	}
	contex_pancy->OMSetRenderTargets(1, &render_target, DSV_gbuffer);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason d3d_pancy_basic_singleton::set_render_target(ID3D11RenderTargetView  **render_target, int size)
{
	if (render_target == NULL)
	{
		engine_basic::engine_fail_reason failed_reason("set rendertarget error RTV is empty & DSV is default");
		return failed_reason;
	}
	contex_pancy->OMSetRenderTargets(size, render_target, DSV_gbuffer);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason d3d_pancy_basic_singleton::restore_render_target()
{
	contex_pancy->OMSetRenderTargets(1, &RTV_back_buffer, DSV_gbuffer);
	contex_pancy->RSSetViewports(1, &viewPort);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason d3d_pancy_basic_singleton::clear_basicrender_target() 
{
	//初始化
	XMVECTORF32 color = { 0.0f,0.0f,0.0f,1.0f };
	contex_pancy->ClearRenderTargetView(RTV_back_buffer, reinterpret_cast<float*>(&color));
	contex_pancy->ClearDepthStencilView(DSV_gbuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason d3d_pancy_basic_singleton::end_draw() 
{
	HRESULT hr = swapchain->Present(0, 0);
	if (FAILED(hr)) 
	{
		engine_basic::engine_fail_reason error_message(hr,"swap the backbuffer to screen error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void d3d_pancy_basic_singleton::release()
{
	if (d3dbasic_pInstance != NULL)
	{
		safe_release(RTV_back_buffer);
		safe_release(DSV_gbuffer);
		safe_release(swapchain);
		safe_release(contex_pancy);
		//swapchain->Release();
		//contex_pancy->Release();
#if defined(DEBUG) || defined(_DEBUG)
		ID3D11Debug *d3dDebug;
		HRESULT hr = device_pancy->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
		if (SUCCEEDED(hr))
		{
			hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		}
		if (d3dDebug != nullptr)            d3dDebug->Release();
#endif
		if (device_pancy != nullptr)            device_pancy->Release();
	}
}


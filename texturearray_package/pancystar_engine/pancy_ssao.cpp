#include"pancy_ssao.h"
ssao_pancy::ssao_pancy(int width, int height)
{
	map_width = width;
	map_height = height;
}
void ssao_pancy::update_windowsize(int wind_width_need, int wind_height_need)
{
	map_width = wind_width_need;
	map_height = wind_height_need;
	release_texture();
	build_texture();
}
engine_basic::engine_fail_reason ssao_pancy::basic_create()
{
	//创建全屏幕平面
	fullscreen_buffer = new mesh_aosquare(false);
	engine_basic::engine_fail_reason check_error = fullscreen_buffer->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	build_offset_vector();
	//创建纹理
	check_error = build_randomtex();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = build_texture();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void ssao_pancy::compute_ssaomap()
{
	D3D11_VIEWPORT           render_viewport;      //视口信息
	render_viewport.TopLeftX = 0.0f;
	render_viewport.TopLeftY = 0.0f;
	render_viewport.Width = static_cast<float>(map_width / 2.0f);
	render_viewport.Height = static_cast<float>(map_height / 2.0f);
	render_viewport.MinDepth = 0.0f;
	render_viewport.MaxDepth = 1.0f;
	d3d_pancy_basic_singleton::GetInstance()->set_viewport(render_viewport);
	//绑定渲染目标纹理，不设置深度模缓冲区因为这里不需要
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(ambient_target, NULL);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(ambient_target, clearColor);
	//设置渲染状态
	static const XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX P = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	XMFLOAT4X4 PT;
	XMStoreFloat4x4(&PT, P*T);
	engine_basic::engine_fail_reason check_error;
	auto shader_aopass = shader_control::GetInstance()->get_shader_ssaodraw(check_error);

	shader_aopass->set_ViewToTexSpace(&PT);
	shader_aopass->set_OffsetVectors(random_Offsets);

	XMFLOAT4 FrustumFarCorner[4];  //投影视截体的远截面的四个角点
	engine_basic::perspective_message::get_instance()->get_FrustumFarCorner(FrustumFarCorner);
	shader_aopass->set_FrustumCorners(FrustumFarCorner);
	shader_aopass->set_NormalDepthtex(normaldepth_tex);
	shader_aopass->set_Depthtex(depth_tex);
	shader_aopass->set_randomtex(randomtex);

	ID3DX11EffectTechnique* tech;
	//选定绘制路径
	shader_aopass->get_technique(&tech, "draw_ssaomap");
	fullscreen_buffer->get_teque(tech);
	fullscreen_buffer->show_mesh();

	shader_aopass->set_NormalDepthtex(NULL);
	shader_aopass->set_Depthtex(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, NULL_target, NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
	d3d_pancy_basic_singleton::GetInstance()->reset_viewport();
}
ID3D11ShaderResourceView* ssao_pancy::get_aomap()
{
	return ambient_tex_blur2;
}
void ssao_pancy::get_normaldepthmap(ID3D11ShaderResourceView *normalspec_need, ID3D11ShaderResourceView *depth_need)
{
	normaldepth_tex = normalspec_need;
	depth_tex = depth_need;
}
engine_basic::engine_fail_reason ssao_pancy::build_texture()
{
	HRESULT hr;
	//作为shader resource view的普通纹理(无多重采样)
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = map_width;
	texDesc.Height = map_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~全屏幕纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11Texture2D* ambientTex1 = 0;
	ID3D11Texture2D* ambientTex2 = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &ambientTex1);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ambient blurmap1 texture error in ssao part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(ambientTex1, 0, &ambient_tex_blur1);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ambient blurmap1 texture resource view error in ssao part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(ambientTex1, 0, &ambient_target_blur1);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ambient blurmap1 texture render target view error in ssao part");
		return error_message;
	}

	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &ambientTex2);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ambient blurmap2 texture error in ssao part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(ambientTex2, 0, &ambient_tex_blur2);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ambient blurmap2 texture resource view error in ssao part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(ambientTex2, 0, &ambient_target_blur2);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ambient blurmap2 texture render target view error in ssao part");
		return error_message;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~半屏幕纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.Width = static_cast<UINT>(static_cast<float>(map_width) / 2.0f);
	texDesc.Height = static_cast<UINT>(static_cast<float>(map_height) / 2.0f);
	texDesc.Format = DXGI_FORMAT_R16_FLOAT;
	ID3D11Texture2D* ambientTex0 = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, 0, &ambientTex0);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ambient map texture1 error in ssao part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(ambientTex0, 0, &ambient_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ambient map tex resource view error in ssao part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateRenderTargetView(ambientTex0, 0, &ambient_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ambient map render target view error in ssao part");
		return error_message;
	}

	ambientTex0->Release();
	ambientTex1->Release();
	ambientTex2->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void ssao_pancy::build_offset_vector()
{
	random_Offsets[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	random_Offsets[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	random_Offsets[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	random_Offsets[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	random_Offsets[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	random_Offsets[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	random_Offsets[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	random_Offsets[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	random_Offsets[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	random_Offsets[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	random_Offsets[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	random_Offsets[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	random_Offsets[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	random_Offsets[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < 14; ++i)
	{
		float s = 0.25f + 0.75f * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&random_Offsets[i]));
		XMStoreFloat4(&random_Offsets[i], v);
	}
}
engine_basic::engine_fail_reason ssao_pancy::build_randomtex()
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.SysMemPitch = 256 * 4 * sizeof(char);
	unsigned char* color;
	color = (unsigned char*)malloc(256 * 256 * 4 * sizeof(unsigned char));
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				color[i * 256 * 4 + j * 4 + k] = rand() % 256;
			}
			color[i * 256 * 4 + j * 4 + 3] = 0;
		}
	}
	initData.pSysMem = color;

	HRESULT hr;
	ID3D11Texture2D* tex = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, &initData, &tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create random texture error in ssao part");
		return error_message;
	}
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(tex, 0, &randomtex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create random texture resource view error in ssao part");
		return error_message;
	}
	tex->Release();
	free(color);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void ssao_pancy::blur_ssaomap()
{
	basic_blur(ambient_tex, ambient_target_blur1, true);
	basic_blur(ambient_tex_blur1, ambient_target_blur2, false);

	
	static const XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);
	XMFLOAT4X4 view_mat;
	pancy_camera::get_instance()->count_view_matrix(&view_mat);
	XMMATRIX V = XMLoadFloat4x4(&view_mat);
	XMMATRIX P = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_matrix());
	XMFLOAT4X4 VPT;
	XMStoreFloat4x4(&VPT, V * P * T);
	
	
	engine_basic::engine_fail_reason check_error;
	auto shader_need = shader_control::GetInstance()->get_shader_virtual_light(check_error);
	auto shader_deffered = shader_control::GetInstance()->get_shader_lightdeffered(check_error);
	auto shader_sky = shader_control::GetInstance()->get_shader_sky_draw(check_error);
	shader_sky->set_trans_texproj(&VPT);
	shader_need->set_ssaotex(ambient_tex_blur2);
	shader_deffered->set_ssaotex(ambient_tex_blur2);
	shader_need->set_trans_ssao(&VPT);
	shader_deffered->set_trans_ssao(&VPT);

}
void ssao_pancy::basic_blur(ID3D11ShaderResourceView *texin, ID3D11RenderTargetView *texout, bool if_row)
{

	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	ID3D11RenderTargetView* renderTargets[1] = { texout };
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(texout, NULL);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearRenderTargetView(texout, black);


	//contex_pancy->RSSetViewports(1, &render_viewport);
	engine_basic::engine_fail_reason check_error;
	auto shader_blurpass = shader_control::GetInstance()->get_shader_ssaoblur(check_error);

	shader_blurpass->set_image_size(1.0f / static_cast<float>(d3d_pancy_basic_singleton::GetInstance()->get_wind_width()), 1.0f / static_cast<float>(d3d_pancy_basic_singleton::GetInstance()->get_wind_height()));
	shader_blurpass->set_tex_resource(normaldepth_tex, texin);
	shader_blurpass->set_Depthtex(depth_tex);
	//选定绘制路径
	ID3DX11EffectTechnique* tech;
	if (if_row)
	{
		shader_blurpass->get_technique(&tech, "HorzBlur");
	}
	else
	{
		shader_blurpass->get_technique(&tech, "VertBlur");
	}
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	fullscreen_buffer->get_teque(tech);
	fullscreen_buffer->show_mesh();

	shader_blurpass->set_tex_resource(NULL, NULL);
	shader_blurpass->set_Depthtex(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
	}
}
void ssao_pancy::check_ssaomap()
{
	/*
	//contex_pancy->RSSetViewports(1, &render_viewport);
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	ID3D11RenderTargetView* renderTargets[1] = { ambient_target1 };
	auto *shader_blurpass = shader_list->get_shader_ssaoblur();
	shader_blurpass->set_image_size(1.0f / render_viewport.Width, 1.0f / render_viewport.Height);
	shader_blurpass->set_tex_resource(normaldepth_tex, ambient_tex0);
	UINT stride = sizeof(pancy_point);
	UINT offset = 0;

	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &AoMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(AoMap_IB, DXGI_FORMAT_R16_UINT, 0);

	ID3DX11EffectTechnique* tech;
	//选定绘制路径
	shader_blurpass->get_technique(&tech, "HorzBlur");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
	tech->GetPassByIndex(p)->Apply(0, contex_pancy);
	contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_blurpass->set_tex_resource(NULL, NULL);
	tech->GetPassByIndex(0)->Apply(0, contex_pancy);
	*/
}
void ssao_pancy::release_texture()
{
	safe_release(ambient_target);
	safe_release(ambient_tex);
	safe_release(ambient_target_blur1);
	safe_release(ambient_tex_blur1);
	safe_release(ambient_target_blur2);
	safe_release(ambient_tex_blur2);
}
void ssao_pancy::release()
{
	safe_release(randomtex);
	fullscreen_buffer->release();
	release_texture();
}

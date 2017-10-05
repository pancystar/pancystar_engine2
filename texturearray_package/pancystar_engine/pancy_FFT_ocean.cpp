#include"pancy_FFT_ocean.h"
CSFFT512x512_Plan::CSFFT512x512_Plan()
{
	pd3dImmediateContext = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex();
}
engine_basic::engine_fail_reason CSFFT512x512_Plan::create_cbuffers_512x512(UINT slices_in)
{
	HRESULT hr;
	// Create 6 cbuffers for 512x512 transform.

	D3D11_BUFFER_DESC cb_desc;
	cb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_desc.CPUAccessFlags = 0;
	cb_desc.MiscFlags = 0;
	cb_desc.ByteWidth = 32;//sizeof(float) * 5;
	cb_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA cb_data;
	cb_data.SysMemPitch = 0;
	cb_data.SysMemSlicePitch = 0;

	struct CB_Structure
	{
		UINT thread_count;
		UINT ostride;
		UINT istride;
		UINT pstride;
		float phase_base;
	};

	// Buffer 0
	const UINT thread_count = slices_in * (512 * 512) / 8;
	UINT ostride = 512 * 512 / 8;
	UINT istride = ostride;
	double phase_base = -TWO_PI / (512.0 * 512.0);

	CB_Structure cb_data_buf0 = { thread_count, ostride, istride, 512, (float)phase_base };
	cb_data.pSysMem = &cb_data_buf0;

	auto pd3dDevice = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device();
	hr = pd3dDevice->CreateBuffer(&cb_desc, &cb_data, &pRadix008A_CB[0]);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create FFT buffer[0] error");
		return error_message;
	}
	// Buffer 1
	istride /= 8;
	phase_base *= 8.0;

	CB_Structure cb_data_buf1 = { thread_count, ostride, istride, 512, (float)phase_base };
	cb_data.pSysMem = &cb_data_buf1;

	hr = pd3dDevice->CreateBuffer(&cb_desc, &cb_data, &pRadix008A_CB[1]);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create FFT buffer[1] error");
		return error_message;
	}

	// Buffer 2
	istride /= 8;
	phase_base *= 8.0;

	CB_Structure cb_data_buf2 = { thread_count, ostride, istride, 512, (float)phase_base };
	cb_data.pSysMem = &cb_data_buf2;

	hr = pd3dDevice->CreateBuffer(&cb_desc, &cb_data, &pRadix008A_CB[2]);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create FFT buffer[2] error");
		return error_message;
	}

	// Buffer 3
	istride /= 8;
	phase_base *= 8.0;
	ostride /= 512;

	CB_Structure cb_data_buf3 = { thread_count, ostride, istride, 1, (float)phase_base };
	cb_data.pSysMem = &cb_data_buf3;

	hr = pd3dDevice->CreateBuffer(&cb_desc, &cb_data, &pRadix008A_CB[3]);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create FFT buffer[3] error");
		return error_message;
	}

	// Buffer 4
	istride /= 8;
	phase_base *= 8.0;

	CB_Structure cb_data_buf4 = { thread_count, ostride, istride, 1, (float)phase_base };
	cb_data.pSysMem = &cb_data_buf4;

	hr = pd3dDevice->CreateBuffer(&cb_desc, &cb_data, &pRadix008A_CB[4]);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create FFT buffer[4] error");
		return error_message;
	}

	// Buffer 5
	istride /= 8;
	phase_base *= 8.0;

	CB_Structure cb_data_buf5 = { thread_count, ostride, istride, 1, (float)phase_base };
	cb_data.pSysMem = &cb_data_buf5;

	hr = pd3dDevice->CreateBuffer(&cb_desc, &cb_data, &pRadix008A_CB[5]);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create FFT buffer[5] error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason CSFFT512x512_Plan::create(UINT slices_in)
{
	HRESULT hr;
	slices = slices_in;
	auto pd3dDevice = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device();
	pd3dImmediateContext = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex();
	// Constants
	// Create 6 cbuffers for 512x512 transform
	auto check_error = create_cbuffers_512x512(slices);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}

	// Temp buffer
	D3D11_BUFFER_DESC buf_desc;
	buf_desc.ByteWidth = sizeof(float) * 2 * (512 * slices) * 512;
	buf_desc.Usage = D3D11_USAGE_DEFAULT;
	buf_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	buf_desc.CPUAccessFlags = 0;
	buf_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	buf_desc.StructureByteStride = sizeof(float) * 2;

	hr = pd3dDevice->CreateBuffer(&buf_desc, NULL, &pBuffer_Tmp);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create FFT Temp buffer error");
		return error_message;
	}

	// Temp undordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.NumElements = (512 * slices) * 512;
	uav_desc.Buffer.Flags = 0;

	hr = pd3dDevice->CreateUnorderedAccessView(pBuffer_Tmp, &uav_desc, &pUAV_Tmp);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create FFT Temp UAV error");
		return error_message;
	}

	// Temp shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = DXGI_FORMAT_UNKNOWN;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv_desc.Buffer.FirstElement = 0;
	srv_desc.Buffer.NumElements = (512 * slices) * 512;

	hr = pd3dDevice->CreateShaderResourceView(pBuffer_Tmp, &srv_desc, &pSRV_Tmp);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create FFT Temp SRV error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void CSFFT512x512_Plan::release()
{
	SAFE_RELEASE(pSRV_Tmp);
	SAFE_RELEASE(pUAV_Tmp);
	SAFE_RELEASE(pBuffer_Tmp);

	for (int i = 0; i < 6; i++)
		SAFE_RELEASE(pRadix008A_CB[i]);
}





OceanSimulator::OceanSimulator()
{
	// If the device becomes invalid at some point, delete current instance and generate a new one.
	m_pd3dDevice = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device();
	m_pd3dImmediateContext = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex();
	m_fft_plan = new CSFFT512x512_Plan();
}
engine_basic::engine_fail_reason OceanSimulator::createBufferAndUAV(ID3D11Device* pd3dDevice, void* data, UINT byte_width, UINT byte_stride, ID3D11Buffer** ppBuffer, ID3D11UnorderedAccessView** ppUAV, ID3D11ShaderResourceView** ppSRV)
{
	HRESULT hr;
	// Create buffer
	D3D11_BUFFER_DESC buf_desc;
	buf_desc.ByteWidth = byte_width;
	buf_desc.Usage = D3D11_USAGE_DEFAULT;
	buf_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	buf_desc.CPUAccessFlags = 0;
	buf_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	buf_desc.StructureByteStride = byte_stride;

	D3D11_SUBRESOURCE_DATA init_data = { data, 0, 0 };

	hr = pd3dDevice->CreateBuffer(&buf_desc, data != NULL ? &init_data : NULL, ppBuffer);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ocean simulate buffer error");
		return error_message;
	}

	// Create undordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.NumElements = byte_width / byte_stride;
	uav_desc.Buffer.Flags = 0;

	hr = pd3dDevice->CreateUnorderedAccessView(*ppBuffer, &uav_desc, ppUAV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ocean simulate buffer UAV error");
		return error_message;
	}

	// Create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = DXGI_FORMAT_UNKNOWN;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv_desc.Buffer.FirstElement = 0;
	srv_desc.Buffer.NumElements = byte_width / byte_stride;

	hr = pd3dDevice->CreateShaderResourceView(*ppBuffer, &srv_desc, ppSRV);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ocean simulate buffer SRV error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason OceanSimulator::createTextureAndViews(ID3D11Device* pd3dDevice, UINT width, UINT height, DXGI_FORMAT format, ID3D11Texture2D** ppTex, ID3D11ShaderResourceView** ppSRV, ID3D11RenderTargetView** ppRTV)
{
	HRESULT hr;
	// Create 2D texture
	D3D11_TEXTURE2D_DESC tex_desc;
	tex_desc.Width = width;
	tex_desc.Height = height;
	tex_desc.MipLevels = 0;
	tex_desc.ArraySize = 1;
	tex_desc.Format = format;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	hr = pd3dDevice->CreateTexture2D(&tex_desc, NULL, ppTex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ocean simulate texture2Ddata error");
		return error_message;
	}

	// Create shader resource view
	(*ppTex)->GetDesc(&tex_desc);
	if (ppSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		srv_desc.Format = format;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = tex_desc.MipLevels;
		srv_desc.Texture2D.MostDetailedMip = 0;

		hr = pd3dDevice->CreateShaderResourceView(*ppTex, &srv_desc, ppSRV);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "create ocean simulate texture SRV error");
			return error_message;
		}
	}

	// Create render target view
	if (ppRTV)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
		rtv_desc.Format = format;
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtv_desc.Texture2D.MipSlice = 0;

		hr = pd3dDevice->CreateRenderTargetView(*ppTex, &rtv_desc, ppRTV);
		if (FAILED(hr))
		{
			engine_basic::engine_fail_reason error_message(hr, "create ocean simulate texture RTV error");
			return error_message;
		}
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason OceanSimulator::create(OceanParameter& params)
{
	// Height map H(0)
	int height_map_size = (params.dmap_dim + 4) * (params.dmap_dim + 1);
	XMFLOAT2* h0_data = new XMFLOAT2[height_map_size * sizeof(XMFLOAT2)];
	float* omega_data = new float[height_map_size * sizeof(float)];
	initHeightMap(params, h0_data, omega_data);

	m_param = params;
	int hmap_dim = params.dmap_dim;
	int input_full_size = (hmap_dim + 4) * (hmap_dim + 1);
	// This value should be (hmap_dim / 2 + 1) * hmap_dim, but we use full sized buffer here for simplicity.
	int input_half_size = hmap_dim * hmap_dim;
	int output_size = hmap_dim * hmap_dim;

	// For filling the buffer with zeroes.
	char* zero_data = new char[3 * output_size * sizeof(float) * 2];
	memset(zero_data, 0, 3 * output_size * sizeof(float) * 2);

	// RW buffer allocations
	// H0
	UINT float2_stride = 2 * sizeof(float);
	auto check_error = createBufferAndUAV(m_pd3dDevice, h0_data, input_full_size * float2_stride, float2_stride, &m_pBuffer_Float2_H0, &m_pUAV_H0, &m_pSRV_H0);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	// Notice: The following 3 buffers should be half sized buffer because of conjugate symmetric input. But
	// we use full sized buffers due to the CS4.0 restriction.

	// Put H(t), Dx(t) and Dy(t) into one buffer because CS4.0 allows only 1 UAV at a time
	check_error = createBufferAndUAV(m_pd3dDevice, zero_data, 3 * input_half_size * float2_stride, float2_stride, &m_pBuffer_Float2_Ht, &m_pUAV_Ht, &m_pSRV_Ht);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	// omega
	check_error = createBufferAndUAV(m_pd3dDevice, omega_data, input_full_size * sizeof(float), sizeof(float), &m_pBuffer_Float_Omega, &m_pUAV_Omega, &m_pSRV_Omega);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	// Notice: The following 3 should be real number data. But here we use the complex numbers and C2C FFT
	// due to the CS4.0 restriction.
	// Put Dz, Dx and Dy into one buffer because CS4.0 allows only 1 UAV at a time
	check_error = createBufferAndUAV(m_pd3dDevice, zero_data, 3 * output_size * float2_stride, float2_stride, &m_pBuffer_Float_Dxyz, &m_pUAV_Dxyz, &m_pSRV_Dxyz);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	SAFE_DELETE_ARRAY(zero_data);
	SAFE_DELETE_ARRAY(h0_data);
	SAFE_DELETE_ARRAY(omega_data);

	// D3D11 Textures
	check_error = createTextureAndViews(m_pd3dDevice, hmap_dim, hmap_dim, DXGI_FORMAT_R32G32B32A32_FLOAT, &m_pDisplacementMap, &m_pDisplacementSRV, &m_pDisplacementRTV);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	check_error = createTextureAndViews(m_pd3dDevice, hmap_dim, hmap_dim, DXGI_FORMAT_R16G16B16A16_FLOAT, &m_pGradientMap, &m_pGradientSRV, &m_pGradientRTV);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	// Samplers
	D3D11_SAMPLER_DESC sam_desc;
	sam_desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.MipLODBias = 0;
	sam_desc.MaxAnisotropy = 1;
	sam_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sam_desc.BorderColor[0] = 1.0f;
	sam_desc.BorderColor[1] = 1.0f;
	sam_desc.BorderColor[2] = 1.0f;
	sam_desc.BorderColor[3] = 1.0f;
	sam_desc.MinLOD = -FLT_MAX;
	sam_desc.MaxLOD = FLT_MAX;
	HRESULT hr = m_pd3dDevice->CreateSamplerState(&sam_desc, &m_pPointSamplerState);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ocean simulate sampler state error");
		return error_message;
	}

	// Quad vertex buffer
	D3D11_BUFFER_DESC vb_desc;
	vb_desc.ByteWidth = 4 * sizeof(XMFLOAT4);
	vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vb_desc.CPUAccessFlags = 0;
	vb_desc.MiscFlags = 0;

	float quad_verts[] =
	{
		-1, -1, 0, 1,
		-1,  1, 0, 1,
		1, -1, 0, 1,
		1,  1, 0, 1,
	};
	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = &quad_verts[0];
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	hr = m_pd3dDevice->CreateBuffer(&vb_desc, &init_data, &m_pQuadVB);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ocean simulate point Buffer error");
		return error_message;
	}

	// Constant buffers
	UINT actual_dim = m_param.dmap_dim;
	UINT input_width = actual_dim + 4;
	// We use full sized data here. The value "output_width" should be actual_dim/2+1 though.
	UINT output_width = actual_dim;
	UINT output_height = actual_dim;
	UINT dtx_offset = actual_dim * actual_dim;
	UINT dty_offset = actual_dim * actual_dim * 2;
	UINT immutable_consts[] = { actual_dim, input_width, output_width, output_height, dtx_offset, dty_offset };
	D3D11_SUBRESOURCE_DATA init_cb0 = { &immutable_consts[0], 0, 0 };

	D3D11_BUFFER_DESC cb_desc;
	cb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_desc.CPUAccessFlags = 0;
	cb_desc.MiscFlags = 0;
	cb_desc.ByteWidth = PAD16(sizeof(immutable_consts));
	hr = m_pd3dDevice->CreateBuffer(&cb_desc, &init_cb0, &m_pImmutableCB);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ocean simulate Immutable Buffer error");
		return error_message;
	}

	ID3D11Buffer* cbs[1] = { m_pImmutableCB };
	m_pd3dImmediateContext->CSSetConstantBuffers(0, 1, cbs);
	m_pd3dImmediateContext->PSSetConstantBuffers(0, 1, cbs);

	cb_desc.Usage = D3D11_USAGE_DYNAMIC;
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb_desc.MiscFlags = 0;
	cb_desc.ByteWidth = PAD16(sizeof(float) * 3);
	hr = m_pd3dDevice->CreateBuffer(&cb_desc, NULL, &m_pPerFrameCB);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create ocean simulate PerFrame Buffer error");
		return error_message;
	}

	// FFT
	check_error = m_fft_plan->create(3);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}


#ifdef CS_DEBUG_BUFFER
	D3D11_BUFFER_DESC buf_desc;
	buf_desc.ByteWidth = 3 * input_half_size * float2_stride;
	buf_desc.Usage = D3D11_USAGE_STAGING;
	buf_desc.BindFlags = 0;
	buf_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	buf_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	buf_desc.StructureByteStride = float2_stride;

	m_pd3dDevice->CreateBuffer(&buf_desc, NULL, &m_pDebugBuffer);
	assert(m_pDebugBuffer);
#endif
	engine_basic::engine_fail_reason succeed;
	return succeed;
}


void OceanSimulator::release()
{
	m_fft_plan->release();

	SAFE_RELEASE(m_pBuffer_Float2_H0);
	SAFE_RELEASE(m_pBuffer_Float_Omega);
	SAFE_RELEASE(m_pBuffer_Float2_Ht);
	SAFE_RELEASE(m_pBuffer_Float_Dxyz);

	SAFE_RELEASE(m_pPointSamplerState);

	SAFE_RELEASE(m_pQuadVB);

	SAFE_RELEASE(m_pUAV_H0);
	SAFE_RELEASE(m_pUAV_Omega);
	SAFE_RELEASE(m_pUAV_Ht);
	SAFE_RELEASE(m_pUAV_Dxyz);

	SAFE_RELEASE(m_pSRV_H0);
	SAFE_RELEASE(m_pSRV_Omega);
	SAFE_RELEASE(m_pSRV_Ht);
	SAFE_RELEASE(m_pSRV_Dxyz);

	SAFE_RELEASE(m_pDisplacementMap);
	SAFE_RELEASE(m_pDisplacementSRV);
	SAFE_RELEASE(m_pDisplacementRTV);

	SAFE_RELEASE(m_pGradientMap);
	SAFE_RELEASE(m_pGradientSRV);
	SAFE_RELEASE(m_pGradientRTV);


	SAFE_RELEASE(m_pImmutableCB);
	SAFE_RELEASE(m_pPerFrameCB);

#ifdef CS_DEBUG_BUFFER
	SAFE_RELEASE(m_pDebugBuffer);
#endif
}


// Initialize the vector field.
// wlen_x: width of wave tile, in meters
// wlen_y: length of wave tile, in meters
float OceanSimulator::Gauss()
{
	float u1 = rand() / (float)RAND_MAX;
	float u2 = rand() / (float)RAND_MAX;
	if (u1 < 1e-6f)
		u1 = 1e-6f;
	return sqrtf(-2 * logf(u1)) * cosf(2 * XM_PI * u2);
}
float OceanSimulator::Phillips(XMFLOAT2 K, XMFLOAT2 W, float v, float a, float dir_depend)
{
	// largest possible wave from constant wind of velocity v
	float l = v * v / GRAV_ACCEL;
	// damp out waves with very small length w << l
	float w = l / 1000;

	float Ksqr = K.x * K.x + K.y * K.y;
	float Kcos = K.x * W.x + K.y * W.y;
	float phillips = a * expf(-1 / (l * l * Ksqr)) / (Ksqr * Ksqr * Ksqr) * (Kcos * Kcos);

	// filter out waves moving opposite to wind
	if (Kcos < 0)
		phillips *= dir_depend;

	// damp out waves with very small length w << l
	return phillips * expf(-Ksqr * w * w);
}
void OceanSimulator::initHeightMap(OceanParameter& params, XMFLOAT2* out_h0, float* out_omega)
{
	int i, j;
	XMFLOAT2 K, Kn;

	XMFLOAT2 wind_dir;
	XMVECTOR vec_dir_wind = XMLoadFloat2(&params.wind_dir);
	XMStoreFloat2(&wind_dir, XMVector2Normalize(vec_dir_wind)); ;
	//D3DXVec2Normalize(&wind_dir, &params.wind_dir);
	float a = params.wave_amplitude * 1e-7f;	// It is too small. We must scale it for editing.
	float v = params.wind_speed;
	float dir_depend = params.wind_dependency;

	int height_map_dim = params.dmap_dim;
	float patch_length = params.patch_length;

	// initialize random generator.
	srand(0);

	for (i = 0; i <= height_map_dim; i++)
	{
		// K is wave-vector, range [-|DX/W, |DX/W], [-|DY/H, |DY/H]
		K.y = (-height_map_dim / 2.0f + i) * (2 * XM_PI / patch_length);

		for (j = 0; j <= height_map_dim; j++)
		{
			K.x = (-height_map_dim / 2.0f + j) * (2 * XM_PI / patch_length);

			float phil = (K.x == 0 && K.y == 0) ? 0 : sqrtf(Phillips(K, wind_dir, v, a, dir_depend));

			out_h0[i * (height_map_dim + 4) + j].x = float(phil * Gauss() * HALF_SQRT_2);
			out_h0[i * (height_map_dim + 4) + j].y = float(phil * Gauss() * HALF_SQRT_2);

			// The angular frequency is following the dispersion relation:
			//            out_omega^2 = g*k
			// The equation of Gerstner wave:
			//            x = x0 - K/k * A * sin(dot(K, x0) - sqrt(g * k) * t), x is a 2D vector.
			//            z = A * cos(dot(K, x0) - sqrt(g * k) * t)
			// Gerstner wave shows that a point on a simple sinusoid wave is doing a uniform circular
			// motion with the center (x0, y0, z0), radius A, and the circular plane is parallel to
			// vector K.
			out_omega[i * (height_map_dim + 4) + j] = sqrtf(GRAV_ACCEL * sqrtf(K.x * K.x + K.y * K.y));
		}
	}
}
void OceanSimulator::radix008A(CSFFT512x512_Plan* fft_plan, ID3D11UnorderedAccessView* pUAV_Dst, ID3D11ShaderResourceView* pSRV_Src, UINT thread_count, UINT istride)
{
	// Setup execution configuration
	UINT grid = thread_count / COHERENCY_GRANULARITY;
	ID3D11DeviceContext* pd3dImmediateContext = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex();

	engine_basic::engine_fail_reason check_error;
	auto shader_fft = shader_control::GetInstance()->get_shader_compute_fft(check_error);
	shader_fft->set_shader_resource(pSRV_Src);
	shader_fft->set_compute_UAV(pUAV_Dst);
	/*
	// Buffers
	ID3D11ShaderResourceView* cs_srvs[1] = { pSRV_Src };
	pd3dImmediateContext->CSSetShaderResources(0, 1, cs_srvs);

	ID3D11UnorderedAccessView* cs_uavs[1] = { pUAV_Dst };
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, cs_uavs, (UINT*)(&cs_uavs[0]));
	// Shader
	if (istride > 1)
	pd3dImmediateContext->CSSetShader(fft_plan->pRadix008A_CS, NULL, 0);
	else
	pd3dImmediateContext->CSSetShader(fft_plan->pRadix008A_CS2, NULL, 0);
	*/
	// Execute
	//pd3dImmediateContext->Dispatch(grid, 1, 1);
	if (istride > 1)
	{
		shader_fft->dispatch(grid, "common_FFT");
	}
	else
	{
		shader_fft->dispatch(grid, "common_FFT2");
	}

	//shader_fft->set_shader_resource(NULL);
	//shader_fft->set_compute_UAV(NULL);
	/*
	// Unbind resource
	cs_srvs[0] = NULL;
	pd3dImmediateContext->CSSetShaderResources(0, 1, cs_srvs);

	cs_uavs[0] = NULL;
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, cs_uavs, (UINT*)(&cs_uavs[0]));
	*/
}
void OceanSimulator::fft_512x512_c2c(CSFFT512x512_Plan* fft_plan, ID3D11UnorderedAccessView* pUAV_Dst, ID3D11ShaderResourceView* pSRV_Dst, ID3D11ShaderResourceView* pSRV_Src)
{
	const UINT thread_count = fft_plan->slices * (512 * 512) / 8;
	ID3D11UnorderedAccessView* pUAV_Tmp = fft_plan->pUAV_Tmp;
	ID3D11ShaderResourceView* pSRV_Tmp = fft_plan->pSRV_Tmp;
	ID3D11DeviceContext* pd3dContext = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex();
	ID3D11Buffer* cs_cbs[1];

	engine_basic::engine_fail_reason check_error;
	auto shader_fft = shader_control::GetInstance()->get_shader_compute_fft(check_error);

	UINT istride = 512 * 512 / 8;
	cs_cbs[0] = fft_plan->pRadix008A_CB[0];
	shader_fft->set_Constant_Buffer(cs_cbs[0]);
	//pd3dContext->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	radix008A(fft_plan, pUAV_Tmp, pSRV_Src, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = fft_plan->pRadix008A_CB[1];
	shader_fft->set_Constant_Buffer(cs_cbs[0]);
	//pd3dContext->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	radix008A(fft_plan, pUAV_Dst, pSRV_Tmp, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = fft_plan->pRadix008A_CB[2];
	shader_fft->set_Constant_Buffer(cs_cbs[0]);
	//pd3dContext->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	radix008A(fft_plan, pUAV_Tmp, pSRV_Dst, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = fft_plan->pRadix008A_CB[3];
	shader_fft->set_Constant_Buffer(cs_cbs[0]);
	//pd3dContext->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	radix008A(fft_plan, pUAV_Dst, pSRV_Tmp, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = fft_plan->pRadix008A_CB[4];
	shader_fft->set_Constant_Buffer(cs_cbs[0]);
	//pd3dContext->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	radix008A(fft_plan, pUAV_Tmp, pSRV_Dst, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = fft_plan->pRadix008A_CB[5];
	shader_fft->set_Constant_Buffer(cs_cbs[0]);
	//pd3dContext->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	radix008A(fft_plan, pUAV_Dst, pSRV_Tmp, thread_count, istride);
}
void OceanSimulator::updateDisplacementMap(float time)
{
	// ---------------------------- H(0) -> H(t), D(x, t), D(y, t) --------------------------------
	// Compute shader
	engine_basic::engine_fail_reason check_error;
	auto shader_oceansimulate_cs = shader_control::GetInstance()->get_shader_oceansimulate_cs(check_error);
	/*
	m_pd3dImmediateContext->CSSetShader(m_pUpdateSpectrumCS, NULL, 0);

	// Buffers
	ID3D11ShaderResourceView* cs0_srvs[2] = { m_pSRV_H0, m_pSRV_Omega };
	m_pd3dImmediateContext->CSSetShaderResources(0, 2, cs0_srvs);

	ID3D11UnorderedAccessView* cs0_uavs[1] = { m_pUAV_Ht };
	m_pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, cs0_uavs, (UINT*)(&cs0_uavs[0]));
	*/
	shader_oceansimulate_cs->set_shader_resource_h0(m_pSRV_H0);
	shader_oceansimulate_cs->set_shader_resource_omega(m_pSRV_Omega);
	shader_oceansimulate_cs->set_compute_UAV(m_pUAV_Ht);
	// Consts
	D3D11_MAPPED_SUBRESOURCE mapped_res;
	m_pd3dImmediateContext->Map(m_pPerFrameCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
	assert(mapped_res.pData);
	float* per_frame_data = (float*)mapped_res.pData;
	// g_Time
	per_frame_data[0] = time * m_param.time_scale;
	// g_ChoppyScale
	per_frame_data[1] = m_param.choppy_scale;
	// g_GridLen
	per_frame_data[2] = m_param.dmap_dim / m_param.patch_length;
	m_pd3dImmediateContext->Unmap(m_pPerFrameCB, 0);
	//ID3D11Buffer* cs_cbs[2] = { m_pImmutableCB, m_pPerFrameCB };
	//m_pd3dImmediateContext->CSSetConstantBuffers(0, 2, cs_cbs);
	shader_oceansimulate_cs->set_Constant_Buffer_Immutable(m_pImmutableCB);
	shader_oceansimulate_cs->set_Constant_Buffer_ChangePerFrame(m_pPerFrameCB);
	// Run the CS
	UINT group_count_x = (m_param.dmap_dim + BLOCK_SIZE_X - 1) / BLOCK_SIZE_X;
	UINT group_count_y = (m_param.dmap_dim + BLOCK_SIZE_Y - 1) / BLOCK_SIZE_Y;
	//m_pd3dImmediateContext->Dispatch(group_count_x, group_count_y, 1);
	shader_oceansimulate_cs->dispatch(group_count_x, group_count_y);
	// Unbind resources for CS
	//cs0_uavs[0] = NULL;
	//m_pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, cs0_uavs, (UINT*)(&cs0_uavs[0]));
	//cs0_srvs[0] = NULL;
	//cs0_srvs[1] = NULL;
	//m_pd3dImmediateContext->CSSetShaderResources(0, 2, cs0_srvs);


	// ------------------------------------ Perform FFT -------------------------------------------
	fft_512x512_c2c(m_fft_plan, m_pUAV_Dxyz, m_pSRV_Dxyz, m_pSRV_Ht);

	// --------------------------------- Wrap Dx, Dy and Dz ---------------------------------------

	// Push RT
	ID3D11RenderTargetView* old_target;
	ID3D11DepthStencilView* old_depth;
	m_pd3dImmediateContext->OMGetRenderTargets(1, &old_target, &old_depth);
	D3D11_VIEWPORT old_viewport;
	UINT num_viewport = 1;
	m_pd3dImmediateContext->RSGetViewports(&num_viewport, &old_viewport);

	D3D11_VIEWPORT new_vp = { 0, 0, (float)m_param.dmap_dim, (float)m_param.dmap_dim, 0.0f, 1.0f };
	m_pd3dImmediateContext->RSSetViewports(1, &new_vp);


	auto shader_oceansimulate_vps = shader_control::GetInstance()->get_shader_oceansimulate_vps(check_error);


	// Set RT
	ID3D11RenderTargetView* rt_views[1] = { m_pDisplacementRTV };
	m_pd3dImmediateContext->OMSetRenderTargets(1, rt_views, NULL);

	/*
	// VS & PS
	m_pd3dImmediateContext->VSSetShader(m_pQuadVS, NULL, 0);
	m_pd3dImmediateContext->PSSetShader(m_pUpdateDisplacementPS, NULL, 0);

	// Constants
	ID3D11Buffer* ps_cbs[2] = { m_pImmutableCB, m_pPerFrameCB };
	m_pd3dImmediateContext->PSSetConstantBuffers(0, 2, ps_cbs);

	// Buffer resources
	ID3D11ShaderResourceView* ps_srvs[1] = { m_pSRV_Dxyz };
	m_pd3dImmediateContext->PSSetShaderResources(0, 1, ps_srvs);
	*/
	shader_oceansimulate_vps->set_shader_resource_buffer(m_pSRV_Dxyz);
	shader_oceansimulate_vps->set_Constant_Buffer_Immutable(m_pImmutableCB);
	shader_oceansimulate_vps->set_Constant_Buffer_ChangePerFrame(m_pPerFrameCB);

	// IA setup
	ID3D11Buffer* vbs[1] = { m_pQuadVB };
	UINT strides[1] = { sizeof(XMFLOAT4) };
	UINT offsets[1] = { 0 };
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);
	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	/*
	// Perform draw call
	m_pd3dImmediateContext->Draw(4, 0);
	*/
	//选定绘制路径

	ID3DX11EffectTechnique* teque_need;
	shader_oceansimulate_vps->get_technique(&teque_need, "Ocean_UpdateDisplacement");
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		teque_need->GetPassByIndex(i)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Draw(4, 0);
	}
	// Unbind
	ID3D11ShaderResourceView* ps_srvs[1] = { NULL };
	m_pd3dImmediateContext->PSSetShaderResources(0, 1, ps_srvs);


	// ----------------------------------- Generate Normal ----------------------------------------

	// Set RT
	rt_views[0] = m_pGradientRTV;
	m_pd3dImmediateContext->OMSetRenderTargets(1, rt_views, NULL);
	/*
	// VS & PS
	m_pd3dImmediateContext->VSSetShader(m_pQuadVS, NULL, 0);
	m_pd3dImmediateContext->PSSetShader(m_pGenGradientFoldingPS, NULL, 0);

	// Texture resource and sampler
	ps_srvs[0] = m_pDisplacementSRV;
	m_pd3dImmediateContext->PSSetShaderResources(0, 1, ps_srvs);

	ID3D11SamplerState* samplers[1] = { m_pPointSamplerState };
	m_pd3dImmediateContext->PSSetSamplers(0, 1, &samplers[0]);
	*/
	shader_oceansimulate_vps->set_shader_resource_texture(m_pDisplacementSRV);
	// Perform draw call
	//m_pd3dImmediateContext->Draw(4, 0);
	shader_oceansimulate_vps->get_technique(&teque_need, "Ocean_GradientFolding");
	teque_need->GetDesc(&techDesc);
	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		teque_need->GetPassByIndex(i)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Draw(4, 0);
	}
	// Unbind
	ps_srvs[0] = NULL;
	m_pd3dImmediateContext->PSSetShaderResources(0, 1, ps_srvs);

	// Pop RT
	m_pd3dImmediateContext->RSSetViewports(1, &old_viewport);
	m_pd3dImmediateContext->OMSetRenderTargets(1, &old_target, old_depth);
	SAFE_RELEASE(old_target);
	SAFE_RELEASE(old_depth);

	m_pd3dImmediateContext->GenerateMips(m_pGradientSRV);

	// Define CS_DEBUG_BUFFER to enable writing a buffer into a file.
#ifdef CS_DEBUG_BUFFER
	{
		m_pd3dImmediateContext->CopyResource(m_pDebugBuffer, m_pBuffer_Float_Dxyz);
		D3D11_MAPPED_SUBRESOURCE mapped_res;
		m_pd3dImmediateContext->Map(m_pDebugBuffer, 0, D3D11_MAP_READ, 0, &mapped_res);

		// set a break point below, and drag MappedResource.pData into in your Watch window
		// and cast it as (float*)

		// Write to disk
		D3DXVECTOR2* v = (D3DXVECTOR2*)mapped_res.pData;

		FILE* fp = fopen(".\\tmp\\Ht_raw.dat", "wb");
		fwrite(v, 512 * 512 * sizeof(float) * 2 * 3, 1, fp);
		fclose(fp);

		m_pd3dImmediateContext->Unmap(m_pDebugBuffer, 0);
	}
#endif
}
ID3D11ShaderResourceView* OceanSimulator::getD3D11DisplacementMap()
{
	return m_pDisplacementSRV;
}
ID3D11ShaderResourceView* OceanSimulator::getD3D11GradientMap()
{
	return m_pGradientSRV;
}
const OceanParameter& OceanSimulator::getParameters()
{
	return m_param;
}



FFT_ocean::FFT_ocean() 
{
}

engine_basic::engine_fail_reason FFT_ocean::create(const OceanParameter& ocean_param, ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	engine_basic::engine_fail_reason check_error;
	HRESULT hr;
	//创建水面范围网格
	fullscreen_buffer = new mesh_multisquare_tessellation(false, 25);
	check_error = fullscreen_buffer->create_object();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	g_PatchLength = ocean_param.patch_length;
	g_DisplaceMapDim = ocean_param.dmap_dim;
	g_WindDir = ocean_param.wind_dir;

	// D3D buffers
	check_error = createFresnelMap(pd3dDevice);
	if (!check_error.check_if_failed()) 
	{
		return check_error;
	}
	check_error = loadTextures(pd3dDevice);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	/*
	// HLSL
	// Vertex & pixel shaders
	ID3DBlob* pBlobOceanSurfVS = NULL;
	ID3DBlob* pBlobOceanSurfPS = NULL;
	ID3DBlob* pBlobWireframePS = NULL;

	CompileShaderFromFile(L"ocean_shading.hlsl", "OceanSurfVS", "vs_4_0", &pBlobOceanSurfVS);
	CompileShaderFromFile(L"ocean_shading.hlsl", "OceanSurfPS", "ps_4_0", &pBlobOceanSurfPS);
	CompileShaderFromFile(L"ocean_shading.hlsl", "WireframePS", "ps_4_0", &pBlobWireframePS);
	assert(pBlobOceanSurfVS);
	assert(pBlobOceanSurfPS);
	assert(pBlobWireframePS);

	pd3dDevice->CreateVertexShader(pBlobOceanSurfVS->GetBufferPointer(), pBlobOceanSurfVS->GetBufferSize(), NULL, &g_pOceanSurfVS);
	pd3dDevice->CreatePixelShader(pBlobOceanSurfPS->GetBufferPointer(), pBlobOceanSurfPS->GetBufferSize(), NULL, &g_pOceanSurfPS);
	pd3dDevice->CreatePixelShader(pBlobWireframePS->GetBufferPointer(), pBlobWireframePS->GetBufferSize(), NULL, &g_pWireframePS);
	assert(g_pOceanSurfVS);
	assert(g_pOceanSurfPS);
	assert(g_pWireframePS);
	SAFE_RELEASE(pBlobOceanSurfPS);
	SAFE_RELEASE(pBlobWireframePS);

	// Input layout
	D3D11_INPUT_ELEMENT_DESC mesh_layout_desc[] =
	{
	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	pd3dDevice->CreateInputLayout(mesh_layout_desc, 1, pBlobOceanSurfVS->GetBufferPointer(), pBlobOceanSurfVS->GetBufferSize(), &g_pMeshLayout);
	assert(g_pMeshLayout);

	SAFE_RELEASE(pBlobOceanSurfVS);
	*/
	// Constants
	D3D11_BUFFER_DESC cb_desc;
	cb_desc.Usage = D3D11_USAGE_DYNAMIC;
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb_desc.MiscFlags = 0;
	cb_desc.ByteWidth = PAD16(sizeof(Const_Per_Call));
	cb_desc.StructureByteStride = 0;
	hr = pd3dDevice->CreateBuffer(&cb_desc, NULL, &g_pPerCallCB);
	if (FAILED(hr)) 
	{
		engine_basic::engine_fail_reason error_message(hr,"create water constant buffer error");
	}

	Const_Shading shading_data;
	// Grid side length * 2
	shading_data.g_TexelLength_x2 = g_PatchLength / g_DisplaceMapDim * 2;;
	// Color
	shading_data.g_SkyColor = g_SkyColor;
	shading_data.g_WaterbodyColor = g_WaterbodyColor;
	// Texcoord
	shading_data.g_UVScale = 1.0f / g_PatchLength;
	shading_data.g_UVOffset = 0.5f / g_DisplaceMapDim;
	// Perlin
	shading_data.g_PerlinSize = g_PerlinSize;
	shading_data.g_PerlinAmplitude = g_PerlinAmplitude;
	shading_data.g_PerlinGradient = g_PerlinGradient;
	shading_data.g_PerlinOctave = g_PerlinOctave;
	// Multiple reflection workaround
	shading_data.g_BendParam = g_BendParam;
	// Sun streaks
	shading_data.g_SunColor = g_SunColor;
	shading_data.g_SunDir = g_SunDir;
	shading_data.g_Shineness = g_Shineness;

	D3D11_SUBRESOURCE_DATA cb_init_data;
	cb_init_data.pSysMem = &shading_data;
	cb_init_data.SysMemPitch = 0;
	cb_init_data.SysMemSlicePitch = 0;

	cb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	cb_desc.CPUAccessFlags = 0;
	cb_desc.ByteWidth = PAD16(sizeof(Const_Shading));
	cb_desc.StructureByteStride = 0;
	hr = pd3dDevice->CreateBuffer(&cb_desc, &cb_init_data, &g_pShadingCB);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create water constant buffer error");
	}

	// State blocks
	D3D11_RASTERIZER_DESC ras_desc;
	ras_desc.FillMode = D3D11_FILL_SOLID;
	ras_desc.CullMode = D3D11_CULL_NONE;
	ras_desc.FrontCounterClockwise = FALSE;
	ras_desc.DepthBias = 0;
	ras_desc.SlopeScaledDepthBias = 0.0f;
	ras_desc.DepthBiasClamp = 0.0f;
	ras_desc.DepthClipEnable = TRUE;
	ras_desc.ScissorEnable = FALSE;
	ras_desc.MultisampleEnable = TRUE;
	ras_desc.AntialiasedLineEnable = FALSE;

	hr = pd3dDevice->CreateRasterizerState(&ras_desc, &g_pRSState_Solid);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create water render State Solid error");
	}

	ras_desc.FillMode = D3D11_FILL_WIREFRAME;

	hr = pd3dDevice->CreateRasterizerState(&ras_desc, &g_pRSState_Wireframe);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create water render State wireframe error");
	}

	D3D11_DEPTH_STENCIL_DESC depth_desc;
	memset(&depth_desc, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depth_desc.DepthEnable = FALSE;
	depth_desc.StencilEnable = FALSE;
	hr = pd3dDevice->CreateDepthStencilState(&depth_desc, &g_pDSState_Disable);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create water render State disable error");
	}

	D3D11_BLEND_DESC blend_desc;
	memset(&blend_desc, 0, sizeof(D3D11_BLEND_DESC));
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;
	blend_desc.RenderTarget[0].BlendEnable = TRUE;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = pd3dDevice->CreateBlendState(&blend_desc, &g_pBState_Transparent);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create water render State transparent error");
	}
	engine_basic::engine_fail_reason succeed;;
	return succeed;
}

void FFT_ocean::release()
{
	fullscreen_buffer->release();
	SAFE_RELEASE(g_pFresnelMap);
	SAFE_RELEASE(g_pSRV_Fresnel);
	SAFE_RELEASE(g_pSRV_Perlin);
	SAFE_RELEASE(g_pSRV_ReflectCube);

	SAFE_RELEASE(g_pPerCallCB);
	SAFE_RELEASE(g_pPerFrameCB);
	SAFE_RELEASE(g_pShadingCB);

	SAFE_RELEASE(g_pRSState_Solid);
	SAFE_RELEASE(g_pRSState_Wireframe);
	SAFE_RELEASE(g_pDSState_Disable);
	SAFE_RELEASE(g_pBState_Transparent);
}

engine_basic::engine_fail_reason FFT_ocean::createFresnelMap(ID3D11Device* pd3dDevice)
{
	HRESULT hr;
	//DWORD* buffer = new DWORD[FRESNEL_TEX_SIZE];
	DWORD buffer[FRESNEL_TEX_SIZE];
	for (int i = 0; i < FRESNEL_TEX_SIZE; i++)
	{
		float cos_a = i / (FLOAT)FRESNEL_TEX_SIZE;
		// Using water's refraction index 1.33
		XMVECTOR rec_cos_a = XMLoadFloat(&cos_a);
		float refraction_need = 1.33f;
		XMVECTOR rec_refrection = XMLoadFloat(&refraction_need);
		float fresnel_float;
		XMStoreFloat(&fresnel_float, XMFresnelTerm(rec_cos_a, rec_refrection)*255);
		//DWORD fresnel = (DWORD)(D3DXFresnelTerm(cos_a, 1.33f) * 255);
		DWORD fresnel = (DWORD)(fresnel_float);
		DWORD sky_blend = (DWORD)(powf(1 / (1 + cos_a), g_SkyBlending) * 255);

		buffer[i] = (sky_blend << 8) | fresnel;
	}

	D3D11_TEXTURE1D_DESC tex_desc;
	tex_desc.Width = FRESNEL_TEX_SIZE;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.Usage = D3D11_USAGE_IMMUTABLE;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = buffer;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	hr = pd3dDevice->CreateTexture1D(&tex_desc, &init_data, &g_pFresnelMap);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create water Fresnel buffer error");
	}

	//SAFE_DELETE_ARRAY(buffer);

	// Create shader resource
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	srv_desc.Texture1D.MipLevels = 1;
	srv_desc.Texture1D.MostDetailedMip = 0;

	hr = pd3dDevice->CreateShaderResourceView(g_pFresnelMap, &srv_desc, &g_pSRV_Fresnel);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create water Fresnel SRV error");
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

engine_basic::engine_fail_reason FFT_ocean::loadTextures(ID3D11Device* pd3dDevice)
{
	WCHAR strPath[MAX_PATH];
	HRESULT hr = CreateDDSTextureFromFile(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), L"OceanCS\\perlin_noise.dds", 0, &g_pSRV_Perlin, 0, 0);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "read water perlin_noise texture OceanCS\\perlin_noise.dds error");
	}
	hr = CreateDDSTextureFromFile(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), L"OceanCS\\reflect_cube.dds", 0, &g_pSRV_ReflectCube, 0, 0);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "read water perlin_noise texture OceanCS\\reflect_cube.dds error");
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

void FFT_ocean::renderdraw(ID3D11ShaderResourceView* displacemnet_map, ID3D11ShaderResourceView* gradient_map, float time, ID3D11DeviceContext* pd3dContext)
{
	pd3dContext->RSSetState(g_pRSState_Solid);
	engine_basic::engine_fail_reason check_error;
	auto shader_oceandraw = shader_control::GetInstance()->get_shader_oceandraw_tess(check_error);
	shader_oceandraw->set_texture_displayment(displacemnet_map);
	shader_oceandraw->set_texture_Perlin(g_pSRV_Perlin);
	shader_oceandraw->set_texture_gradient(gradient_map);
	shader_oceandraw->set_texture_Fresnel(g_pSRV_Fresnel);
	shader_oceandraw->set_texture_ReflectCube(g_pSRV_ReflectCube);
	shader_oceandraw->set_Constant_Buffer_Shading(g_pShadingCB);
	XMFLOAT4X4 matView;
	pancy_camera::get_instance()->count_view_matrix(&matView);
	XMStoreFloat4x4(&matView, XMLoadFloat4x4(&XMFLOAT4X4(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1)) * XMLoadFloat4x4(&matView));
	XMFLOAT4X4 matProj = engine_basic::perspective_message::get_instance()->get_proj_matrix();
	//XMMATRIX matWorld;
	
	XMFLOAT4X4 mat_scal;
	XMStoreFloat4x4(&mat_scal, XMMatrixScaling(64*15.2, 64 * 15.2, 0));
	shader_oceandraw->set_trans_scal(&mat_scal);
	XMFLOAT3 posView;
	pancy_camera::get_instance()->get_view_position(&posView);
	shader_oceandraw->set_view_pos(posView);
	//int level_size = 64, length_need = 128000;
	//matWorld = XMMatrixScaling(length_need / level_size, length_need / level_size, 0);

	XMFLOAT4X4 mat_world;
	XMStoreFloat4x4(&mat_world, XMMatrixScaling(0.1f / 64 * 15, 0.1f / 64 * 15, 0.1f / 64 * 15));
	shader_oceandraw->set_trans_world(&mat_world);
	XMMATRIX matWVP = XMLoadFloat4x4(&mat_world) * XMLoadFloat4x4(&matView) * XMLoadFloat4x4(&matProj);
	
	/*
	XMMATRIX matInvWV = XMMatrixScaling(0.1f / 64 * 15, 0.1f / 64 * 15, 0.1f / 64 * 15) * XMLoadFloat4x4(&matView);
	matInvWV = XMMatrixInverse(NULL, matInvWV);
	XMFLOAT3 vLocalEye(0, 0, 0);
	XMStoreFloat3(&vLocalEye, XMVector3TransformCoord(XMLoadFloat3(&vLocalEye), matInvWV));
	shader_oceandraw->set_view_pos(vLocalEye);
	*/

	XMFLOAT4X4 final_mat;
	XMStoreFloat4x4(&final_mat, matWVP);
	shader_oceandraw->set_trans_all(&final_mat);

	ID3DX11EffectTechnique* tech_need;
	shader_oceandraw->get_technique(&tech_need,"LightWater");
	fullscreen_buffer->get_teque(tech_need);
	fullscreen_buffer->show_mesh();
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->HSSetShader(NULL, 0, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->DSSetShader(NULL, 0, 0);
	pd3dContext->RSSetState(NULL);
}
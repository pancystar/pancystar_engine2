#include"pancy_lighting.h"
light_control_singleton *light_control_singleton::light_control_pInstance = NULL;



basic_shadow_map::basic_shadow_map(int width_need, int height_need)
{
	depthmap_tex = NULL;
	depthmap_target = NULL;
	change_shadowsize(width_need, height_need);
}
engine_basic::engine_fail_reason basic_shadow_map::set_renderstate_spot(XMFLOAT3 light_position, XMFLOAT3 light_dir)
{
	//光源视角取景变换
	XMVECTOR lightPos = XMLoadFloat3(&light_position);

	//XMVECTOR lightPos = -2.0f*shadow_range.Radius*lightDir;
	XMVECTOR light_dir_vec = XMLoadFloat3(&light_dir);
	XMVECTOR up = XMVectorSet(0.0f, -1.0f, 1.0f, 0.0f);
	XMMATRIX viewmat = DirectX::XMMatrixLookToLH(lightPos, light_dir_vec, up);
	//透视投影
	//XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, shadowmap_width*1.0f / shadowmap_height*1.0f, 0.1f, 300.0f);
	XMMATRIX proj = XMLoadFloat4x4(&engine_basic::perspective_message::get_instance()->get_proj_shadow_matrix());
	//正投影矩阵
	XMMATRIX final_matrix = viewmat * proj;
	XMStoreFloat4x4(&shadow_build, final_matrix);

	//3D重建后的对比投影矩阵
	XMMATRIX T_need(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
		);
	XMStoreFloat4x4(&shadow_rebuild, final_matrix*T_need);

	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &shadow_map_VP);
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(NULL, depthmap_target);
	/*
	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetRenderTargets(1, renderTargets, depthmap_target);
	*/
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearDepthStencilView(depthmap_target, D3D11_CLEAR_DEPTH, 1.0f, 0);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason basic_shadow_map::set_renderstate(XMFLOAT4X4 shadow_matrix)
{
	shadow_build = shadow_matrix;
	XMMATRIX final_matrix = XMLoadFloat4x4(&shadow_matrix);
	//3D重建后的对比投影矩阵
	XMMATRIX T_need(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
		);
	XMStoreFloat4x4(&shadow_rebuild, final_matrix*T_need);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetViewports(1, &shadow_map_VP);
	//ID3D11RenderTargetView* renderTargets[1] = { 0 };
	//contex_pancy->OMSetRenderTargets(1, renderTargets, depthmap_target);
	d3d_pancy_basic_singleton::GetInstance()->set_render_target(NULL, depthmap_target);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->ClearDepthStencilView(depthmap_target, D3D11_CLEAR_DEPTH, 1.0f, 0);
	engine_basic::engine_fail_reason succeed;
	return succeed;
}

ID3D11ShaderResourceView* basic_shadow_map::get_mapresource()
{
	return depthmap_tex;
}
void basic_shadow_map::release()
{
	if (depthmap_tex != NULL)
	{
		depthmap_tex->Release();
		depthmap_tex = NULL;
	}

	if (depthmap_target != NULL)
	{
		depthmap_target->Release();
		depthmap_target = NULL;
	}
}
void basic_shadow_map::change_shadowsize(int width_need, int height_need)
{
	shadowmap_width = width_need;
	shadowmap_height = height_need;
	//释放之前的shader resource view以及render target view
	if (depthmap_tex != NULL)
	{
		depthmap_tex->Release();
		depthmap_tex = NULL;
	}
	if (depthmap_target != NULL)
	{
		depthmap_target->Release();
		depthmap_target = NULL;
	}
	//指定渲染视口的大小
	shadow_map_VP.TopLeftX = 0.0f;
	shadow_map_VP.TopLeftY = 0.0f;
	shadow_map_VP.Width = static_cast<float>(shadowmap_width);
	shadow_map_VP.Height = static_cast<float>(shadowmap_height);
	shadow_map_VP.MinDepth = 0.0f;
	shadow_map_VP.MaxDepth = 1.0f;
}
engine_basic::engine_fail_reason basic_shadow_map::init_texture(ID3D11Texture2D* depthMap_array, int index_need)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~深度模板渲染目标~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	D3D11_TEXTURE2D_DESC texDesc;
	depthMap_array->GetDesc(&texDesc);
	//建立GPU上的两种资源：纹理资源以及渲染目标资源
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc =
	{
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D11_DSV_DIMENSION_TEXTURE2DARRAY
	};
	dsvDesc.Texture2DArray.ArraySize = 1;
	dsvDesc.Texture2DArray.FirstArraySlice = index_need;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateDepthStencilView(depthMap_array, &dsvDesc, &depthmap_target);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create shader resource view error when create shadowmap resource");
		return error_message;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~深度信息访问资源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc =
	{
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		D3D11_SRV_DIMENSION_TEXTURE2DARRAY
	};
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2DArray.ArraySize = 1;
	srvDesc.Texture2DArray.FirstArraySlice = index_need;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(depthMap_array, &srvDesc, &depthmap_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create render target view error when create shadowmap resource");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason basic_shadow_map::create(ID3D11Texture2D* depthMap_array, int index_need)
{
	return init_texture(depthMap_array, index_need);
}
engine_basic::engine_fail_reason basic_shadow_map::reset_texture(ID3D11Texture2D* depthMap_array, int index_need)
{
	//释放之前的shader resource view以及render target view
	if (depthmap_tex != NULL)
	{
		depthmap_tex->Release();
		depthmap_tex = NULL;
	}
	if (depthmap_target != NULL)
	{
		depthmap_target->Release();
		depthmap_target = NULL;
	}
	return init_texture(depthMap_array, index_need);
}

//光源部分
basic_point_lighting::basic_point_lighting(light_type type_need_light, shadow_type type_need_shadow)
{
	light_source_type = type_need_light;
	shadow_source_type = type_need_shadow;
	if (type_need_light == direction_light)
	{
		init_comman_dirlight(type_need_shadow);
	}
	else if (type_need_light == point_light)
	{
		init_comman_pointlight(type_need_shadow);
	}
	else if (type_need_light == spot_light)
	{
		init_comman_spotlight(type_need_shadow);
	}
}
void basic_point_lighting::init_comman_dirlight(shadow_type type_need_shadow)
{
	XMFLOAT4 rec_ambient(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 rec_diffuse(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_specular(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT3 rec_dir(-0.998858631,0.0229830630,0.0418711342);
	light_data.ambient = rec_ambient;
	light_data.diffuse = rec_diffuse;
	light_data.specular = rec_specular;
	light_data.dir = rec_dir;
	light_data.range = 10.0f;
	light_data.light_type.x = direction_light;
	light_data.light_type.y = type_need_shadow;
}
void basic_point_lighting::init_comman_pointlight(shadow_type type_need_shadow)
{
	XMFLOAT4 rec_ambient1(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse1(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular1(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT3 rec_decay(1.0f, 0.6f, 0.0f);

	light_data.ambient = rec_ambient1;
	light_data.diffuse = rec_diffuse1;
	light_data.specular = rec_specular1;
	light_data.decay = rec_decay;
	light_data.range = 150.0f;
	light_data.position = XMFLOAT3(0.0f, 15.0f, 0.0f);
	light_data.light_type.x = point_light;
	light_data.light_type.y = type_need_shadow;
}
void basic_point_lighting::init_comman_spotlight(shadow_type type_need_shadow)
{
	XMFLOAT4 rec_ambient1(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse1(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular1(1.0f, 1.0f, 1.0f, 1.0f);

	XMFLOAT3 rec_decay1(1.0f, 0.3f, 0.0f);

	light_data.ambient = rec_ambient1;
	light_data.diffuse = rec_diffuse1;
	light_data.specular = rec_specular1;
	light_data.decay = rec_decay1;
	light_data.range = 100.0f;
	light_data.position = XMFLOAT3(0.0f, 2.5f, 2.5f);
	light_data.dir = XMFLOAT3(0.0f, -1.0f, -1.0f);
	light_data.spot = 12.0f;
	light_data.theta = 3.141592653f / 5.0f;
	light_data.light_type.x = spot_light;
	light_data.light_type.y = type_need_shadow;
}
void basic_point_lighting::set_light_ambient(float red, float green, float blue, float alpha)
{
	light_data.ambient = XMFLOAT4(red, green, green, alpha);
}
void basic_point_lighting::set_light_diffuse(float red, float green, float blue, float alpha)
{
	light_data.diffuse = XMFLOAT4(red, green, green, alpha);
}
void basic_point_lighting::set_light_specular(float red, float green, float blue, float alpha)
{
	light_data.specular = XMFLOAT4(red, green, green, alpha);
}
void basic_point_lighting::set_light_position(float x, float y, float z)
{
	light_data.position = XMFLOAT3(x, y, z);
}
void basic_point_lighting::set_light_dir(float x, float y, float z)
{
	light_data.dir = XMFLOAT3(x, y, z);
}
void basic_point_lighting::set_light_decay(float x0, float x1, float x2)
{
	light_data.decay = XMFLOAT3(x0, x1, x2);
}
void basic_point_lighting::set_light_range(float range_need)
{
	light_data.range = range_need;
}
void basic_point_lighting::set_light_spottheata(float theta)
{
	light_data.theta = theta;
}
void basic_point_lighting::set_light_spotstrenth(float spot)
{
	light_data.spot = spot;
}
void basic_point_lighting::set_frontlight(int light_num)
{
	//auto shader_test = shader_control::GetInstance()->get_shader_prelight();
	//shader_test->set_light(light_data, light_num);
}
void basic_point_lighting::set_defferedlight(int light_num)
{
	engine_basic::engine_fail_reason check_error;
	auto shader_test = shader_control::GetInstance()->get_shader_lightbuffer(check_error);
	shader_test->set_light(light_data, light_num);
}



spotlight_with_shadowmap::spotlight_with_shadowmap(int width_shadow, int height_shadow) : basic_point_lighting(light_type::spot_light, shadow_type::shadow_map)
{
	shadowmap_deal = new basic_shadow_map(width_shadow, height_shadow);
}
/*
engine_basic::engine_fail_reason spotlight_with_shadowmap::create(ID3D11Texture2D* depthMap_array, int index_need)
{
return shadowmap_deal->create(depthMap_array, index_need);
}
*/
engine_basic::engine_fail_reason spotlight_with_shadowmap::reset_texture(ID3D11Texture2D* depthMap_array, int index_need)
{
	return shadowmap_deal->reset_texture(depthMap_array, index_need);
}



void spotlight_with_shadowmap::draw_shadow(pancy_geometry_control *geometry_list)
{
	shadowmap_deal->set_renderstate_spot(light_data.position, light_data.dir);
	//pancy_geometry_control_singleton::get_instance()->render_shadowmap(shadowmap_deal->get_shadow_build_matrix(),false);
	geometry_list->render_shadowmap(shadowmap_deal->get_shadow_build_matrix(), false);
}
void spotlight_with_shadowmap::release()
{
	shadowmap_deal->release();
}

sunlight_with_shadowmap::sunlight_with_shadowmap(int width_need, int height_need, int shadow_divide_num) : basic_point_lighting(light_type::direction_light, shadow_type::shadow_map)
{
	shadow_width = width_need;
	shadow_height = height_need;
	shadow_devide = shadow_divide_num;
	sunlight_lamda_log = 0.75;
	for (int i = 0; i < 20; ++i)
	{
		shadowmap_array[i] = NULL;
	}
	for (int i = 0; i < shadow_devide; ++i)
	{
		shadowmap_array[i] = new basic_shadow_map(width_need, height_need);
	}

}
engine_basic::engine_fail_reason sunlight_with_shadowmap::create()
{
	//~~~~~~~~~~~~~~~~~~~~创建阴影图纹理数组资源~~~~~~~~~~~~~~~~~~~
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = shadow_width;
	texDesc.Height = shadow_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = shadow_devide;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D          *sunlight_pssm_ShadowArray;     //阳光分级阴影;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, NULL, &sunlight_pssm_ShadowArray);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_meaasge(hr, "create sunlight shadowmap texarray error");
		return error_meaasge;
	}
	//~~~~~~~~~~~~~~~~~~~~创建阴影图纹理数组访问器~~~~~~~~~~~~~~~~~~~
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd = {
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
	};
	dsrvd.Texture2DArray.ArraySize = shadow_devide;
	dsrvd.Texture2DArray.FirstArraySlice = 0;
	dsrvd.Texture2DArray.MipLevels = 1;
	dsrvd.Texture2DArray.MostDetailedMip = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(sunlight_pssm_ShadowArray, &dsrvd, &sunlight_pssm_Shadowresource);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_meaasge(hr, "create sunlight shadowmap texarray resource view error");
		return error_meaasge;
	}
	for (int i = 0; i < shadow_devide; ++i)
	{
		auto error_message = shadowmap_array[i]->create(sunlight_pssm_ShadowArray, i);
		if (!error_message.check_if_failed())
		{
			return error_message;
		}
	}
	sunlight_pssm_ShadowArray->Release();
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void sunlight_with_shadowmap::draw_shadow(pancy_geometry_control *geometry_list)
{
	divide_view_frustum(sunlight_lamda_log, shadow_devide);
	for (int i = 0; i < shadow_devide; ++i)
	{
		draw_shadow_basic(geometry_list,i);
	}
	engine_basic::engine_fail_reason check_error;
	auto shader_deffered = shader_control::GetInstance()->get_shader_lightbuffer(check_error);
	XMFLOAT4 rec_depth_devide;
	rec_depth_devide.x = sunlight_pssm_depthdevide[1];
	rec_depth_devide.y = sunlight_pssm_depthdevide[2];
	rec_depth_devide.z = sunlight_pssm_depthdevide[3];
	rec_depth_devide.w = sunlight_pssm_depthdevide[4];
	shader_deffered->set_depth_devide(rec_depth_devide);
	XMUINT3 rec_sunlight_num = XMUINT3(shadow_devide, 0, 0);
	shader_deffered->set_sunlight_num(rec_sunlight_num);
	shader_deffered->set_sunshadow_tex(sunlight_pssm_Shadowresource);
	XMFLOAT4X4 mat_sun_2tex[30];
	for (int i = 0; i < shadow_devide; ++i)
	{
		mat_sun_2tex[i] = shadowmap_array[i]->get_shadow_rebuild_matrix();
	}
	shader_deffered->set_sunshadow_matrix(mat_sun_2tex, shadow_devide);
	shader_deffered->set_sunlight(light_data);
}
void sunlight_with_shadowmap::draw_shadow_basic(pancy_geometry_control *geometry_list,int count)
{
	//更新渲染状态
	shadowmap_array[count]->set_renderstate(mat_sunlight_pssm[count]);
	//绘制阴影
	//pancy_geometry_control_singleton::get_instance()->render_shadowmap(shadowmap_array[count]->get_shadow_build_matrix(),false);
	geometry_list->render_shadowmap(shadowmap_array[count]->get_shadow_build_matrix(), false);
}
void sunlight_with_shadowmap::divide_view_frustum(float lamda_log, int divide_num)
{
	float C_log, C_uni;
	for (int i = 0; i < divide_num + 1; ++i)
	{
		float now_percent = static_cast<float>(i) / static_cast<float>(divide_num);
		C_log = engine_basic::perspective_message::get_instance()->get_perspective_near_plane() * pow((300 / engine_basic::perspective_message::get_instance()->get_perspective_near_plane()), now_percent);
		C_uni = engine_basic::perspective_message::get_instance()->get_perspective_near_plane() + (300 - engine_basic::perspective_message::get_instance()->get_perspective_near_plane()) * now_percent;
		sunlight_pssm_depthdevide[i] = C_log * lamda_log + (1.0f - lamda_log) * C_uni - 25;
		if (i > 0)
		{
			mat_sunlight_pssm[i - 1] = build_matrix_sunlight(sunlight_pssm_depthdevide[0], sunlight_pssm_depthdevide[i], light_data.dir);
		}
	}
}
XMFLOAT4X4 sunlight_with_shadowmap::build_matrix_sunlight(float near_plane, float far_plane, XMFLOAT3 light_dir)
{
	//视截体的中心点
	XMFLOAT3 center_pos;
	/*
	//直接计算中心点坐标的方法
	XMFLOAT3 view_dir, view_pos;
	scene_camera->get_view_direct(&view_dir);
	XMVECTOR dir_view_vec = XMLoadFloat3(&view_dir);
	scene_camera->get_view_position(&view_pos);
	XMVECTOR now_near_center = XMLoadFloat3(&view_pos) , now_far_center = XMLoadFloat3(&view_pos);
	now_near_center +=  dir_view_vec * near_plane;
	now_far_center += dir_view_vec * far_plane;
	XMVECTOR now_center = (now_near_center + now_far_center) / 2.0f;
	XMStoreFloat3(&center_pos, now_center);
	*/
	//取景变换的逆变换
	XMFLOAT4X4 invview_float4x4;
	pancy_camera::get_instance()->count_invview_matrix(&invview_float4x4);
	XMMATRIX invview_mat = XMLoadFloat4x4(&invview_float4x4);
	//将(0,0,(near+far) / 2)进行逆取景变换得到中心点的世界坐标
	XMVECTOR center_view = XMLoadFloat4(&XMFLOAT4(0.0f, 0.0f, (near_plane + far_plane) / 2.0f, 1.0f));
	XMVECTOR center_check = XMVector4Transform(center_view, invview_mat);
	XMStoreFloat3(&center_pos, center_check);

	XMFLOAT4 FrustumnearCorner[4];
	XMFLOAT4 FrustumFarCorner[4];
	//近截面的四个角点
	float aspect = static_cast<float>(d3d_pancy_basic_singleton::GetInstance()->get_wind_width()) / static_cast<float>(d3d_pancy_basic_singleton::GetInstance()->get_wind_height());
	float halfHeight = near_plane * tanf(0.5f*engine_basic::perspective_message::get_instance()->get_perspective_angle());
	float halfWidth = aspect * halfHeight;

	FrustumnearCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, near_plane, 1.0f);
	FrustumnearCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, near_plane, 1.0f);
	FrustumnearCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, near_plane, 1.0f);
	FrustumnearCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, near_plane, 1.0f);
	for (int i = 0; i < 4; ++i)
	{
		//还原到世界坐标系
		XMVECTOR rec_now = XMLoadFloat4(&FrustumnearCorner[i]);
		XMVECTOR rec_ans = XMVector4Transform(rec_now, invview_mat);
		XMStoreFloat4(&FrustumnearCorner[i], rec_ans);
	}
	//远截面的四个角点
	halfHeight = far_plane * tanf(0.5f*engine_basic::perspective_message::get_instance()->get_perspective_angle());
	halfWidth = aspect * halfHeight;
	FrustumFarCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, far_plane, 1.0f);
	FrustumFarCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, far_plane, 1.0f);
	FrustumFarCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, far_plane, 1.0f);
	FrustumFarCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, far_plane, 1.0f);
	for (int i = 0; i < 4; ++i)
	{
		//还原到世界坐标系
		XMVECTOR rec_now = XMLoadFloat4(&FrustumFarCorner[i]);
		XMVECTOR rec_ans = XMVector4Transform(rec_now, invview_mat);
		XMStoreFloat4(&FrustumFarCorner[i], rec_ans);
	}
	//光源视角取景变换
	XMVECTOR lightDir = XMLoadFloat3(&light_dir);
	XMVECTOR lightPos = center_check;
	XMFLOAT3 up_dir = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMVECTOR upDir = XMLoadFloat3(&up_dir);
	XMFLOAT3 check_dir;
	XMStoreFloat3(&check_dir, XMVector3Cross(lightDir, upDir));
	if (abs(check_dir.x) < 0.001f && abs(check_dir.y) < 0.001f && abs(check_dir.z) < 0.001f)
	{
		up_dir = XMFLOAT3(0.0f, 1.0f, 0.0f);
	}
	XMFLOAT4X4 view_mat_pre;
	pancy_camera::get_instance()->count_view_matrix(light_dir, up_dir, center_pos, &view_mat_pre);
	XMMATRIX light_view_mat = XMLoadFloat4x4(&view_mat_pre);
	XMVECTOR rec_check_center = XMVector4Transform(center_check, light_view_mat);
	XMFLOAT4 float_check_center;
	XMStoreFloat4(&float_check_center, rec_check_center);

	for (int i = 0; i < 4; ++i)
	{
		//变换到光源视角
		XMVECTOR rec_now = XMLoadFloat4(&FrustumnearCorner[i]);
		XMVECTOR rec_ans = XMVector4Transform(rec_now, light_view_mat);
		XMStoreFloat4(&FrustumnearCorner[i], rec_ans);
	}
	for (int i = 0; i < 4; ++i)
	{
		//变换到光源视角
		XMVECTOR rec_now = XMLoadFloat4(&FrustumFarCorner[i]);
		XMVECTOR rec_ans = XMVector4Transform(rec_now, light_view_mat);
		XMStoreFloat4(&FrustumFarCorner[i], rec_ans);
	}
	XMFLOAT3 min_pos;
	XMFLOAT3 max_pos;
	build_AABB_box(FrustumnearCorner, FrustumFarCorner, min_pos, max_pos);
	//将投影中心拉到合适的位置
	XMFLOAT3 new_center_pos = XMFLOAT3((min_pos.x + max_pos.x) / 2.0f, (min_pos.y + max_pos.y) / 2.0f, min_pos.z);
	new_center_pos.z -= 1.0f;
	//乘以取景变换的逆矩阵得到新的投影中心点在世界坐标系的位置
	XMFLOAT4X4 inv_view_light;
	pancy_camera::get_instance()->count_invview_matrix(light_dir, up_dir, center_pos, &inv_view_light);
	XMMATRIX light_invview_mat = XMLoadFloat4x4(&inv_view_light);
	XMVECTOR new_center_vector = XMLoadFloat4(&XMFLOAT4(new_center_pos.x, new_center_pos.y, new_center_pos.z, 1.0f));
	XMVECTOR rec_trans_center = XMVector4Transform(new_center_vector, light_invview_mat);
	XMStoreFloat3(&new_center_pos, rec_trans_center);
	//重新计算取景变换矩阵
	pancy_camera::get_instance()->count_view_matrix(light_dir, up_dir, new_center_pos, &view_mat_pre);
	//根据AABB包围盒的长宽高计算投影变换矩阵
	float view_width_need = max_pos.x - min_pos.x;
	float view_height_need = max_pos.y - min_pos.y;
	XMMATRIX mat_orth_view = XMMatrixOrthographicLH(view_width_need, view_height_need, 0.5f, (max_pos.z - min_pos.z) + 1.0f);
	//计算取景*投影矩阵
	light_view_mat = XMLoadFloat4x4(&view_mat_pre);
	XMMATRIX final_matrix = light_view_mat * mat_orth_view;
	XMFLOAT4X4 mat_view_project;
	XMStoreFloat4x4(&mat_view_project, final_matrix);
	return mat_view_project;
}
void sunlight_with_shadowmap::build_AABB_box(XMFLOAT4 near_point[4], XMFLOAT4 far_point[4], XMFLOAT3 &min_pos, XMFLOAT3 &max_pos)
{
	min_pos = XMFLOAT3(99999.0f, 999999.0f, 99999.0f);
	max_pos = XMFLOAT3(-99999.0f, -99999.0f, -99999.0f);
	//包围盒算法求视截体
	for (int i = 0; i < 4; ++i)
	{
		if (near_point[i].x > max_pos.x)
		{
			max_pos.x = near_point[i].x;
		}
		if (near_point[i].x < min_pos.x)
		{
			min_pos.x = near_point[i].x;
		}

		if (near_point[i].y > max_pos.y)
		{
			max_pos.y = near_point[i].y;
		}
		if (near_point[i].y < min_pos.y)
		{
			min_pos.y = near_point[i].y;
		}

		if (near_point[i].z > max_pos.z)
		{
			max_pos.z = near_point[i].z;
		}
		if (near_point[i].z < min_pos.z)
		{
			min_pos.z = near_point[i].z;
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		if (far_point[i].x > max_pos.x)
		{
			max_pos.x = far_point[i].x;
		}
		if (far_point[i].x < min_pos.x)
		{
			min_pos.x = far_point[i].x;
		}

		if (far_point[i].y > max_pos.y)
		{
			max_pos.y = far_point[i].y;
		}
		if (far_point[i].y < min_pos.y)
		{
			min_pos.y = far_point[i].y;
		}

		if (far_point[i].z > max_pos.z)
		{
			max_pos.z = far_point[i].z;
		}
		if (far_point[i].z < min_pos.z)
		{
			min_pos.z = far_point[i].z;
		}
	}
	//规范化视截体的视口
	if (min_pos.x < min_pos.y)
	{
		min_pos.y = min_pos.x;
	}
	else
	{
		min_pos.x = min_pos.y;
	}
	if (max_pos.x > max_pos.y)
	{
		max_pos.y = max_pos.x;
	}
	else
	{
		max_pos.x = max_pos.y;
	}
}
void sunlight_with_shadowmap::release()
{
	sunlight_pssm_Shadowresource->Release();
	for (int i = 0; i < shadow_devide; ++i)
	{
		shadowmap_array[i]->release();
	}
}

light_control_singleton::light_control_singleton(int max_shadow_num_need, int common_shadow_width_need, int common_shadow_height_need)
{
	max_shadow_num = max_shadow_num_need;
	common_shadow_width = common_shadow_width_need;
	common_shadow_height = common_shadow_height_need;
	sunlight_use = 0;
	sunlight_IDcount = 0;
}
void light_control_singleton::add_light_without_shadow(light_type type_need_light)
{
	//初始化一个无影光源
	basic_point_lighting *light_need = new basic_point_lighting(type_need_light, shadow_none);
	nonshadow_light_list.push_back(*light_need);
}
void light_control_singleton::add_spotlight_with_shadow_map()
{
	//初始化一个带阴影图聚光灯光源
	spotlight_with_shadowmap *new_spotlight = new spotlight_with_shadowmap(common_shadow_width, common_shadow_height);
	shadowmap_light_list.push_back(*new_spotlight);
}
engine_basic::engine_fail_reason light_control_singleton::add_sunlight_with_shadow_map(int width_shadow, int height_shadow, int shadow_num, int &sunlight_ID)
{
	sunlight_with_shadowmap *light_need = new sunlight_with_shadowmap(width_shadow, height_shadow, shadow_num);
	engine_basic::engine_fail_reason check_error = light_need->create();
	sunlight_ID = -1;
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	std::pair<int, sunlight_with_shadowmap> data_need(sunlight_IDcount, *light_need);
	auto check_iferror = sun_pssmshadow_light.insert(data_need);
	if (!check_iferror.second)
	{
		engine_basic::engine_fail_reason error_message("a same sunlight has already in");
		return error_message;
	}
	sunlight_ID = sunlight_IDcount++;
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
engine_basic::engine_fail_reason light_control_singleton::create()
{
	//~~~~~~~~~~~~~~~~~~~~创建阴影图纹理数组资源~~~~~~~~~~~~~~~~~~~
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = common_shadow_width;
	texDesc.Height = common_shadow_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = max_shadow_num;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture2D(&texDesc, NULL, &ShadowTextureArray);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create basic spot shadowmap texarray error");
		return error_message;
	}
	//~~~~~~~~~~~~~~~~~~~~创建阴影图纹理数组访问器~~~~~~~~~~~~~~~~~~~
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd = {
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
	};
	dsrvd.Texture2DArray.ArraySize = max_shadow_num;
	dsrvd.Texture2DArray.FirstArraySlice = 0;
	dsrvd.Texture2DArray.MipLevels = 1;
	dsrvd.Texture2DArray.MostDetailedMip = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(ShadowTextureArray, &dsrvd, &shadow_map_resource);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create basic spot shadowmap texarray resource view error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
void light_control_singleton::update_sunlight(XMFLOAT3 dir) 
{
	auto data_now = sun_pssmshadow_light.find(sunlight_use);
	if (data_now != sun_pssmshadow_light.end())
	{
		data_now->second.set_light_dir(dir.x, dir.y, dir.z);
	}
}
void light_control_singleton::draw_shadow(pancy_geometry_control *geometry_list)
{
	int count_light = 0;
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		rec_shadow_light._Ptr->reset_texture(ShadowTextureArray, count_light++);
		rec_shadow_light._Ptr->draw_shadow(geometry_list);
	}
	auto data_now = sun_pssmshadow_light.find(sunlight_use);
	if (data_now != sun_pssmshadow_light.end())
	{
		data_now->second.draw_shadow(geometry_list);
	}
	update_and_setlight();
}
void light_control_singleton::release()
{
	ShadowTextureArray->Release();
	shadow_map_resource->Release();
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		rec_shadow_light._Ptr->release();
	}
	for (auto rec_shadow_sun = sun_pssmshadow_light.begin(); rec_shadow_sun != sun_pssmshadow_light.end(); ++rec_shadow_sun)
	{
		rec_shadow_sun->second.release();
	}
	nonshadow_light_list.clear();
	shadowmap_light_list.clear();
	sun_pssmshadow_light.clear();
}
void light_control_singleton::update_and_setlight()
{
	int count_light_point = 0, count_light_dir = 0, count_light_spot = 0, count = 0;
	int count_shadow_point = 0, count_shadow_dir = 0, count_shadow_spot = 0;
	//普通带阴影的聚光灯光源
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		rec_shadow_light._Ptr->set_frontlight(count);
		rec_shadow_light._Ptr->set_defferedlight(count);
		count_shadow_spot += 1;
		count += 1;
	}
	//设置无影光源(方向光)
	for (auto rec_non_light = nonshadow_light_list.begin(); rec_non_light != nonshadow_light_list.end(); ++rec_non_light)
	{
		light_type check_light = rec_non_light._Ptr->get_light_type();
		if (check_light == direction_light)
		{
			rec_non_light._Ptr->set_frontlight(count);
			rec_non_light._Ptr->set_defferedlight(count);
			count_light_dir += 1;
		}
		count += 1;
	}
	//设置无影光源(聚光灯)
	for (auto rec_non_light = nonshadow_light_list.begin(); rec_non_light != nonshadow_light_list.end(); ++rec_non_light)
	{
		light_type check_light = rec_non_light._Ptr->get_light_type();
		if (check_light == point_light)
		{
			rec_non_light._Ptr->set_frontlight(count);
			rec_non_light._Ptr->set_defferedlight(count);
			count_light_point += 1;
		}
		count += 1;
	}
	//设置无影光源(点光源)
	for (auto rec_non_light = nonshadow_light_list.begin(); rec_non_light != nonshadow_light_list.end(); ++rec_non_light)
	{
		light_type check_light = rec_non_light._Ptr->get_light_type();
		if (check_light == spot_light)
		{
			rec_non_light._Ptr->set_frontlight(count);
			rec_non_light._Ptr->set_defferedlight(count);
			count_light_spot += 1;
		}
		count += 1;
	}
	engine_basic::engine_fail_reason check_error;
	auto shader_deffered = shader_control::GetInstance()->get_shader_lightbuffer(check_error);
	XMFLOAT4X4 mat_shadow[30];
	int shadow_num;
	get_shadow_map_matrix(mat_shadow, shadow_num);
	shader_deffered->set_shadow_matrix(mat_shadow, shadow_num);
	shader_deffered->set_shadow_tex(shadow_map_resource);
	XMUINT3 shadownum = XMUINT3(count_shadow_dir, count_shadow_point, count_shadow_spot);
	XMUINT3 lightnum = XMUINT3(count_light_dir, count_light_point, count_light_spot);
	shader_deffered->set_light_num(lightnum);
	shader_deffered->set_shadow_num(shadownum);
}
void light_control_singleton::get_shadow_map_matrix(XMFLOAT4X4* mat_out, int &mat_num_out)
{
	mat_num_out = 0;
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		mat_out[mat_num_out++] = rec_shadow_light._Ptr->get_shadow_rebuild_matrix();
	}
}
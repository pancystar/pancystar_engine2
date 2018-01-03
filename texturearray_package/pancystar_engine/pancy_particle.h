#pragma once
#include"geometry.h"
#include"pancystar_engine_basic.h"
#include"pancy_d3d11_basic.h"
#include"shader_pancy.h"
enum ParticleLimitShape
{
	cube = 1,
	sphere = 2,
	cone = 3,
};
struct ParticleLimitShape_cube
{
	XMFLOAT3 square_size;
	XMFLOAT3 speed_dir;
};
struct ParticleLimitShape_sphere
{
	float ridaus;
};
struct ParticleLimitShape_cone
{
	XMFLOAT3 direction;
	float angle;
};
template<typename T>
class particle_looping
{
	bool if_custom_shader;   //是否自定义shader
	int  particle_copy;                 //粒子数量
	bool if_firstrun;                   //判断粒子系统是否已经开始工作
	float age_need;                     //粒子寿命
										//float game_time;                    //全局游戏时间
										//float time_delta;                   //两帧之间的时间间隔
	float gravity_scale;                //重力系数
	XMFLOAT3 start_pos;                 //粒子位置
										//XMFLOAT3 start_dir;                 //粒子产生方向
	UINT particle_num;                  //粒子数量
	ID3D11Buffer* auto_Vinput_need;     //粒子顶点输入缓冲区
	ID3D11Buffer* first_Vdraw_need;     //粒子顶点初始缓冲区
	ID3D11Buffer* auto_Vstream0ut_need; //粒子顶点产出缓冲区
	ID3D11ShaderResourceView* particle_tex;//粒子纹理
	ID3D11ShaderResourceView* random_tex;  //随机数纹理
	ID3D11ShaderResourceView* color_tex;   //颜色纹理
public:
	particle_looping(UINT max_particle_num);
	void draw_particle();
	void update(float delta_time, float time_world, XMFLOAT4X4 *mat_proj, XMFLOAT3 *view_direct);//更新粒子动画时间
	void set_particle_direct(XMFLOAT3 *position, XMFLOAT3 *direct);
	engine_basic::engine_fail_reason create(wchar_t *texture_path);
	void reset();
	void release();
private:
	engine_basic::engine_fail_reason init_vertex_buff();
	engine_basic::engine_fail_reason init_texture(wchar_t *texture_path);
	engine_basic::engine_fail_reason CreateRandomTexture1DSRV();
	template<class K>
	void safe_release(K t)
	{
		if (t != NULL)
		{
			t->Release();
			t = 0;
		}
	}
};
template<typename T>
engine_basic::engine_fail_reason particle_looping<T>::CreateRandomTexture1DSRV()
{
	// 创建随机数据.
	XMFLOAT4 randomValues[1024];
	for (int i = 0; i < 1024; ++i)
	{
		randomValues[i].x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		randomValues[i].y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		randomValues[i].z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		randomValues[i].w = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	}
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = randomValues;
	initData.SysMemPitch = 1024 * sizeof(XMFLOAT4);
	initData.SysMemSlicePitch = 0;
	// 创建纹理.
	D3D11_TEXTURE1D_DESC texDesc;
	texDesc.Width = 1024;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.ArraySize = 1;
	ID3D11Texture1D* randomTex = 0;
	HRESULT hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateTexture1D(&texDesc, &initData, &randomTex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create random tex data error in particle system");
		return error_message;
	}
	// 创建资源视图.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
	viewDesc.Texture1D.MostDetailedMip = 0;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateShaderResourceView(randomTex, &viewDesc, &random_tex);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create random tex SRV error in particle system");
		return error_message;
	}
	if (randomTex != NULL)
	{
		randomTex->Release();
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;

}
template<typename T>
particle_looping<T>::particle_looping(UINT max_particle_num)
{
	auto_Vinput_need = NULL;
	first_Vdraw_need = NULL;
	auto_Vstream0ut_need = NULL;
	random_tex = NULL;
	particle_num = max_particle_num;
}
template<typename T>
engine_basic::engine_fail_reason particle_looping<T>::create(wchar_t *texture_path)
{
	if_custom_shader = false;
	//particle_fx = shader_control::GetInstance()->get_shader_particle_basic();
	engine_basic::engine_fail_reason check_error = init_texture(texture_path);
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	//reset();
	check_error = init_vertex_buff();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
template<typename T>
engine_basic::engine_fail_reason particle_looping<T>::init_texture(wchar_t *texture_path)
{
	engine_basic::engine_fail_reason check_error = CreateRandomTexture1DSRV();
	if (!check_error.check_if_failed())
	{
		return check_error;
	}
	HRESULT hr = CreateDDSTextureFromFile(d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device(), texture_path, 0, &particle_tex, 0, 0);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "load particle color texture_input error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
template<typename T>
engine_basic::engine_fail_reason particle_looping<T>::init_vertex_buff()
{
	HRESULT hr;
	//初始顶点缓存
	D3D11_BUFFER_DESC VB_desc;
	VB_desc.Usage = D3D11_USAGE_DEFAULT;
	VB_desc.ByteWidth = sizeof(T) * 1;
	VB_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VB_desc.CPUAccessFlags = 0;
	VB_desc.MiscFlags = 0;
	VB_desc.StructureByteStride = 0;
	T p;
	ZeroMemory(&p, sizeof(T));
	p.Age = 0.0f;
	p.Type = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &p;
	hr = d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&VB_desc, &vinitData, &first_Vdraw_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create particle vertex buffer data_input error");
		return error_message;
	}
	//运行中交换的顶点缓存
	//VB_desc.Usage = D3D11_USAGE_DYNAMIC;
	VB_desc.ByteWidth = sizeof(T) * particle_num;
	VB_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&VB_desc, 0, &auto_Vinput_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create particle vertex buffer stream_input error");
		return error_message;
	}
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_device()->CreateBuffer(&VB_desc, 0, &auto_Vstream0ut_need);
	if (FAILED(hr))
	{
		engine_basic::engine_fail_reason error_message(hr, "create particle vertex buffer stream_output error");
		return error_message;
	}
	engine_basic::engine_fail_reason succeed;
	return succeed;
}
template<typename T>
void particle_looping<T>::update(float delta_time, float time_world, XMFLOAT4X4 *mat_proj, XMFLOAT3 *view_direct)
{
	engine_basic::engine_fail_reason check_error;
	std::shared_ptr<shader_particle> particle_fx;
	if (!if_custom_shader)
	{
		particle_fx = shader_control::GetInstance()->get_shader_particle_basic(check_error);       //粒子着色器
		if (!check_error.check_if_failed())
		{
			check_error.show_failed_reason();
			return;
		}
	}
	else
	{
		particle_fx = NULL;
		return;
	}
	//time_delta = delta_time;
	//game_time = time_world;
	age_need += delta_time;
	particle_fx->set_trans_all(mat_proj);
	particle_fx->set_viewposition(*view_direct);
	particle_fx->set_frametime(time_world, delta_time);
}
template<typename T>
void particle_looping<T>::reset()
{
	if_firstrun = true;
	//game_time = 0.0f;
	age_need = 0.0f;
	//time_delta = 0.0f;
}
template<typename T>
void particle_looping<T>::release()
{
	safe_release(auto_Vinput_need);
	safe_release(first_Vdraw_need);
	safe_release(auto_Vstream0ut_need);
	safe_release(particle_tex);
	safe_release(random_tex);
}
template<typename T>
void particle_looping<T>::set_particle_direct(XMFLOAT3 *position, XMFLOAT3 *direct)
{
	start_pos = *position;
}
template<typename T>
void particle_looping<T>::draw_particle()
{
	engine_basic::engine_fail_reason check_error;
	std::shared_ptr<shader_particle> particle_fx;
	if (!if_custom_shader)
	{
		particle_fx = shader_control::GetInstance()->get_shader_particle_basic(check_error);       //粒子着色器
		if (!check_error.check_if_failed())
		{
			check_error.show_failed_reason();
			return;
		}
	}
	else
	{
		particle_fx = NULL;
		return;
	}
	check_error = particle_fx->set_startposition(start_pos);
	check_error = particle_fx->set_texture(particle_tex);
	check_error = particle_fx->set_randomtex(random_tex);
	//粒子的更新shader
	ID3DX11EffectTechnique                *teque_need;          //绘制路径
	check_error = particle_fx->get_technique(&teque_need, "StreamOutTech");
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(T);
	UINT offset = 0;

	//根据发射器粒子的位置确定顶点缓存
	if (if_firstrun)
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetVertexBuffers(0, 1, &first_Vdraw_need, &stride, &offset);
	else
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetVertexBuffers(0, 1, &auto_Vinput_need, &stride, &offset);

	//设置SO管线的输出顶点缓存
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->SOSetTargets(1, &auto_Vstream0ut_need, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());

		if (if_firstrun)
		{
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->Draw(1, 0);
			if_firstrun = false;
		}
		else
		{
			d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->DrawAuto();
		}
	}
	//粒子的渲染shader
	//解绑顶点输出缓冲
	ID3D11Buffer* bufferArray[1] = { 0 };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->SOSetTargets(1, bufferArray, &offset);

	// 交换输入输出缓冲
	std::swap(auto_Vinput_need, auto_Vstream0ut_need);
	//绘制输出缓冲区生成的粒子
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->IASetVertexBuffers(0, 1, &auto_Vinput_need, &stride, &offset);
	check_error = particle_fx->get_technique(&teque_need, "DrawTech");
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex());
		d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->DrawAuto();
	}
	//撤销渲染状态
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->RSSetState(0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->GSSetShader(NULL, 0, 0);
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetDepthStencilState(0, 0);
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	d3d_pancy_basic_singleton::GetInstance()->get_d3d11_contex()->OMSetBlendState(0, blendFactor, 0xffffffff);
}
#pragma once
#include"geometry.h"
#include"pancystar_engine_basic.h"
#include"pancy_d3d11_basic.h"
#include"shader_pancy.h"
#include"PancyCamera.h"
#include"PancyInput.h"
class atmosphere_pretreatment
{
	Geometry_basic           *fullscreen_buffer;          //全屏幕平面
	Geometry_basic           *fullscreen_aobuffer;          //全屏幕平面
	ID3D11ShaderResourceView *transmittance_SRV;
	ID3D11RenderTargetView *transmittance_RTV;

	ID3D11ShaderResourceView *Irradiance_SRV;
	ID3D11RenderTargetView *Irradiance_RTV;

	ID3D11ShaderResourceView *SinglMieScattering_SRV;
	std::vector<ID3D11RenderTargetView*>SingleMieScattering_RTV;

	ID3D11ShaderResourceView *Scattering_SRV;
	std::vector<ID3D11RenderTargetView*>Scattering_RTV;


	ID3D11ShaderResourceView *DirectIrradiance_SRV;
	ID3D11RenderTargetView *DirectIrradiance_RTV;

	ID3D11ShaderResourceView *IndirectIrradiance_SRV;
	ID3D11RenderTargetView *IndirectIrradiance_RTV;

	ID3D11ShaderResourceView *MultipleScattering_SRV;
	ID3D11RenderTargetView *MultipleScattering_RTV;


	ID3D11ShaderResourceView *delta_Irradiance_SRV;
	ID3D11RenderTargetView *delta_Irradiance_RTV;

	ID3D11ShaderResourceView *delta_MieScattering_SRV;
	std::vector<ID3D11RenderTargetView*>delta_MieScattering_RTV;

	ID3D11ShaderResourceView *delta_rayleigh_scattering_SRV;
	std::vector<ID3D11RenderTargetView*> delta_rayleigh_scattering_RTV;

	ID3D11ShaderResourceView *delta_scattering_density_SRV;
	std::vector<ID3D11RenderTargetView*> delta_scattering_density_RTV;

	ID3D11ShaderResourceView *delta_multi_scattering_SRV;
	std::vector<ID3D11RenderTargetView*> delta_multi_scattering_RTV;
	ID3D11BlendState *add_blend;
public:
	atmosphere_pretreatment();
	engine_basic::engine_fail_reason create();
	void build_atomosphere_texture();
	void display(XMFLOAT3 sundir);
	void release();
private:
	engine_basic::engine_fail_reason init_texture_2D(int width_tex, int height_tex, ID3D11ShaderResourceView **SRV_input, ID3D11RenderTargetView **RTV_input);
	engine_basic::engine_fail_reason init_texture_3D(int width_tex, int height_tex, int depth_tex, ID3D11ShaderResourceView **SRV_input, std::vector<ID3D11RenderTargetView*> &RTV_input);
	engine_basic::engine_fail_reason init_texture();
	void draw_transmittance();
	void draw_irradiance();
	void draw_SingleScattering();
	void draw_Scattering_density(int layer_scattering);
	void draw_indirect_irradiance(int layer_scattering);
	void draw_multi_scattering(int layer_scattering);
};
#include"atmosphere_function.hlsli"
cbuffer per_program
{
	float4x4 luminance_from_radiance;
	uint scattering_order;
	uint layer;
};
Texture2D transmittance_texture;
Texture3D single_rayleigh_scattering_texture;
Texture3D single_mie_scattering_texture;
Texture3D multiple_scattering_texture;
Texture2D irradiance_texture;
Texture3D scattering_density_texture;
struct VertexIn
{
	float3	pos 	: POSITION;     //顶点位置
	float2  tex1    : TEXCOORD;     //顶点纹理坐标
};
struct VertexOut
{
	float4 PosH       : SV_POSITION; //渲染管线必要顶点
	float2 Tex        : TEXCOORD1;   //纹理坐标
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	float3 now_pos;
	vout.PosH = float4(vin.pos, 1.0f);
	vout.Tex = vin.tex1;
	return vout;
}
//透射率预计算
float4 PS_transmit(VertexOut pin) : SV_Target
{
	//float4 texcolor = float4(pin.Tex,1.0f,0.0f);
	//float2 uv_need = float2(1.0f - pin.Tex.y, pin.Tex.x);
	float2 uv_need = float2(pin.Tex.x, pin.Tex.y);
	//float4 texcolor = float4(ComputeTransmittanceToTopAtmosphereBoundaryTexture(ATMOSPHERE, uv_need),1.0f);
	float4 texcolor = float4(ComputeTransmittanceToTopAtmosphereBoundaryTexture(ATMOSPHERE, pin.PosH.xy), 1.0f);
	return texcolor;
}
//直接光照计算
struct PixelOut_irradiance
{
	float4 delta_irradiance     : SV_TARGET0;
	float4 irradiance           : SV_TARGET1;
};
PixelOut_irradiance PS_irradiance2(VertexOut pin)
{
	PixelOut_irradiance out_data;
	out_data.delta_irradiance = float4(ComputeDirectIrradianceTexture(ATMOSPHERE, transmittance_texture, pin.Tex),0.0f);
	out_data.irradiance = float4(0.0f,0.0f,0.0f,0.0f);
	return out_data;
};
//单层散射计算
struct PixelOut_SingleScattering
{
	float4 delta_rayleigh         : SV_TARGET0;
	float4 delta_mie              : SV_TARGET1;
	float4 scattering             : SV_TARGET2;
	float4 single_mie_scattering  : SV_TARGET3;
};
PixelOut_SingleScattering PS_SingleScattering(VertexOut pin)
{
	PixelOut_SingleScattering out_data;
	out_data.delta_rayleigh = float4(0.0f, 0.0f, 0.0f, 0.0f);
	out_data.delta_mie = float4(0.0f, 0.0f, 0.0f, 0.0f);
	ComputeSingleScatteringTexture(ATMOSPHERE, transmittance_texture, float3(pin.PosH.xy, layer + 0.5f),out_data.delta_rayleigh.xyz, out_data.delta_mie.xyz);
	out_data.scattering = float4(mul(float4(out_data.delta_rayleigh.rgb,0.0f),luminance_from_radiance).rgb,mul(float4(out_data.delta_mie.xyz,0.0f),luminance_from_radiance).r);
	out_data.single_mie_scattering = mul(float4(out_data.delta_mie.xyz,0.0f),luminance_from_radiance);
	return out_data;
};
//散射密度
float4 PS_ScatteringDensity(VertexOut pin) : SV_Target
{
	float4 scattering_density = float4(0.0f,0.0f,0.0f,0.0f);
	scattering_density.xyz = ComputeScatteringDensityTexture(
		ATMOSPHERE, transmittance_texture, single_rayleigh_scattering_texture,
		single_mie_scattering_texture, multiple_scattering_texture,
		irradiance_texture, float3(pin.PosH.xy, layer + 0.5),
		scattering_order);
	return scattering_density;
}
//间接光照
struct PixelOut_Indirectirradiance
{
	float4 delta_irradiance     : SV_TARGET0;
	float4 irradiance           : SV_TARGET1;
};
PixelOut_Indirectirradiance PS_Indirecirradiance(VertexOut pin)
{
	PixelOut_Indirectirradiance out_data;
	out_data.delta_irradiance = float4(ComputeIndirectIrradianceTexture(ATMOSPHERE, single_rayleigh_scattering_texture,single_mie_scattering_texture, multiple_scattering_texture, pin.Tex, scattering_order),1.0f);
	out_data.irradiance = mul(float4(out_data.delta_irradiance.xyz,0.0f),luminance_from_radiance);
	return out_data;
};
//多层散射效果
struct PixelOut_MultipleScattering
{
	float4 delta_multiple_scattering     : SV_TARGET0;
	float4 scattering                    : SV_TARGET1;
};
PixelOut_MultipleScattering PS_MultipleScattering(VertexOut pin)
{
	PixelOut_MultipleScattering out_data;
	float nu;
	out_data.delta_multiple_scattering = float4(ComputeMultipleScatteringTexture(ATMOSPHERE, transmittance_texture, scattering_density_texture,float3(pin.PosH.xy, layer + 0.5), nu),0.0f);
	out_data.scattering = mul(float4(out_data.delta_multiple_scattering.rgb / RayleighPhaseFunction(nu),0.0),luminance_from_radiance);
	return out_data;
};
technique11 draw_transmit
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_transmit()));
	}
}
technique11 draw_direct_irradiance
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_irradiance2()));
	}
}
technique11 draw_SingleScattering
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_SingleScattering()));
	}
}
technique11 draw_ScatteringDensity
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_ScatteringDensity()));
	}
}
technique11 draw_Indirectirradiance
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Indirecirradiance()));
	}
}
technique11 draw_MultipleScattering
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_MultipleScattering()));
	}
}
#include"HDR_define.hlsli"
cbuffer perframe
{
	uint4                   input_range;
	float4                  light_average;//平均像素信息(平均亮度，高光分界线，高光最高强度，tonemapping系数)
};
cbuffer always
{
	float4x4                YUV2RGB;
	float4x4                RGB2YUV;
};
Texture2D               input_tex;
Texture2D               input_bloom;
StructuredBuffer<float> input_buffer;
DepthStencilState NoDepthWrites
{
	DepthEnable = TRUE;
	DepthWriteMask = ZERO;
};
SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};
SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
struct VertexIn//普通顶点
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
	//因为是前一个shader光栅化完毕的像素点，不需要做任何变化
	vout.PosH = float4(vin.pos, 1.0f);
	//记录下纹理坐标
	vout.Tex = vin.tex1;
	return vout;
}




float4 PS(VertexOut Input) : SV_TARGET
{
	//颜色采样
	float4 input_texcolor = input_tex.Sample(samTex_liner, Input.Tex);
	float4 color_lum = float4(0.0f, 0.0f, 0.0f, 1.0f);
	//计算平均亮度
	float light_avege_lum = 0.0f;
	for (uint i = 0; i < input_range.z; ++i)
	{
		light_avege_lum += input_buffer[i];
	}
	light_avege_lum = exp(light_avege_lum);
	//获取高光bllom
	float4 input_bloomcolor = input_bloom.Sample(samTex_liner, Input.Tex);
	//计算最终颜色
	float4 finalcolor = float4(ACESFitted(input_texcolor, light_avege_lum)+ 0.6f*input_bloomcolor , input_texcolor.a);
	//return finalcolor;
	//return input_texcolor;
	//finalcolor = float4(float3(1.0, 1.0, 1.0) - exp(-input_texcolor.rgb / 1 * 10),1.0f);
	//gamma校正
	return finalcolor;
	//return float4(pow(input_texcolor.rgb, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2)), 1.0f);
}
technique11 draw_HDRfinal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		//SetDepthStencilState(NoDepthWrites, 0);
	}
}
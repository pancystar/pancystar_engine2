Texture2D gInputImage;
Texture2D gInputReflect;
cbuffer cbPerFrame
{
	float4 tex_range_color_normal;
};
SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};
struct VertexIn//普通顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
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
	vout.PosH = float4(vin.pos, 1.0f);
	vout.Tex = vin.tex1;
	return vout;
}
float4 PS(VertexOut pin) : SV_Target
{
	float4 reflect_color = gInputReflect.SampleLevel(samInputImage,pin.Tex,0);
	float reflectance = 0.3f;
	//float reflectance = 0.6f*reflect_color.a;
	float accept_range = 0.2f, final_count = 0.0f;
	float4 final_color = (1.0f - reflectance) * gInputImage.SampleLevel(samInputImage, pin.Tex, 0) + reflectance * reflect_color;
	final_color.a = 1.0f;
	return final_color;
}
technique11 blend_reflect
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
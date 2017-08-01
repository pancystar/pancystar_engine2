Texture2D gInputImage;
Texture2DArray gInputReflect;
Texture2D gInputAO;
Texture2D gInputnormal_metallic;
Texture2D gInputspecular_roughness;
Texture2D gInputbrdf;
Texture2D gInput_albedo_nov;
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
SamplerState samTex
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
	AddressU = WRAP;
	AddressV = WRAP;
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
	/*
	//采样AO
	float tex_ao = gInputAO.SampleLevel(samTex_liner, pin.Tex, 0).r;
	//采样金属度及粗糙度
	float tex_matallic = gInputnormal_metallic.SampleLevel(samTex_liner, pin.Tex, 0).a;
	float4 specular_roughness = gInputspecular_roughness.SampleLevel(samTex_liner, pin.Tex, 0);
	float4 SpecularColor = float4(specular_roughness.rgb,1.0f);
	float  tex_roughness = specular_roughness.a;
	//采样albedo与实现夹角
	float4 tex_albedo_nov = gInput_albedo_nov.SampleLevel(samTex, pin.Tex, 0);
	float4 tex_albedo_in = float4(tex_albedo_nov.rgb, 1.0f);
	float NoV = tex_albedo_nov.a;
	float4 EnvBRDF = gInputbrdf.Sample(samTex_liner,float2(tex_roughness,NoV));
	//采样环境光
	float index = tex_roughness * 4;
	float4 enviornment_color = gInputReflect.Sample(samTex_liner, float3(pin.Tex, index));
	float4 enviornment_color_diffuse = gInputReflect.Sample(samTex_liner, float3(pin.Tex, 3));
	//计算环境光反射
	float4 ambient_diffuse = tex_ao * (0.8f) *(1.0f - tex_matallic) * tex_albedo_in;
	float4 ambient_specular = 1.5f * tex_ao * (0.8f*enviornment_color + 0.2f*tex_albedo_in) * (SpecularColor * EnvBRDF.x + EnvBRDF.y);
	*/


	//float4 reflect_color = gInputReflect.SampleLevel(samInputImage,pin.Tex,0);
	//float reflectance = 0.4f;
	//float reflectance = 0.6f*reflect_color.a;
	//float accept_range = 0.2f, final_count = 0.0f;
	//float4 final_color = (1.0f - reflectance) * gInputImage.SampleLevel(samInputImage, pin.Tex, 0) + reflectance * reflect_color;
	//final_color.a = 1.0f;
	float4 pre_color = gInputImage.SampleLevel(samInputImage, pin.Tex, 0);
	//float4 final_color = ambient_diffuse + ambient_specular + pre_color - 0.4f*tex_ao * tex_albedo_in;
	float4 reflect_color = gInputReflect.Sample(samTex_liner, float3(pin.Tex, 0));
	return pre_color;
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
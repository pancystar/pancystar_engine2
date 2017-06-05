cbuffer PerFrame
{
	float4 UI_scal;        //UI所占的大小(长，宽，深度，额外)
	float4 UI_pos;         //UI所在的位置
};
Texture2D texture_need;
SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
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
	now_pos = vin.pos * UI_scal.xyz;
	now_pos = now_pos + UI_pos.xyz;
	vout.PosH = float4(now_pos, 1.0f);
	vout.Tex = vin.tex1;
	return vout;
}
float4 PS(VertexOut pin) : SV_Target
{
	float2 tex_uv = pin.Tex;
	float4 texcolor = texture_need.Sample(samTex_liner, tex_uv);
	return texcolor;
}
technique11 draw_ui
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
Texture2D depth_input;
float3 cube_count;
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
	float4 PosH     : SV_POSITION; //渲染管线必要顶点
	float2 tex      : TEXCOORD;     //顶点纹理坐标
};
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// Set a very far depth value if sampling outside of the NormalDepth map
	// so we do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(1e5f, 0.0f, 0.0f, 1e5f);
};
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	//顶点的投影坐标(用于光栅化)
	vout.PosH = float4(vin.pos, 1.0f);
	//定点的纹理坐标(用于msaa采样)
	vout.tex = vin.tex1;
	return vout;
}
//----------------------------------------------------------------------------------
float2 PS(VertexOut IN) : SV_TARGET
{
	float2 final_pixel;
	final_pixel.r = cube_count.r;
	final_pixel.g = depth_input.SampleLevel(samNormalDepth, IN.tex, 0).r;
	return final_pixel;
}
technique11 resolove_alpha
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

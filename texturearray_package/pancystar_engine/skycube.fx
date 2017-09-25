cbuffer PerFrame
{
	float4x4 world_matrix;      //世界变换
	float4x4 normal_matrix;     //法线变换
	float4x4 final_matrix;      //总变换
	float4x4 textureproj_matrix;//纹理投影变换
	float3   position_view;     //视点位置
};
TextureCube texture_cube;
Texture2D atmosphere_mask;           //大气散射掩码
SamplerState samTex
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};
struct Vertex_IN//含法线贴图顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float3	tangent : TANGENT;      //顶点切向量
	uint4   texid   : TEXINDICES;   //纹理索引
	float4  tex1    : TEXDIFFNORM;  //顶点纹理坐标(漫反射及法线纹理)
	float4  tex2    : TEXOTHER;     //顶点纹理坐标(其它纹理)
};
struct VertexOut
{
	float4 position      : SV_POSITION;    //变换后的顶点坐标
	float3 normal        : TEXCOORD0;      //变换后的法向量
	float3 position_bef	 : TEXCOORD1;      //变换前的顶点坐标
	float4 pos_texproj   : TEXCOORD2;      //阴影顶点坐标
};
RasterizerState DisableCulling
{
	CullMode = FRONT;
};
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.position_bef = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	vout.normal = normalize(mul(float4(vin.normal, 0.0f), normal_matrix)).xyz;
	vout.pos_texproj = mul(float4(vout.position_bef, 1.0f), textureproj_matrix);
	return vout;
}
float4 PS(VertexOut pin) :SV_TARGET
{
	float4 tex_color = float4(0.0f,0.0f,0.0f,0.0f);
	float4 color_fog = float4(0.75f, 0.75f, 0.75f, 1.0f);
	float3 view_direct = normalize(pin.position_bef - position_view);
	float3 map_direct = view_direct.xyz;//视线向量
	
	tex_color = texture_cube.Sample(samTex, map_direct);
	pin.pos_texproj /= pin.pos_texproj.w;
	float4 atomosphere_color = atmosphere_mask.Sample(samTex, pin.pos_texproj.xy);
	return tex_color;
}

technique11 draw_sky
{
	Pass p0
	{
		SetVertexShader(CompileShader(vs_5_0,VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetRasterizerState(DisableCulling);
	}
}


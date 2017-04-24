float4x4         final_matrix;     //总变换
struct Vertex_IN
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float3	tangent : TANGENT;      //顶点切向量
	uint4   texid   : TEXINDICES;   //纹理索引
	float2  tex1    : TEXCOORD;     //顶点纹理坐标
};
struct VertexOut
{
	float4 position      : SV_POSITION;    //变换后的顶点坐标
	float3 color         : NORMAL;    //变换后的顶点坐标
};
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	if (vin.normal.r < 0.0f && vin.normal.g < 0.0f && vin.normal.b < 0.0f)
	{
		vin.normal.r = 1.0f;
		vin.normal.b = 1.0f;
	}
	if (vin.normal.r < 0.0f)
	{
		vin.normal.r = 1.0f;
	}
	if (vin.normal.g < 0.0f)
	{
		vin.normal.g = 1.0f;
	}
	if (vin.normal.b < 0.0f)
	{
		vin.normal.b = 1.0f;
	}vout.color = vin.normal.xyz;
	return vout;
}
float4 PS(VertexOut pin) :SV_TARGET
{
	return float4(pin.color,1.0f);
}
technique11 colorTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
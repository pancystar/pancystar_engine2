cbuffer PerFrame
{
	float4x4 world_matrix;      //世界变换
	float4x4 normal_matrix;     //法线变换
	float4x4 final_matrix;      //总变换
	float4x4 world_matrix_array[300];
	float4x4 view_proj_matrix;
};
Texture2DArray   texture_pack_array;  //贴图包
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
	float3	tangent : TANGENT;      //顶点切向量
	uint4   texid   : TEXINDICES;   //纹理索引
	float4  tex1    : TEXDIFFNORM;  //顶点纹理坐标(漫反射及法线纹理)
	float4  tex2    : TEXOTHER;     //顶点纹理坐标(其它纹理)
};
struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float3 PosV       : POSITION;
	float3 NormalV    : NORMAL;
	float3 tangent    : TANGENT;
	uint4   texid     : TEXINDICES;   //纹理索引
	float4  tex1      : TEXDIFFNORM;  //顶点纹理坐标(漫反射及法线纹理)
	float4  tex2      : TEXOTHER;     //顶点纹理坐标(其它纹理)
};
struct Vertex_IN_instance
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float3	tangent : TANGENT;      //顶点切向量
	uint4   texid   : TEXINDICES;   //纹理索引
	float4  tex1    : TEXDIFFNORM;  //顶点纹理坐标(漫反射及法线纹理)
	float4  tex2    : TEXOTHER;     //顶点纹理坐标(其它纹理)
	uint InstanceId : SV_InstanceID;//instace索引号
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosV = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	vout.NormalV = mul(float4(vin.normal, 0.0f), normal_matrix).xyz;
	vout.tangent = mul(float4(vin.tangent, 0.0f), normal_matrix).xyz;
	vout.PosH = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}
VertexOut VS_instance(Vertex_IN_instance vin)
{
	VertexOut vout;
	vout.PosV = mul(float4(vin.pos, 1.0f), world_matrix_array[vin.InstanceId]).xyz;
	vout.NormalV = mul(float4(vin.normal, 0.0f), world_matrix_array[vin.InstanceId]).xyz;
	vout.tangent = mul(float4(vin.tangent, 0.0f), world_matrix_array[vin.InstanceId]).xyz;
	vout.PosH = mul(float4(vout.PosV, 1.0f), view_proj_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}
float4 PS(VertexOut pin) : SV_Target
{
	float texID_data_diffuse = pin.texid.x;
	float4 tex_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex1.xy, texID_data_diffuse));
	clip(tex_color.a - 0.5f);
	pin.NormalV = normalize(pin.NormalV);
	return float4(pin.NormalV, 10.0f);
}
float4 PS_withnormal(VertexOut pin) : SV_Target
{
	float texID_data_diffuse = pin.texid.x;
	float texID_data_normal = pin.texid.y;
	float4 tex_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex1.xy, texID_data_diffuse));
	clip(tex_color.a - 0.5f);
	pin.NormalV = normalize(pin.NormalV);
	pin.tangent = normalize(pin.tangent);
	//求解图片所在自空间->模型所在统一世界空间的变换矩阵
	float3 N = pin.NormalV;
	float3 T = normalize(pin.tangent - N * pin.tangent * N);
	float3 B = cross(N, T);
	float3x3 T2W = float3x3(T, B, N);
	float3 normal_map = texture_pack_array.Sample(samTex_liner, float3(pin.tex1.zw, texID_data_normal)).rgb;//从法线贴图中获得法线采样
	//float3 normal_map = texture_normal.Sample(samTex_liner, pin.tex).rgb;
	normal_map = 2 * normal_map - 1;                               //将向量从图片坐标[0,1]转换至真实坐标[-1,1]  
	normal_map = normalize(mul(normal_map, T2W));                  //切线空间至世界空间
	pin.NormalV = normal_map;
	return float4(normal_map, 10.0);
}
technique11 NormalDepth
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
technique11 NormalDepth_withinstance
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
technique11 NormalDepth_withinstance_normal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withnormal()));
	}
}
technique11 NormalDepth_withnormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withnormal()));
	}
}
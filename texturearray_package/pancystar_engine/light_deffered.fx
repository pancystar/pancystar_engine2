/*延迟光照效果*/
#include"light_count.hlsli"
cbuffer perobject
{
	pancy_material   material_need;    //材质
	float4x4         world_matrix;     //世界变换
	float3           position_view;         //视点位置
	float4x4         final_matrix;     //总变换
	float4x4         ssao_matrix;      //ssao变换
	float4x4         world_matrix_array[300];
	float4x4         view_proj_matrix;
};
Texture2D        texture_light_diffuse;      //漫反射光照贴图
Texture2D        texture_light_specular;     //镜面反射光照贴图
Texture2DArray        texture_pack_array;         //纹理打包贴图
Texture2D        texture_ssao;               //ssao贴图
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
struct Vertex_IN//含法线贴图顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float3	tangent : TANGENT;      //顶点切向量
	uint4   texid   : TEXINDICES;   //纹理索引
	float4  tex1    : TEXDIFFNORM;  //顶点纹理坐标(漫反射及法线纹理)
	float4  tex2    : TEXOTHER;     //顶点纹理坐标(其它纹理)
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

struct VertexOut
{
	float3 position_before : POSITION;
	float4 position        : SV_POSITION;    //变换后的顶点坐标
	uint4   texid          : TEXINDICES;     //纹理索引
	float4  tex1           : TEXDIFFNORM;    //顶点纹理坐标(漫反射及法线纹理)
	float4  tex2           : TEXOTHER;       //顶点纹理坐标(其它纹理)
	float4 pos_ssao        : POSITION1;      //阴影顶点坐标
};
struct PixelOut
{
	float4 final_color;
	float4 reflect_message;
};

VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position_before = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	vout.pos_ssao = mul(float4(vout.position_before, 1.0f), ssao_matrix);
	return vout;
}
VertexOut VS_instance(Vertex_IN_instance vin)
{
	VertexOut vout;
	vout.position_before = mul(float4(vin.pos, 1.0f), world_matrix_array[vin.InstanceId]).xyz;
	vout.position = mul(float4(vout.position_before, 1.0f), view_proj_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	vout.pos_ssao = mul(float4(vout.position_before, 1.0f), ssao_matrix);
	return vout;
}
PixelOut PS(VertexOut pin) :SV_TARGET
{
	pin.pos_ssao /= pin.pos_ssao.w;
	float texID_data_diffuse = pin.texid.x;
	float4 tex_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex1.xy, texID_data_diffuse));
	//float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.6f);
	float4 ambient = 0.5f*float4(1.0f, 1.0f, 1.0f, 0.0f) * texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	float4 diffuse = material_need.diffuse * texture_light_diffuse.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);      //漫反射光
	float4 spec = material_need.specular * texture_light_specular.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);       //镜面反射光
	float4 final_color = tex_color *(ambient + diffuse) + spec;
	final_color.a = tex_color.a;

	PixelOut ans_pix;
	ans_pix.final_color = final_color;
	ans_pix.reflect_message = material_need.reflect;
	return ans_pix;
}
technique11 LightTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
technique11 LightTech_instance
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
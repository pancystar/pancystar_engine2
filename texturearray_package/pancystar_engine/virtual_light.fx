#include<light_count.hlsli>
float4x4         world_matrix;     //世界变换
float4x4         normal_matrix;    //法线变换
float4x4         final_matrix;     //总变换
float4x4         ssao_matrix;      //ssao变换

Texture2D        texture_diffuse;      //漫反射贴图
Texture2D        texture_specular;     //镜面反射贴图
Texture2D        texturet_normal;      //法线贴图

Texture2D        texture_ssao;      //ao贴图

Texture2DArray   texture_pack_diffuse;


SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
struct Vertex_IN
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
	float3 normal        : NORMAL;         //变换后的法向量
	float3 tangent       : TANGENT;        //顶点切向量
	uint4  texid         : TEXINDICES;     //纹理索引
	float4  tex1         : TEXDIFFNORM;  //顶点纹理坐标(漫反射及法线纹理)
	float4  tex2         : TEXOTHER;     //顶点纹理坐标(其它纹理)
	float3 position_bef  : POSITION;       //变换前的顶点坐标
	float4 pos_ssao      : AOPOSITION;      //阴影顶点坐标
};
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.normal = mul(float4(vin.normal, 0.0f), normal_matrix).xyz;
	vout.tangent = mul(float4(vin.tangent, 0.0f), normal_matrix).xyz;
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	vout.position_bef = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	vout.pos_ssao = mul(float4(vout.position_bef, 1.0f), ssao_matrix);
	return vout;
}
float4 PS(VertexOut pin) :SV_TARGET
{
	float4 A,D,S;
	pancy_light_basic light_dir;
	light_dir.ambient = float4(0.5f, 0.5f, 0.5f,1.0f);
	light_dir.diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
	light_dir.specular = float4(0.5f, 0.5f, 0.5f, 1.0f);
	light_dir.dir = float3(1.0f,0.0f, 0.0f);

	pancy_material material_need;
	material_need.ambient = float4(1.0f, 1.0f, 1.0f, 1.0f);
	material_need.diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
	material_need.specular = float4(1.0f, 1.0f, 1.0f, 1.0f);
	material_need.reflect = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float3 normal_need = normalize(pin.normal);
	float3 view_dir = float3(0.0f, 0.0f, 1.0f);

	compute_dirlight(material_need, light_dir, normal_need, view_dir,A,D,S);
	float4 tex_color = texture_diffuse.Sample(samTex_liner,pin.tex1.xy);
	float4 final_color = tex_color *(A + D) + S;
	return final_color;
}
float4 PS_array(VertexOut pin) :SV_TARGET
{
	pin.pos_ssao /= pin.pos_ssao.w;
	float4 A,D,S;
	pancy_light_basic light_dir;
	light_dir.ambient = float4(0.5f, 0.5f, 0.5f,1.0f);
	light_dir.diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
	light_dir.specular = float4(0.5f, 0.5f, 0.5f, 1.0f);
	light_dir.dir = float3(1.0f,0.0f, 0.0f);

	pancy_material material_need;
	material_need.ambient = float4(1.0f, 1.0f, 1.0f, 1.0f);
	material_need.diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
	material_need.specular = float4(1.0f, 1.0f, 1.0f, 1.0f);
	material_need.reflect = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float3 normal_need = normalize(pin.normal);
	float3 view_dir = float3(0.0f, 0.0f, 1.0f);
	compute_dirlight(material_need, light_dir, normal_need, view_dir,A,D,S);
	float texID_data = pin.texid.x;
	if (texID_data > 1.0f) 
	{
		texID_data *= 1.001;
	}
	float4 tex_color = texture_pack_diffuse.Sample(samTex_liner, float3(pin.tex1.xy, texID_data));
	float4 ao_color = texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
//	if (texID_data < 1.5f) 
//	{
//		tex_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
//	}
	
	float4 final_color = ao_color *A + D + S;
	return final_color;
}
technique11 light_tech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
technique11 light_tech_array
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_array()));
	}
}
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
cbuffer perframe
{
	float4x4         view_matrix;    //取景变换
	float4x4         invview_matrix; //取景变换逆变换
};


Texture2D        texture_light_diffuse;      //漫反射光照贴图
Texture2D        texture_light_specular;     //镜面反射光照贴图
Texture2DArray        texture_pack_array;         //纹理打包贴图
Texture2D        texture_ssao;               //ssao贴图
Texture2D        gNormalspecMap;        //屏幕空间法线&镜面反射强度纹理
Texture2D        gInputspecular_roughness;
Texture2D        gInputbrdf;
TextureCube      IBL_cube;
//TextureCube      IBL_cube;
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
	float3 position_view   : VPOSITION;
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
	vout.position_view = mul(float4(vout.position_before, 1.0f), view_matrix).xyz;
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
	vout.position_view = mul(float4(vout.position_before, 1.0f), view_matrix).xyz;
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
	//float4 ambient = 0.4f* texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	float tex_ao = texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	//采集法线与金属度
	float4 normalmetallic = gNormalspecMap.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);
	float3 normal_dir = normalize(normalmetallic.rgb);
	float tex_matallic = normalmetallic.a;
	//将法线变换到世界空间
	normal_dir = mul(float4(normal_dir,0.0f), invview_matrix);
	//计算反射线
	float3 view_dir = normalize(position_view - pin.position_before);
	float NoV = dot(view_dir, normal_dir);
	float3 reflect_dir = normalize(reflect(-view_dir, normal_dir));
	//采集镜面光与粗糙度
	float4 specular_roughness = gInputspecular_roughness.SampleLevel(samTex_liner, pin.pos_ssao.xy, 0);
	float4 SpecularColor = float4(specular_roughness.rgb, 1.0f);
	float  tex_roughness = specular_roughness.a;
	//采集brdf
	float4 EnvBRDF = gInputbrdf.Sample(samTex_liner, float2(tex_roughness, NoV));
	//采样环境光
	uint index = tex_roughness * 6;
	float4 enviornment_color = IBL_cube.SampleLevel(samTex_liner, reflect_dir, index);
	//计算环境光反射
	float4 ambient_diffuse = 0.4f*tex_ao *(1.0f - tex_matallic) * tex_color;
	float4 ambient_specular = 1.0f*tex_ao * (0.6f*enviornment_color + 0.4f*tex_color) * (SpecularColor * EnvBRDF.x + EnvBRDF.y);
	float4 ambient = ambient_diffuse + ambient_specular;

	//float4 ambient = 0;
	float4 diffuse = texture_light_diffuse.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);      //漫反射光
	float4 spec =texture_light_specular.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);       //镜面反射光
	float4 final_color = ambient + tex_color * diffuse + spec;
	final_color.a = tex_color.a;

	/*
	//获取金属度及粗糙度材质
	float texID_data_metallic = pin.texid.z;//金属度纹理ID
	float texID_data_roughness = pin.texid.w;//粗糙度纹理ID
	float metallic_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex2.xy, texID_data_metallic)).r;
	float roughness_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex2.zw, texID_data_roughness)).r;
	//final_color = float4(metallic_color, metallic_color, metallic_color,1);
	//final_color = float4(roughness_color, roughness_color, roughness_color, 1);
	*/
	PixelOut ans_pix;
	ans_pix.final_color = final_color;

	//float3 normal_dir = normalize(gNormalspecMap.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).rgb);
	//float3 position_view_sun = float3(0.0f, 0.0f, 0.0f);
	//float3 view_dir = normalize(position_view_sun - pin.position_view);

	ans_pix.reflect_message.rgb = tex_color.rgb;
	ans_pix.reflect_message.r = 0.5;
	return ans_pix;
}
PixelOut PS_withputao(VertexOut pin) :SV_TARGET
{
	pin.pos_ssao /= pin.pos_ssao.w;
	float texID_data_diffuse = pin.texid.x;
	float4 tex_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex1.xy, texID_data_diffuse));
	//float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.6f);
	float4 ambient = 0.8f*float4(1.0f, 1.0f, 1.0f, 0.0f);
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
technique11 LightTech_withoutao
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withputao()));
	}
}
technique11 LightTech_instance_withoutao
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withputao()));
	}
}
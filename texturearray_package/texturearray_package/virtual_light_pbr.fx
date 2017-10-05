#include<light_count.hlsli>
float3           position_view;    //视点位置
float4x4         world_matrix;     //世界变换
float4x4         normal_matrix;    //法线变换
float4x4         final_matrix;     //总变换

Texture2D        texture_diffuse;      //漫反射贴图
Texture2D        texture_specular;     //镜面反射贴图
Texture2D        texturet_normal;      //法线贴图

Texture2D        texture_metallic;     //金属度贴图
Texture2D        texturet_roughness;   //粗糙度贴图
Texture2D        texture_rdfluv;   //粗糙度贴图

Texture2DArray   texture_pack_diffuse;
TextureCube      texture_environment;


SamplerState samTex_liner
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 16;
	AddressU = Wrap;
	AddressV = Wrap;
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
};
void count_pbr_reflect(
	float4 tex_albedo_in,
	float  tex_matallic,
	float  tex_roughness,
	float3 light_dir_in,
	float3 normal,
	float3 direction_view,
	float diffuse_angle,
	out float4 diffuse_out,
	out float4 specular_out
	)
{
	float4 F0 = lerp(0.04, tex_albedo_in, tex_matallic);
	float3 h_vec = normalize((light_dir_in + direction_view) / 2.0f);
	diffuse_out = tex_albedo_in * (1 - tex_matallic);

	float pi = 3.141592653;
	//float diffuse_angle = dot(light_dir_in, normal); //漫反射夹角
	float view_angle = dot(direction_view, normal);//视线夹角
	float cos_vh = dot(direction_view, h_vec);
	float4 fresnel = F0 + (float4(1.0f,1.0f,1.0f,1.0f) - F0)*(1.0f - pow(cos_vh, 5.0f));//菲涅尔项;
	
	//float4 fresnel = F0 + (float4(1.0f, 1.0f, 1.0f, 1.0f) - F0) * pow(2, ((-5.55473*cos_vh - 6.98316)*cos_vh));
	//NDF法线扰乱项
	float alpha = tex_roughness * tex_roughness;
	float nh_mul = dot(normal, h_vec);
	float divide_ndf1 = nh_mul*nh_mul * (alpha * alpha - 1.0f) + 1.0f;
	float divide_ndf2 = pi * divide_ndf1 *divide_ndf1;
	float ndf = (alpha*alpha) / divide_ndf2;
	//GGX遮挡项
	
	float ggx_k = (tex_roughness + 1.0f) * (tex_roughness + 1.0f) / 8.0f;
	float ggx_v = view_angle / (view_angle*(1 - ggx_k) + ggx_k);
	float ggx_l = diffuse_angle / (diffuse_angle*(1 - ggx_k) + ggx_k);
	float ggx = ggx_v * ggx_l;
	float3 v = reflect(light_dir_in, normal);
	float blin_phong = pow(max(dot(v, direction_view), 0.0f),10);
	//最终的镜面反射项
	specular_out = (fresnel * ndf * ggx) / (4 * view_angle * diffuse_angle);
	//specular_out = fresnel * ndf *blin_phong / (4 * view_angle * diffuse_angle);
}
void compute_dirlight_2(
	float4 tex_albedo_in,
	float  tex_matallic,
	float  tex_roughness,
	pancy_light_basic light_dir,
	float3 normal,
	float3 direction_view,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	float4 mat_diffuse;
	float4 mat_specular_pre;

	float4 SpecularColor = lerp(0.04, tex_albedo_in, tex_matallic);
	//float4 SpecularColor = float4(tex_matallic, tex_matallic, tex_matallic,1.0f);
	
	float NoV = saturate(dot(normal, direction_view)-0.01);
	float4 EnvBRDF = texture_rdfluv.Sample(samTex_liner,float2(tex_roughness,NoV));
	
	//texture_rdfluv
	float diffuse_angle = dot(-light_dir.dir, normal); //漫反射夹角
	count_pbr_reflect(tex_albedo_in, tex_matallic, tex_roughness, -light_dir.dir, normal, direction_view, diffuse_angle, mat_diffuse, mat_specular_pre);

	float3 reflect_direct = normalize(reflect(-direction_view, normal));
	float3 map_direct = reflect_direct.xyz;//视线向量

	uint index = tex_roughness * 6;
	float4 enviornment_color = texture_environment.SampleLevel(samTex_liner, map_direct, index);

	ambient =0 ;
	//ambient *= 0.5f;
	//ambient = enviornment_color * tex_albedo_in * (EnvBRDF.x + EnvBRDF.y);         //环境光
	float4 ambient_diffuse = 0.8f*(1.0f - tex_matallic) * tex_albedo_in;
	//float4 ambient_diffuse = enviornment_color*0.3f * tex_albedo_in;
	float4 ambient_specular = (0.6f*enviornment_color+ 0.4f*tex_albedo_in) * (SpecularColor * EnvBRDF.x + EnvBRDF.y);
	ambient = (ambient_diffuse + ambient_specular);
	if (diffuse_angle > 0.0f)
	{
		//diffuse = 0.0f;//漫反射光
		//spec = 0.0f;;    //镜面反射光
		diffuse = diffuse_angle * mat_diffuse * light_dir.diffuse;//漫反射光
		spec = diffuse_angle * mat_specular_pre * light_dir.specular;    //镜面反射光
	}
	else
	{
		diffuse = 0.0f;//漫反射光
		spec = 0.0f;;    //镜面反射光
	}
}

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
	return vout;
}
float4 PS_pbr(VertexOut pin) :SV_TARGET
{
	float4 A,D,S;
	pancy_light_basic light_dir;
	light_dir.ambient = float4(1.0f, 1.0f, 1.0f,1.0f);
	light_dir.diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
	light_dir.specular = float4(1.5f, 1.5f, 1.5f, 1.0f);
	light_dir.dir = normalize(float3(1.0f,0.0f, 0.0f));

	float4 tex_albedo =   texture_diffuse.Sample(samTex_liner, pin.tex1.xy);
	float tex_metallic =  texture_metallic.Sample(samTex_liner, pin.tex1.xy).r;
	float tex_roughness = texturet_roughness.Sample(samTex_liner, pin.tex1.xy).r;

	float3 normal_need = normalize(pin.normal);
	float3 view_dir = normalize(position_view - pin.position_bef);

	compute_dirlight_2(tex_albedo, tex_metallic, tex_roughness, light_dir, normal_need, view_dir,A,D,S);
	float4 final_color =A + D + S;

	/*
	light_dir.ambient = light_dir.ambient;
	light_dir.diffuse = 0.3*light_dir.diffuse;
	light_dir.specular = 0.3*light_dir.specular;
	light_dir.dir = normalize(float3(-1.0f, 0.0f, 0.0f));
	compute_dirlight_2(tex_albedo, tex_metallic, tex_roughness, light_dir, normal_need, view_dir, A, D, S);
	final_color += A + D + S;
	
	light_dir.dir = normalize(float3(0.0f, 0.0f, 1.0f));
	compute_dirlight_2(tex_albedo, tex_metallic, tex_roughness, light_dir, normal_need, view_dir, A, D, S);
	final_color += A + D + S;

	light_dir.dir = normalize(float3(0.0f, 0.0f, -1.0f));
	compute_dirlight_2(tex_albedo, tex_metallic, tex_roughness, light_dir, normal_need, view_dir, A, D, S);
	final_color += A + D + S;

	light_dir.dir = normalize(float3(0.0f, 1.0f, 0.0f));
	compute_dirlight_2(tex_albedo, tex_metallic, tex_roughness, light_dir, normal_need, view_dir, A, D, S);
	final_color += A + D + S;

	light_dir.dir = normalize(float3(0.0f, -1.0f, 0.0f));
	compute_dirlight_2(tex_albedo, tex_metallic, tex_roughness, light_dir, normal_need, view_dir, A, D, S);
	final_color += A + D + S;
	*/
	return final_color;
}
float4 PS(VertexOut pin) :SV_TARGET
{
	float4 A,D,S;
	pancy_light_basic light_dir;
	light_dir.ambient = float4(0.5f, 0.5f, 0.5f, 1.0f);
	light_dir.diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
	light_dir.specular = float4(0.5f, 0.5f, 0.5f, 1.0f);
	light_dir.dir = float3(1.0f, 0.0f, 0.0f);

	pancy_material material_need;
	material_need.ambient = float4(1.0f, 1.0f, 1.0f, 1.0f);
	material_need.diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
	material_need.specular = float4(1.0f, 1.0f, 1.0f, 20.0f);
	material_need.reflect = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float3 normal_need = normalize(pin.normal);
	float3 view_dir = normalize(position_view - pin.position_bef);

	compute_dirlight(material_need, light_dir, normal_need, view_dir, A, D, S);
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex1.xy);
	float4 final_color = tex_color *(A + D) + S;
	return final_color;
}
float4 PS_array(VertexOut pin) :SV_TARGET
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
	float texID_data = pin.texid.y;
	if (texID_data > 1.0f) 
	{
		texID_data *= 1.001;
	}
	float4 tex_color = texture_pack_diffuse.Sample(samTex_liner, float3(pin.tex1.zw, texID_data));
//	if (texID_data < 1.5f) 
//	{
//		tex_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
//	}
	
	float4 final_color = tex_color *(A + D) + S;
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
technique11 light_tech_pbr
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_pbr()));
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
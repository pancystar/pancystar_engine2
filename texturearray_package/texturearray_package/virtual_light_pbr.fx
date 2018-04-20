#include<light_count.hlsli>
float3           position_view;    //�ӵ�λ��
float4x4         world_matrix;     //����任
float4x4         normal_matrix;    //���߱任
float4x4         final_matrix;     //�ܱ任

float4x4 gBoneTransforms[100];//�����任����

Texture2D        texture_diffuse;      //��������ͼ
Texture2D        texture_specular;     //���淴����ͼ
Texture2D        texturet_normal;      //������ͼ

Texture2D        texture_metallic;     //��������ͼ
Texture2D        texturet_roughness;   //�ֲڶ���ͼ
Texture2D        texture_rdfluv;   //�ֲڶ���ͼ

Texture2DArray   texture_pack_diffuse;
Texture2DArray   texture_pack_normal;
Texture2DArray   texture_pack_metallic;
Texture2DArray   texture_pack_roughness;
BlendState CommonBlending
{
	AlphaToCoverageEnable = TRUE;
	BlendEnable[0] = FALSE;
};
TextureCube      texture_environment;
struct mesh_anim 
{
	float3 pos;
	float3 norm;
	float3 tangent;
};
StructuredBuffer<mesh_anim> input_point;
uint4 offset_num;

SamplerState samTex_liner
{
	//Filter = MIN_MAG_MIP_LINEAR;
	Filter = ANISOTROPIC;
	MaxAnisotropy = 16;
	AddressU = wrap;
	AddressV = wrap;
};
RasterizerState DisableCulling
{
	CullMode = FRONT;
};
struct Vertex_IN_bone
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float4  tex1    : TEXDIFFNORM;  //������������(�����估��������)
	float4  tex2    : TEXOTHER;     //������������(��������)
	uint4   bone_id     : BONEINDICES;  //����ID��
	float4  bone_weight : WEIGHTS;      //����Ȩ��
};
struct Vertex_IN
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float4  tex1    : TEXDIFFNORM;  //������������(�����估��������)
	float4  tex2    : TEXOTHER;     //������������(��������)
};
struct Vertex_IN_anim
{
	uint   vertIndex		: SV_VertexID;
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float4  tex1    : TEXDIFFNORM;  //������������(�����估��������)
	float4  tex2    : TEXOTHER;     //������������(��������)
};
struct VertexOut
{
	float4 position      : SV_POSITION;    //�任��Ķ�������
	float3 normal        : NORMAL;         //�任��ķ�����
	float3 tangent       : TANGENT;        //����������
	uint4  texid         : TEXINDICES;     //��������
	float4  tex1         : TEXDIFFNORM;  //������������(�����估��������)
	float4  tex2         : TEXOTHER;     //������������(��������)
	float3 position_bef  : POSITION;       //�任ǰ�Ķ�������
};

struct Vertex_IN_array
{
	float3	pos 	 : POSITION;     //����λ��
	float3	normal   : NORMAL;       //���㷨����
	float3	tangent  : TANGENT;      //����������
	float3  tex      : TEXCOORD;     //�����������
	float4  texrange : TEXRANGE;     //����Χ
};
struct Vertex_IN_anim_array
{
	uint   vertIndex		: SV_VertexID;
	float3	pos 	 : POSITION;     //����λ��
	float3	normal   : NORMAL;       //���㷨����
	float3	tangent  : TANGENT;      //����������
	float3  tex      : TEXCOORD;     //�����������
	float4  texrange : TEXRANGE;     //����Χ
};
struct Vertex_IN_bone_array
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float3  tex      : TEXCOORD;     //�����������
	float4  texrange : TEXRANGE;     //����Χ
	uint4   bone_id     : BONEINDICES;  //����ID��
	float4  bone_weight : WEIGHTS;      //����Ȩ��
};
struct VertexOut_array
{
	float4 position      : SV_POSITION;    //�任��Ķ�������
	float3 normal        : NORMAL;         //�任��ķ�����
	float3 tangent       : TANGENT;        //����������
	float3  tex          : TEXCOORD;       //�����������
	float4  texrange     : TEXRANGE;       //����Χ
	float3 position_bef  : POSITION;       //�任ǰ�Ķ�������
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
	//float diffuse_angle = dot(light_dir_in, normal); //������н�
	float view_angle = dot(direction_view, normal);//���߼н�
	float cos_vh = dot(direction_view, h_vec);
	float4 fresnel = F0 + (float4(1.0f, 1.0f, 1.0f, 1.0f) - F0)*(1.0f - pow(cos_vh, 5.0f));//��������;

	//float4 fresnel = F0 + (float4(1.0f, 1.0f, 1.0f, 1.0f) - F0) * pow(2, ((-5.55473*cos_vh - 6.98316)*cos_vh));
	//NDF����������
	float alpha = tex_roughness * tex_roughness;
	float nh_mul = dot(normal, h_vec);
	float divide_ndf1 = nh_mul*nh_mul * (alpha * alpha - 1.0f) + 1.0f;
	float divide_ndf2 = pi * divide_ndf1 *divide_ndf1;
	float ndf = (alpha*alpha) / divide_ndf2;
	//GGX�ڵ���

	float ggx_k = (tex_roughness + 1.0f) * (tex_roughness + 1.0f) / 8.0f;
	float ggx_v = view_angle / (view_angle*(1 - ggx_k) + ggx_k);
	float ggx_l = diffuse_angle / (diffuse_angle*(1 - ggx_k) + ggx_k);
	float ggx = ggx_v * ggx_l;
	float3 v = reflect(light_dir_in, normal);
	float blin_phong = pow(max(dot(v, direction_view), 0.0f), 10);
	//���յľ��淴����
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

	float NoV = saturate(dot(normal, direction_view) - 0.01);
	float4 EnvBRDF = texture_rdfluv.Sample(samTex_liner, float2(tex_roughness, NoV));

	//texture_rdfluv
	float diffuse_angle = dot(-light_dir.dir, normal); //������н�
	count_pbr_reflect(tex_albedo_in, tex_matallic, tex_roughness, -light_dir.dir, normal, direction_view, diffuse_angle, mat_diffuse, mat_specular_pre);

	float3 reflect_direct = normalize(reflect(-direction_view, normal));
	float3 map_direct = reflect_direct.xyz;//��������

	uint index = tex_roughness * 6;
	float4 enviornment_color = texture_environment.SampleLevel(samTex_liner, map_direct, index);

	ambient = 0;
	//ambient *= 0.5f;
	//ambient = enviornment_color * tex_albedo_in * (EnvBRDF.x + EnvBRDF.y);         //������
	float4 ambient_diffuse = 0.8f*(1.0f - tex_matallic) * tex_albedo_in;
	//float4 ambient_diffuse = enviornment_color*0.3f * tex_albedo_in;
	float4 ambient_specular = (0.6f*enviornment_color + 0.4f*tex_albedo_in) * (SpecularColor * EnvBRDF.x + EnvBRDF.y);
	ambient = (ambient_diffuse + ambient_specular);
	if (diffuse_angle > 0.0f)
	{
		//diffuse = 0.0f;//�������
		//spec = 0.0f;;    //���淴���
		diffuse = diffuse_angle * mat_diffuse * light_dir.diffuse;//�������
		spec = diffuse_angle * mat_specular_pre * light_dir.specular;    //���淴���
	}
	else
	{
		diffuse = 0.0f;//�������
		spec = 0.0f;;    //���淴���
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
VertexOut VS_anim(Vertex_IN_anim vin)
{
	
	VertexOut vout;
	int offset_ID = vin.vertIndex + offset_num.x + offset_num.y * offset_num.z;
	mesh_anim data = input_point[offset_ID];
	float3 point_offset = data.pos;
	
	vout.position = mul(float4(point_offset, 1.0f), final_matrix);
	vout.normal = mul(float4(vin.normal, 0.0f), normal_matrix).xyz;
	//vout.normal = mul(float4(data.norm, 0.0f), normal_matrix).xyz;
	vout.tangent = mul(float4(vin.tangent, 0.0f), normal_matrix).xyz;
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	vout.position_bef = mul(float4(point_offset, 1.0f), world_matrix).xyz;
	return vout;
}
VertexOut VS_bone(Vertex_IN_bone vin)
{
	VertexOut vout;
	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.bone_weight.x;
	weights[1] = vin.bone_weight.y;
	weights[2] = vin.bone_weight.z;
	weights[3] = vin.bone_weight.w;
	for (int i = 0; i < 4; ++i)
	{
		// �����任һ�㲻���ڲ������ŵ���������Կ��Բ������ߵ���ת�ò���
		posL += weights[i] * mul(float4(vin.pos, 1.0f), gBoneTransforms[vin.bone_id[i]]).xyz;
		normalL += weights[i] * mul(vin.normal, (float3x3)gBoneTransforms[vin.bone_id[i]]);
		tangentL += weights[i] * mul(vin.tangent.xyz, (float3x3)gBoneTransforms[vin.bone_id[i]]);
	}
	
	vout.position = mul(float4(posL, 1.0f), final_matrix);
	vout.normal = mul(float4(normalL, 0.0f), normal_matrix).xyz;
	vout.tangent = mul(float4(tangentL, 0.0f), normal_matrix).xyz;
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	vout.position_bef = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	return vout;
}

VertexOut_array VS_array(Vertex_IN_array vin)
{
	VertexOut_array vout;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.normal = mul(float4(vin.normal, 0.0f), normal_matrix).xyz;
	vout.tangent = mul(float4(vin.tangent, 0.0f), normal_matrix).xyz;
	vout.tex = vin.tex;
	vout.texrange = vin.texrange;
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
	float4 tex_albedo = texture_diffuse.Sample(samTex_liner, pin.tex1.xy);
	clip(tex_albedo.a - 0.3f);
	float tex_metallic = texture_metallic.Sample(samTex_liner, pin.tex1.xy).r;
	float tex_roughness = texturet_roughness.Sample(samTex_liner, pin.tex1.xy).r;

	float3 normal_need = normalize(pin.normal);
	float3 view_dir = normalize(position_view - pin.position_bef);

	compute_dirlight_2(tex_albedo, tex_metallic, tex_roughness, light_dir, normal_need, view_dir,A,D,S);
	float4 final_color = A + D + S;

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
	//return float4(normal_need, 1.0f);
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
float2 exchange_wrap(float2 uv, float4 uv_range) 
{
	//��ԭ��ԭʼ����Ĳ�����Χ
	float2 uv_range_tex = float2(uv_range.y - uv_range.x, uv_range.w - uv_range.z);
	float2 uv_st_tex = float2(uv_range.x, uv_range.z);
	float2 uv_ed_tex = float2(uv_range.y, uv_range.w);
	float2 uv_out = uv - uv_st_tex;
	uv_out = frac(frac(uv_out / uv_range_tex)+1);
	uv_out = uv_out * uv_range_tex + uv_st_tex;

	uv_out = clamp(uv_out, uv_st_tex, uv_ed_tex);
	/*
	//�����������
	float uv_x = uv.x - uv_range.x;
	float new_pos_x = fmod(uv_x, range_x) + range_x;
	uv_out.x = fmod(new_pos_x, range_x) + uv_range.x;
	//������������
	float uv_y = uv.y - uv_range.z;
	float new_pos_y = fmod(uv_y, range_y) + range_y;
	uv_out.y = fmod(new_pos_y, range_y) + uv_range.z;
	*/
	return uv_out;
}
float4 PS_array(VertexOut_array pin) :SV_TARGET
{
	float4 A,D,S;

	pancy_light_basic light_dir;
	light_dir.ambient = float4(1.0f, 1.0f, 1.0f,1.0f);
	light_dir.diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
	light_dir.specular = float4(1.5f, 1.5f, 1.5f, 1.0f);
	light_dir.dir = normalize(float3(1.0f,0.0f, 0.0f));
	//float texID_data = pin.texid.y;
	float3 tex_out = pin.tex;
	tex_out.xy = exchange_wrap(pin.tex.xy, pin.texrange);
	float4 tex_albedo = texture_pack_diffuse.SampleGrad(samTex_liner, tex_out,ddx(pin.tex),ddy(pin.tex));
	clip(tex_albedo.a - 0.3f);
	float tex_metallic = texture_pack_metallic.SampleGrad(samTex_liner, tex_out, ddx(pin.tex), ddy(pin.tex)).r;
	float tex_roughness = texture_pack_roughness.SampleGrad(samTex_liner, tex_out, ddx(pin.tex), ddy(pin.tex)).r;

	float3 normal_need = normalize(pin.normal);
	float3 view_dir = normalize(position_view - pin.position_bef);

	compute_dirlight_2(tex_albedo, tex_metallic, tex_roughness, light_dir, normal_need, view_dir,A,D,S);
	float4 final_color = A + D + S;
	//float4 tex_color = texture_pack_diffuse.Sample(samTex_liner, float3(pin.tex1.xy, texID_data));
	//	if (texID_data < 1.5f) 
	//	{
	//		tex_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//	}

	//float4 final_color = tex_color *(A + D) + S;
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
technique11 light_tech_pbranim
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_anim()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_pbr()));
		//SetRasterizerState(DisableCulling);
	}
}
technique11 light_tech_pbr_withbone
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_pbr()));
	}
}
technique11 light_tech_array
{
	pass P0
	{
		//SetBlendState(CommonBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);
		SetVertexShader(CompileShader(vs_5_0, VS_array()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_array()));
	}
}
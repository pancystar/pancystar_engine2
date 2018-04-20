#include"skinmesh.hlsli"
#include"terrain_tess_basic.hlsli"
#include"plant_anim_basic.hlsli"
cbuffer PerFrame
{
	float4x4 world_matrix_array[300];
	float4x4 normal_matrix_array[300];
	float4x4 proj_matrix;
	float4x4 view_matrix;
	float4x4 gBoneTransforms[MAX_BONE_NUM]; //�����任
	uint     bone_num;
};
StructuredBuffer<float4x4> input_buffer;
Texture2DArray   texture_pack_array;  //��ͼ��

RasterizerState DisableCulling
{
	CullMode = FRONT;
};
struct VertexIn//��ͨ����
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float4  tex1    : TEXDIFFNORM;  //������������(�����估��������)
	float4  tex2    : TEXOTHER;     //������������(��������)
};
struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float3 PosV       : POSITION;
	float3 NormalV    : NORMAL;
	float3 tangent    : TANGENT;
	uint4   texid     : TEXINDICES;   //��������
	float4  tex1      : TEXDIFFNORM;  //������������(�����估��������)
	float4  tex2      : TEXOTHER;     //������������(��������)
};
struct Vertex_IN_instance
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float4  tex1    : TEXDIFFNORM;  //������������(�����估��������)
	float4  tex2    : TEXOTHER;     //������������(��������)
	uint InstanceId : SV_InstanceID;//instace������
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	float4x4 mat_world_view = mul(world_matrix, view_matrix);
	float4x4 mat_normal_view = mul(normal_matrix, view_matrix);
	vout.PosV = mul(float4(vin.pos, 1.0f), mat_world_view).xyz;
	vout.NormalV = mul(float4(vin.normal, 0.0f), mat_normal_view).xyz;
	vout.tangent = mul(float4(vin.tangent, 0.0f), mat_normal_view).xyz;
	//vout.PosH = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.PosH = mul(float4(vout.PosV, 1.0f), proj_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}
VertexOut VS_instance(Vertex_IN_instance vin)
{
	VertexOut vout;
	float4x4 mat_world_view = mul(world_matrix_array[vin.InstanceId], view_matrix);
	float4x4 mat_normal_view = mul(normal_matrix_array[vin.InstanceId], view_matrix);
	vout.PosV = mul(float4(vin.pos, 1.0f), mat_world_view).xyz;
	vout.NormalV = mul(float4(vin.normal, 0.0f), mat_normal_view).xyz;
	vout.tangent = mul(float4(vin.tangent, 0.0f), mat_normal_view).xyz;
	vout.PosH = mul(float4(vout.PosV, 1.0f), proj_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}
VertexOut VS_bone(Vertex_IN_bone vin)
{
	VertexOut vout;
	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	count_skin_mesh(vin, gBoneTransforms, posL, normalL, tangentL);
	float4x4 mat_world_view = mul(world_matrix, view_matrix);
	float4x4 mat_normal_view = mul(normal_matrix, view_matrix);
	vout.PosV = mul(float4(posL, 1.0f), mat_world_view).xyz;
	vout.NormalV = mul(float4(normalL, 0.0f), mat_normal_view).xyz;
	vout.tangent = mul(float4(tangentL, 0.0f), mat_normal_view).xyz;
	vout.PosH = mul(float4(posL, 1.0f), final_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}
VertexOut VS_bone_instance(Vertex_IN_bone_instance vin)
{
	VertexOut vout;
	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	float4x4 mat_world_view = mul(world_matrix_array[vin.InstanceId], view_matrix);
	float4x4 mat_normal_view = mul(normal_matrix_array[vin.InstanceId], view_matrix);
	count_skin_mesh(vin, input_buffer, bone_num, posL, normalL, tangentL);

	vout.PosV = mul(float4(posL, 1.0f), mat_world_view).xyz;
	vout.NormalV = mul(float4(normalL, 0.0f), mat_normal_view).xyz;
	vout.tangent = mul(float4(tangentL, 0.0f), mat_normal_view).xyz;
	vout.PosH = mul(float4(vout.PosV, 1.0f), proj_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}
VertexOut VS_mesh_anim(Vertex_IN_anim vin)
{
	float3 pos_anim = get_anim_point(vin);
	VertexOut vout;
	float4x4 mat_world_view = mul(world_matrix, view_matrix);
	float4x4 mat_normal_view = mul(normal_matrix, view_matrix);
	vout.PosV = mul(float4(pos_anim, 1.0f), mat_world_view).xyz;
	vout.NormalV = mul(float4(vin.normal, 0.0f), mat_normal_view).xyz;
	vout.tangent = mul(float4(vin.tangent, 0.0f), mat_normal_view).xyz;
	//vout.PosH = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.PosH = mul(float4(vout.PosV, 1.0f), proj_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}
VertexOut VS_mesh_anim_instance(Vertex_IN_anim_instance vin)
{
	float3 pos_anim = get_anim_point(vin);
	VertexOut vout;
	float4x4 mat_world_view = mul(world_matrix_array[vin.InstanceId], view_matrix);
	float4x4 mat_normal_view = mul(normal_matrix_array[vin.InstanceId], view_matrix);
	vout.PosV = mul(float4(pos_anim, 1.0f), mat_world_view).xyz;
	vout.NormalV = mul(float4(vin.normal, 0.0f), mat_normal_view).xyz;
	vout.tangent = mul(float4(vin.tangent, 0.0f), mat_normal_view).xyz;
	vout.PosH = mul(float4(vout.PosV, 1.0f), proj_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}

struct PixelOut_pbr
{
	float4 normalmetallic     : SV_TARGET0;
	float4 specroughness      : SV_TARGET1;
	float4 atmospheremask     : SV_TARGET2;
};
float4 PS(VertexOut pin) : SV_Target
{
	float texID_data_diffuse = pin.texid.x;
	float4 tex_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex1.xy, texID_data_diffuse));
	clip(tex_color.a - 0.1f);
	pin.NormalV = normalize(pin.NormalV);
	return float4(pin.NormalV, 10.0f);
}
PixelOut_pbr PS_withnormal(VertexOut pin, uniform float mask) : SV_Target
{
	PixelOut_pbr ps_out_pbr;
	ps_out_pbr.atmospheremask.r = mask;
	//��ȡ���������
	float texID_data_diffuse = pin.texid.x;//����������ID
	float texID_data_normal = pin.texid.y;//��������ID
	float4 tex_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex1.xy, texID_data_diffuse));
	clip(tex_color.a - 0.9f);
	//��ȡ�����ȼ��ֲڶȲ���
	float texID_data_metallic = pin.texid.z;//����������ID
	float texID_data_roughness = pin.texid.w;//�ֲڶ�����ID
	float metallic_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex2.xy, texID_data_metallic)).r;
	float roughness_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex2.zw, texID_data_roughness)).r;

	//gammaУ��
	//tex_color = float4(pow(tex_color.rgb, float3(2.2f, 2.2f, 2.2f)), tex_color.a);
	//metallic_color = pow(metallic_color,2.2f);
	//roughness_color = pow(roughness_color, 2.2f);
	//���㾵�淴������ǿ��
	float3 specular_F0 = lerp(0.04, tex_color.rgb, metallic_color);
	ps_out_pbr.specroughness = float4(specular_F0, roughness_color);
	//������������ͼ�Ĳ���
	pin.NormalV = normalize(pin.NormalV);
	if (pin.texid.y == -1)
	{
		ps_out_pbr.normalmetallic = float4(pin.NormalV, metallic_color);
		return ps_out_pbr;
	}
	pin.tangent = normalize(pin.tangent);
	//���ͼƬ�����Կռ�->ģ������ͳһ����ռ�ı任����
	float3 N = pin.NormalV;
	float3 T = normalize(pin.tangent - N * pin.tangent * N);
	float3 B = cross(N, T);
	float3x3 T2W = float3x3(T, B, N);
	float3 normal_map = texture_pack_array.Sample(samTex_liner, float3(pin.tex1.zw, texID_data_normal)).rgb;//�ӷ�����ͼ�л�÷��߲���
	//float3 normal_map = texture_normal.Sample(samTex_liner, pin.tex).rgb;
	normal_map = 2 * normal_map - 1;                               //��������ͼƬ����[0,1]ת������ʵ����[-1,1]  
	normal_map = normalize(mul(normal_map, T2W));                  //���߿ռ�������ռ�
	pin.NormalV = normal_map;
	ps_out_pbr.normalmetallic = float4(pin.NormalV, metallic_color);

	return ps_out_pbr;
}
PixelOut_pbr PS_terrain_normal(DominOut_terrain pin, uniform float mask)
{
	PixelOut_pbr ps_out_pbr;
	//float4 blend_color = float4(1, 0, 0, 0);
	ps_out_pbr.atmospheremask.r = mask;
	float4 blend_color = get_blend_data(pin);
	float4 tex_color = get_terrain_albedo(pin, blend_color);
	clip(tex_color.a - 0.5f);
	//��ȡ�����ȼ��ֲڶȲ���
	float metallic_color = get_terrain_metallic(pin, blend_color).r;
	float roughness_color = get_terrain_roughness(pin, blend_color).r;
	//gammaУ��
	//tex_color = float4(pow(tex_color.rgb, float3(2.2f, 2.2f, 2.2f)), tex_color.a);
	//metallic_color = pow(metallic_color,2.2f);
	//roughness_color = pow(roughness_color, 2.2f);
	//���㾵�淴������ǿ��
	float3 specular_F0 = lerp(0.04, tex_color.rgb, metallic_color);
	ps_out_pbr.specroughness = float4(specular_F0, roughness_color);
	//������ͼ
	float3 normal_map = get_terrain_normal(pin, blend_color);
	//float3 normal_map = ColorTexture_pack_normal_0.Sample(samTex, pin.tex1).xyz * 2.0f - 1.0f;
	//float3 tangent = terrain_tangent.Sample(samTex, pin.tex1).xyz * 2.0f - 1.0f;
	normal_map = mul(float4(normal_map, 0), view_matrix).xyz;
	ps_out_pbr.normalmetallic = float4(normal_map, metallic_color);
	//ps_out_pbr.normalmetallic = metallic_color + roughness_color;
	return ps_out_pbr;
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
technique11 NormalDepth_CullFornt
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withnormal(0.0f)));
		SetRasterizerState(DisableCulling);
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
		SetPixelShader(CompileShader(ps_5_0, PS_withnormal(1.0f)));
	}
}
technique11 NormalDepth_withnormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withnormal(1.0f)));
	}
}
technique11 NormalDepthSkin_withnormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withnormal(1.0f)));
	}
}
technique11 NormalDepthSkin_withinstance_normal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withnormal(1.0f)));
	}
}
technique11 NormalDepth_Terrain
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_terrain()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_terrain_normal(1.0f)));
	}
}

technique11 NormalDepth_withnormal_MeshAnim
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_mesh_anim()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withnormal(1.0f)));
	}
}
technique11 NormalDepth_withinstance_MeshAnim
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_mesh_anim_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withnormal(1.0f)));
	}
}


#include"skinmesh.hlsli"
#include"plant_anim_basic.hlsli"
float4x4         final_matrix;     //�ܱ任
float4x4 view_proj_matrix;
float4x4 world_matrix_array[300];
float4x4 gBoneTransforms[MAX_BONE_NUM]; //�����任
uint     bone_num;
StructuredBuffer<float4x4> input_buffer;
Texture2DArray   texture_pack_array;
SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
RasterizerState Depth
{
	DepthBias = 50000;
	DepthBiasClamp = 0.0f;
	SlopeScaledDepthBias = 1.0f;
};
RasterizerState DisableCulling
{
	CullMode = FRONT;
};
RasterizerState BackCulling
{
	CullMode = BACK;
};
struct Vertex_IN//��ͨ����
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float4  tex1    : TEXDIFFNORM;  //������������(�����估��������)
	float4  tex2    : TEXOTHER;     //������������(��������)
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
struct VertexOut
{
	float4  position      : SV_POSITION;    //�任��Ķ�������
	uint4   texid     : TEXINDICES;   //��������
	float4  tex1      : TEXDIFFNORM;  //������������(�����估��������)
	float4  tex2      : TEXOTHER;     //������������(��������)
};
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(float4(vin.pos, 1.f), final_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}
VertexOut VS_instance(Vertex_IN_instance vin)
{
	VertexOut vout;
	float4 pos_world = mul(float4(vin.pos, 1.0f), world_matrix_array[vin.InstanceId]);
	vout.position = mul(pos_world, view_proj_matrix);
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
	vout.position = mul(float4(posL, 1.f), final_matrix);
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
	count_skin_mesh(vin, input_buffer, bone_num, posL, normalL, tangentL);
	float4 pos_world = mul(float4(posL, 1.0f), world_matrix_array[vin.InstanceId]);
	vout.position = mul(pos_world, view_proj_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}

VertexOut VS_mesh_anim(Vertex_IN_anim vin)
{
	float3 pos_anim = get_anim_point(vin);
	VertexOut vout;
	vout.position = mul(float4(pos_anim, 1.f), final_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}
VertexOut VS_mesh_anim_instance(Vertex_IN_anim_instance vin)
{
	float3 pos_anim = get_anim_point(vin);
	VertexOut vout;
	float4 pos_world = mul(float4(pos_anim, 1.0f), world_matrix_array[vin.InstanceId]);
	vout.position = mul(pos_world, view_proj_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float texID_data_diffuse = pin.texid.x;
	float4 tex_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex1.xy, texID_data_diffuse));
	//float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.9f);
	return float4(0.0f, 0.0f, 0.0f, 1.0f);;
}
/*
float4 PS_instance(VertexOut pin) : SV_Target
{
	float4 tex_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex1.xy, texID_data_diffuse));
	clip(tex_color.a - 0.4f);
	return float4(0.0f, 0.0f, 0.0f, 1.0f);;
}
*/
technique11 ShadowTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetRasterizerState(DisableCulling);
		//SetRasterizerState(Depth);
	}
}
technique11 ShadowTech_instance
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		//SetRasterizerState(Depth);
		SetRasterizerState(DisableCulling);
	}
}
technique11 ShadowTechBone
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetRasterizerState(DisableCulling);
		//SetRasterizerState(Depth);
	}
}
technique11 ShadowTechBone_instance
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		//SetRasterizerState(Depth);
		SetRasterizerState(DisableCulling);
	}
}
technique11 ShadowTechMeshAnim
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_mesh_anim()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetRasterizerState(DisableCulling);
	}
}
technique11 ShadowTechMeshAnim_instance
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_mesh_anim_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetRasterizerState(DisableCulling);
	}
}
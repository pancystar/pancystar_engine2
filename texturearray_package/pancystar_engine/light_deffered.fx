/*�ӳٹ���Ч��*/
//TODO:������Ļ�����Ӱ����->��

#include"light_count.hlsli"
#include"skinmesh.hlsli"
#include"atmosphere_renderfunc.hlsli"
#include"terrain_tess_basic.hlsli"
#include"plant_anim_basic.hlsli"
cbuffer perobject
{
	pancy_material   material_need;    //����
	//float4x4         world_matrix;     //����任
	//float3           position_view;         //�ӵ�λ��
    //float4x4         final_matrix;     //�ܱ任
	//float4x4         ssao_matrix;      //ssao�任
	float4x4         world_matrix_array[300];
	float4x4         view_proj_matrix;
	float4x4         gBoneTransforms[MAX_BONE_NUM];     //����任
	uint             bone_num;
};
cbuffer perframe
{
	float4x4         view_matrix;    //ȡ���任
	float4x4         invview_matrix; //ȡ���任��任
};
DepthStencilState NoDepthWrites
{
	DepthEnable = TRUE;
	DepthWriteMask = ZERO;
};
StructuredBuffer<float4x4> input_buffer;
Texture2D        atmosphere_occlusion;           //����ɢ������
Texture2D        texture_light_diffuse;      //�����������ͼ
Texture2D        texture_light_specular;     //���淴�������ͼ
Texture2DArray   texture_pack_array;         //��������ͼ
Texture2D        texture_ssao;               //ssao��ͼ
Texture2D        gNormalspecMap;        //��Ļ�ռ䷨��&���淴��ǿ������
Texture2D        gInputspecular_roughness;
Texture2D        gInputbrdf;
Texture2D	     fog_color_tex;     //��Чͼ
TextureCube      IBL_cube;
TextureCube      IBL_diffuse;
BlendState CommonBlending
{
	AlphaToCoverageEnable = TRUE;
	BlendEnable[0] = FALSE;
};
struct Vertex_IN//��������ͼ����
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
	float3 position_before : POSITION;
	float3 position_view   : VPOSITION;
	float4 position        : SV_POSITION;    //�任��Ķ�������
	uint4   texid          : TEXINDICES;     //��������
	float4  tex1           : TEXDIFFNORM;    //������������(�����估��������)
	float4  tex2           : TEXOTHER;       //������������(��������)
	float4 pos_ssao        : POSITION1;      //��Ӱ��������
};
struct PixelOut
{
	float4 final_color;
	float4 reflect_message;
};
VertexOut VS_bone(Vertex_IN_bone vin)
{
	VertexOut vout;
	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	count_skin_mesh(vin, gBoneTransforms, posL, normalL, tangentL);
	vout.position_before = mul(float4(posL,1.0f), world_matrix).xyz;
	vout.position_view = mul(float4(vout.position_before, 1.0f), view_matrix).xyz;
	vout.position = mul(float4(posL, 1.0f), final_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	vout.pos_ssao = mul(float4(vout.position_before, 1.0f), ssao_matrix);
	return vout;
}
VertexOut VS_bone_instance(Vertex_IN_bone_instance vin)
{
	VertexOut vout;
	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	count_skin_mesh(vin, input_buffer, bone_num, posL, normalL, tangentL);
	vout.position_before = mul(float4(posL, 1.0f), world_matrix_array[vin.InstanceId]).xyz;
	vout.position_view = mul(float4(vout.position_before, 1.0f), view_matrix).xyz;
	vout.position = mul(float4(vout.position_before, 1.0f), view_proj_matrix);
	//vout.position = mul(float4(posL, 1.0f), final_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	vout.pos_ssao = mul(float4(vout.position_before, 1.0f), ssao_matrix);
	return vout;
}
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
VertexOut VS_mesh_anim(Vertex_IN_anim vin)
{
	VertexOut vout;
	float3 pos_anim = get_anim_point(vin);
	vout.position_before = mul(float4(pos_anim, 1.0f), world_matrix).xyz;
	vout.position_view = mul(float4(vout.position_before, 1.0f), view_matrix).xyz;
	vout.position = mul(float4(pos_anim, 1.0f), final_matrix);
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	vout.pos_ssao = mul(float4(vout.position_before, 1.0f), ssao_matrix);
	return vout;
}
VertexOut VS_mesh_anim_instance(Vertex_IN_anim_instance vin)
{
	VertexOut vout;
	float3 pos_anim = get_anim_point(vin);
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
	//gammaУ��
	//tex_color = float4(pow(tex_color.rgb, float3(2.2f, 2.2f, 2.2f)), tex_color.a);
	//float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	//float4 ambient = 0.4f* texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	float tex_ao = texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	
	//�ɼ������������
	float4 normalmetallic = gNormalspecMap.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);
	float3 normal_dir = normalize(normalmetallic.rgb);
	float tex_matallic = normalmetallic.a;
	//�����߱任������ռ�
	normal_dir = mul(float4(normal_dir,0.0f), invview_matrix);
	//���㷴����
	float3 view_dir = normalize(eye_pos - pin.position_before);
	float NoV = dot(view_dir, normal_dir);
	float3 reflect_dir = normalize(reflect(-view_dir, normal_dir));
	//�ɼ��������ֲڶ�
	float4 specular_roughness = gInputspecular_roughness.SampleLevel(samTex_liner, pin.pos_ssao.xy, 0);
	float4 SpecularColor = float4(specular_roughness.rgb, 1.0f);
	float  tex_roughness = specular_roughness.a;
	//�ɼ�brdf
	float4 EnvBRDF = gInputbrdf.Sample(samTex_liner, float2(tex_roughness, NoV));
	//����������
	uint index = tex_roughness * 6;
	float4 enviornment_color = IBL_cube.SampleLevel(samTex_liner, reflect_dir, index);
	float4 color_diffuse = IBL_diffuse.Sample(samTex_liner, reflect_dir);


	//gammaУ��
	//enviornment_color = float4(pow(enviornment_color.rgb, float3(2.2f, 2.2f, 2.2f)), enviornment_color.a);
	//tex_ao = pow(tex_ao, 2.2f);
	//���㻷���ⷴ��
	
	float4 ambient_diffuse = color_diffuse*tex_ao *(1.0f - tex_matallic) * tex_color;
	float4 ambient_specular = 0.2f*tex_ao * (0.6f*enviornment_color + 0.4f*tex_color) * (SpecularColor * EnvBRDF.x + EnvBRDF.y);
	float4 ambient = ambient_diffuse + ambient_specular;

	//float4 ambient = 0;
	float4 diffuse = texture_light_diffuse.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);      //�������
	//return diffuse;
	float4 spec =texture_light_specular.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);       //���淴���
	float4 final_color = ambient + tex_color * diffuse + spec;
	final_color.a = tex_color.a;

	/*
	//��ȡ�����ȼ��ֲڶȲ���
	float texID_data_metallic = pin.texid.z;//����������ID
	float texID_data_roughness = pin.texid.w;//�ֲڶ�����ID
	float metallic_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex2.xy, texID_data_metallic)).r;
	float roughness_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex2.zw, texID_data_roughness)).r;
	//final_color = float4(metallic_color, metallic_color, metallic_color,1);
	//final_color = float4(roughness_color, roughness_color, roughness_color, 1);
	*/
	PixelOut ans_pix;
	ans_pix.final_color = final_color;
	//ans_pix.final_color = diffuse;
	//float3 normal_dir = normalize(gNormalspecMap.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).rgb);
	//float3 position_view_sun = float3(0.0f, 0.0f, 0.0f);
	//float3 view_dir = normalize(position_view_sun - pin.position_view);

	ans_pix.reflect_message.rgb = tex_color.rgb;
	ans_pix.reflect_message.r = 1.0;
	return ans_pix;
}
PixelOut PS_withputao(VertexOut pin) :SV_TARGET
{
	/*
	pin.pos_ssao /= pin.pos_ssao.w;
	float texID_data_diffuse = pin.texid.x;
	float4 tex_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex1.xy, texID_data_diffuse));
	//float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.6f);
	//float4 ambient = 0.8f*float4(1.0f, 1.0f, 1.0f, 0.0f);
	*/
	pin.pos_ssao /= pin.pos_ssao.w;
	float texID_data_diffuse = pin.texid.x;

	
	float4 tex_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex1.xy, texID_data_diffuse));
	
	//gammaУ��
	//tex_color = float4(pow(tex_color.rgb, float3(2.2f, 2.2f, 2.2f)), tex_color.a);
	//float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.6f);
	//float4 ambient = 0.4f* texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	float tex_ao = texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;

	//�ɼ������������
	float4 normalmetallic = gNormalspecMap.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);
	float3 normal_dir = normalize(normalmetallic.rgb);
	float tex_matallic = normalmetallic.a;
	//�����߱任������ռ�
	normal_dir = mul(float4(normal_dir, 0.0f), invview_matrix);
	//���㷴����
	float3 view_dir = normalize(eye_pos - pin.position_before);
	float NoV = dot(view_dir, normal_dir);
	float3 reflect_dir = normalize(reflect(-view_dir, normal_dir));
	//�ɼ��������ֲڶ�
	float4 specular_roughness = gInputspecular_roughness.SampleLevel(samTex_liner, pin.pos_ssao.xy, 0);
	float4 SpecularColor = float4(specular_roughness.rgb, 1.0f);
	float  tex_roughness = specular_roughness.a;
	//�ɼ�brdf
	float4 EnvBRDF = gInputbrdf.Sample(samTex_liner, float2(tex_roughness, NoV));
	//����������
	uint index = tex_roughness * 6;
	float4 enviornment_color = IBL_cube.SampleLevel(samTex_liner, reflect_dir, index);
	//���㻷���ⷴ��
	float4 ambient_diffuse = 0.2f*2.0f*(1.0f - tex_matallic) * tex_color;
	float4 ambient_specular = 0.2f * 2.0f*(0.6f * enviornment_color + 0.4f*tex_color) * (SpecularColor * EnvBRDF.x + EnvBRDF.y);
	float4 ambient = ambient_diffuse + ambient_specular;

	float4 diffuse = material_need.diffuse * texture_light_diffuse.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);      //�������
	float4 spec = material_need.specular * texture_light_specular.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);       //���淴���
	float4 final_color = tex_color *(ambient + diffuse) + spec;
	final_color.a = tex_color.a;

	PixelOut ans_pix;
	ans_pix.final_color = final_color;
	ans_pix.reflect_message = material_need.reflect;
	return ans_pix;
}
PixelOut PS_withbone(VertexOut pin) :SV_TARGET
{
	float texID_data_diffuse = pin.texid.x;
	float4 tex_color = texture_pack_array.Sample(samTex_liner, float3(pin.tex1.xy, texID_data_diffuse));
	PixelOut ans_pix;
	ans_pix.final_color = tex_color;
	ans_pix.reflect_message = material_need.reflect;
	return ans_pix;
}
PixelOut PS_terrain(DominOut_terrain pin) :SV_TARGET
{
	pin.pos_ssao /= pin.pos_ssao.w;
	float4 blend = get_blend_data(pin);
	float4 tex_color = get_terrain_albedo(pin, blend);
	//gammaУ��
	//tex_color = float4(pow(tex_color.rgb, float3(2.2f, 2.2f, 2.2f)), tex_color.a);
	//float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.6f);
	float tex_ao = texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	float mask_need = atmosphere_occlusion.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	clip(mask_need - 0.99f);
	//�ɼ������������
	float4 normalmetallic = gNormalspecMap.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);
	float3 normal_dir = normalize(normalmetallic.rgb);
	float tex_matallic = normalmetallic.a;
	//�����߱任������ռ�
	normal_dir = mul(float4(normal_dir,0.0f), invview_matrix);
	//���㷴����
	float3 view_dir = normalize(eye_pos - pin.pos_before);
	float NoV = dot(view_dir, normal_dir);
	float3 reflect_dir = normalize(reflect(-view_dir, normal_dir));
	//�ɼ��������ֲڶ�
	float4 specular_roughness = gInputspecular_roughness.SampleLevel(samTex_liner, pin.pos_ssao.xy, 0);
	float4 SpecularColor = float4(specular_roughness.rgb, 1.0f);
	float  tex_roughness = specular_roughness.a;
	//�ɼ�brdf
	float4 EnvBRDF = gInputbrdf.Sample(samTex_liner, float2(tex_roughness, NoV));
	//����������
	uint index = tex_roughness * 6;
	float4 enviornment_color = IBL_cube.SampleLevel(samTex_liner, reflect_dir, index);
	float4 color_diffuse = IBL_diffuse.Sample(samTex_liner, reflect_dir);
	//gammaУ��
	//enviornment_color = float4(pow(enviornment_color.rgb, float3(2.2f, 2.2f, 2.2f)), enviornment_color.a);
	//tex_ao = pow(tex_ao, 2.2f);
	//���㻷���ⷴ��
	float4 ambient_diffuse = color_diffuse*tex_ao *(1.0f - tex_matallic) * tex_color;
	float4 ambient_specular = 0.2f*tex_ao * (0.6f*enviornment_color + 0.4f*tex_color) * (SpecularColor * EnvBRDF.x + EnvBRDF.y);
	float4 ambient = ambient_diffuse + ambient_specular;

	float4 diffuse = texture_light_diffuse.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);      //�������
	float4 spec =texture_light_specular.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);       //���淴���
	//ambient = float4(0, 0, 0, 0);
	float4 final_color = ambient + tex_color * diffuse + spec;
	//float4 final_color = float4(normal_dir,1.0f);
	final_color.a = tex_color.a;


	float4 fog_color = fog_color_tex.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);
	float distance_need = distance(eye_pos, pin.pos_before);
	float s_need = saturate((distance_need - 300.0) / 1000.0);
	final_color.rgb = lerp(final_color.rgb, fog_color.rgb, s_need);
	PixelOut ans_pix;
	ans_pix.final_color.xyz = final_color;

	ans_pix.reflect_message.rgb = tex_color.rgb;
	ans_pix.reflect_message.r = 1.0;
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
technique11 LightTech_bone
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withbone()));
	}
}
technique11 LightTech_instance_bone
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withbone()));
	}
}
technique11 LightTech_Terrain
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_terrain()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_terrain()));
	}
}
technique11 LightTech_plant
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_mesh_anim()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetBlendState(CommonBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);
		//SetDepthStencilState(NoDepthWrites, 0);
	}
}
technique11 LightTech_plant_instance
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_mesh_anim_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
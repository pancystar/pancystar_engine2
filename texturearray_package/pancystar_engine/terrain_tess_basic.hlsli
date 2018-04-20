#include"sample.hlsli"
cbuffer common 
{
	float4x4 world_matrix;      //����任
	float4x4 normal_matrix;     //���߱任
	float4x4 ssao_matrix;       //ssao�任
	float4x4 final_matrix;      //�ܱ任
};
cbuffer PerFramesize
{
	float4 terrain_size;        //{����ʵ�ʴ�С�����������С�����θ߶����ţ�Ԥ��λ}
	float3 eye_pos;             //�ӵ�λ��
};
Texture2D	terrain_height;     //���θ߶�ͼ
Texture2D	terrain_normal;     //���η���ͼ
Texture2D	terrain_tangent;    //��������ͼ
Texture2D	terrain_blend;      //����������ͼ

//������ͼ��
Texture2D ColorTexture_pack_albedo_0;
Texture2D ColorTexture_pack_albedo_1;
Texture2D ColorTexture_pack_albedo_2;
Texture2D ColorTexture_pack_albedo_3;
//������ͼ��
Texture2D ColorTexture_pack_normal_0;
Texture2D ColorTexture_pack_normal_1;
Texture2D ColorTexture_pack_normal_2;
Texture2D ColorTexture_pack_normal_3;
//��������ͼ��
Texture2D ColorTexture_pack_metallic_0;
Texture2D ColorTexture_pack_metallic_1;
Texture2D ColorTexture_pack_metallic_2;
Texture2D ColorTexture_pack_metallic_3;
//�ֲڶ���ͼ��
Texture2D ColorTexture_pack_roughness_0;
Texture2D ColorTexture_pack_roughness_1;
Texture2D ColorTexture_pack_roughness_2;
Texture2D ColorTexture_pack_roughness_3;
/*
Texture2DArray   ColorTexture_pack_albedo;     //������ͼ��
Texture2DArray   ColorTexture_pack_normal;     //������ͼ��
Texture2DArray   ColorTexture_pack_metallic;   //��������ͼ��
Texture2DArray   ColorTexture_pack_roughness;  //�ֲڶ���ͼ��
Texture2D	fog_color;     //��Чͼ
*/

struct patch_tess
{
	float edge_tess[4]: SV_TessFactor;
	float inside_tess[2] : SV_InsideTessFactor;
};
struct VertexIN_terrain
{
	float3	pos 	: POSITION;     //����λ��
	float2  tex1    : TEXHEIGHT;    //�߶�ͼ��������
	float2  tex2    : TEXDIFFUSE;   //��ɫͼ��������
};
struct VertexOut_terrain
{
	float3	position_before 	: POSITION;     //����λ��
	float2  tex1    : TEXHEIGHT;    //�߶�ͼ��������
	float2  tex2    : TEXDIFFUSE;   //��ɫͼ��������
};
struct HullOut_terrain
{
	float3	position_before 	: POSITION;     //����λ��
	float2  tex1    : TEXHEIGHT;    //�߶�ͼ��������
	float2  tex2    : TEXDIFFUSE;   //��ɫͼ��������
};
struct DominOut_terrain
{
	float4 position   : SV_POSITION;     //�任��Ķ�������
	float4 pos_before : POSITIONB;       //��դ��������
	float4 pos_ssao   : POSITION1;      //��Ӱ��������
	float2  tex1      : TEXHEIGHT;       //�߶�ͼ��������
	float2  tex2      : TEXDIFFUSE;      //��ɫͼ��������
	
};

VertexOut_terrain VS_terrain(VertexIN_terrain vin)
{
	VertexOut_terrain vout;
	vout.position_before = vin.pos;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}
int count_divide_num(float3 final_pos)
{
	float dist_2d = distance(final_pos, eye_pos);
	float far_way = 500;
	float near_way = 50;
	float delta = (far_way - dist_2d) / (far_way - near_way);
	delta = clamp(delta, 0, 1);
	//delta = delta * delta;
	return max(64 * delta, 2);
}
patch_tess ConstantHS(InputPatch<VertexOut_terrain, 4> patch, uint PatchID:SV_PrimitiveID)
{
	/*
	P0-------e1-------P1
	|                |
	|                |
	e0|                |e2
	|                |
	|                |
	P2-------e3-------P3
	*/

	patch_tess pt;

	float3 v1_pos = lerp(patch[2].position_before, patch[0].position_before, 0.5);
	float3 final_pos = mul(float4(v1_pos, 1.0f), world_matrix).xyz;
	pt.edge_tess[0] = count_divide_num(final_pos);

	float3 v0_pos = lerp(patch[0].position_before, patch[1].position_before, 0.5);
	final_pos = mul(float4(v0_pos, 1.0f), world_matrix).xyz;
	pt.edge_tess[1] = count_divide_num(final_pos);

	float3 v2_pos = lerp(patch[1].position_before, patch[3].position_before, 0.5);
	final_pos = mul(float4(v2_pos, 1.0f), world_matrix).xyz;
	pt.edge_tess[2] = count_divide_num(final_pos);

	float3 v3_pos = lerp(patch[3].position_before, patch[2].position_before, 0.5);
	final_pos = mul(float4(v3_pos, 1.0f), world_matrix).xyz;
	pt.edge_tess[3] = count_divide_num(final_pos);

	int max1 = max(pt.edge_tess[0], pt.edge_tess[1]);
	int max2 = max(pt.edge_tess[2], pt.edge_tess[3]);
	int max_devide_num = max(max1, max2);

	pt.inside_tess[0] = max_devide_num;
	pt.inside_tess[1] = max_devide_num;

	//pt.edge_tess[0] = 64;
	//pt.edge_tess[1] = 64;
	//pt.edge_tess[2] = 64;
	//pt.edge_tess[3] = 64;
	//pt.inside_tess[0] = 64;
	//pt.inside_tess[1] = 64;

	return pt;
}
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut_terrain HS(
	InputPatch<VertexOut_terrain, 4> patch,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
	HullOut_terrain hout;
	hout.position_before = patch[i].position_before;
	hout.tex1 = patch[i].tex1;
	hout.tex2 = patch[i].tex2;
	return hout;
}
[domain("quad")]
DominOut_terrain DS(
	patch_tess patchTess,
	float2 uv : SV_DomainLocation,
	const OutputPatch<HullOut_terrain, 4> quard
	)
{
	DominOut_terrain rec;
	//��ͼ�����ֵ
	float2 v1_uv_local = lerp(quard[0].tex1, quard[1].tex1, uv.x);
	float2 v2_uv_local = lerp(quard[2].tex1, quard[3].tex1, uv.x);
	float2 uv_local_tex1 = lerp(v1_uv_local, v2_uv_local, uv.y);

	v1_uv_local = lerp(quard[0].tex2, quard[1].tex2, uv.x);
	v2_uv_local = lerp(quard[2].tex2, quard[3].tex2, uv.x);
	float2 uv_local_tex2 = lerp(v1_uv_local, v2_uv_local, uv.y);
	//λ�ò�ֵ
	float3 v1_position = lerp(quard[0].position_before, quard[1].position_before, uv.x);
	float3 v2_position = lerp(quard[2].position_before, quard[3].position_before, uv.x);
	float3 position_before = lerp(v1_position, v2_position, uv.y);
	//�����߶���ͼ
	float sample_height = terrain_height.SampleLevel(samTex_liner, uv_local_tex1, 0).x;
	position_before.y = sample_height * terrain_size.z;
	//�����µĶ���
	rec.position = mul(float4(position_before, 1.0f), final_matrix);
	rec.pos_before = mul(float4(position_before, 1), world_matrix);
	rec.pos_ssao = mul(rec.pos_before, ssao_matrix);
	rec.tex1 = uv_local_tex1;
	rec.tex2 = uv_local_tex2;
	return rec;
};
float4 get_blend_data(DominOut_terrain pin)
{ 
	float4 blend = terrain_blend.Sample(samTex, float2(pin.tex1.x, 1.0f - pin.tex1.y));
	return blend;
}
float3 get_terrain_normal(DominOut_terrain pin, float4 blend)
{
	//��ȡ�������ߣ������Լ���ϲ���
	float3 normal = terrain_normal.Sample(samTex, pin.tex1).xyz * 2.0f - 1.0f;
	float3 tangent = terrain_tangent.Sample(samTex, pin.tex1).xyz * 2.0f - 1.0f;
	
	//float3 normal = float3(1,0,0);
	//float3 tangent = float3(0, 0, 1);
	//һ�η���ӳ��
	float3 normal_before = float3(0, 1, 0);
	float3 tangent_before = float3(1, 0, 0);
	float3 bitan_before = float3(0, 0, 1);
	float3x3 T2W_before = float3x3(tangent_before, bitan_before, normal_before);
	normal = normalize(mul(normal, T2W_before));
	//return normal;
	/*
	//��ȡ�����ɫ
	float4 tex_color_1 = ColorTexture_pack_albedo.Sample(samTex, float3(pin.tex2, 0));
	float4 tex_color_2 = ColorTexture_pack_albedo.Sample(samTex, float3(pin.tex2, 1));
	float4 tex_color_3 = ColorTexture_pack_albedo.Sample(samTex, float3(pin.tex2, 2));
	float4 tex_color_4 = ColorTexture_pack_albedo.Sample(samTex, float3(pin.tex2, 3));

	float4 blend_color = blend.r * tex_color_1 + blend.g * tex_color_2 + blend.b * tex_color_3 + blend.a * tex_color_4;
	*/
	//��ȡ��Ϸ���
	float3 tex_normal_1 = ColorTexture_pack_normal_0.Sample(samTex, pin.tex2).xyz * 2.0f - 1.0f;
	float3 tex_normal_2 = ColorTexture_pack_normal_1.Sample(samTex, pin.tex2).xyz * 2.0f - 1.0f;
	float3 tex_normal_3 = ColorTexture_pack_normal_2.Sample(samTex, pin.tex2).xyz * 2.0f - 1.0f;
	float3 tex_normal_4 = ColorTexture_pack_normal_3.Sample(samTex, pin.tex2).xyz * 2.0f - 1.0f;

	float3 blend_normal = blend.r * tex_normal_1 + blend.g * tex_normal_2 + blend.b * tex_normal_3 + blend.a * tex_normal_4;
	//���η���ӳ��
	tangent = normalize(tangent - normal * tangent * normal);
	float3 bitan = cross(normal, tangent);
	float3x3 T2W = float3x3(tangent, bitan, normal);
	float3 normal_map = normalize(mul(blend_normal, T2W));
	//normal_map = normalize(mul(normal_map, float3x3(view_matrix)));
	return normal_map;
}
float4 get_terrain_albedo(DominOut_terrain pin, float4 blend)
{
	//��ȡ�����ɫ
	float4 tex_color_1 = ColorTexture_pack_albedo_0.Sample(samTex, pin.tex2);
	float4 tex_color_2 = ColorTexture_pack_albedo_1.Sample(samTex, pin.tex2);
	float4 tex_color_3 = ColorTexture_pack_albedo_2.Sample(samTex, pin.tex2);
	float4 tex_color_4 = ColorTexture_pack_albedo_3.Sample(samTex, pin.tex2);
	float4 blend_color = blend.r * tex_color_1 + blend.g * tex_color_2 + blend.b * tex_color_3 + blend.a * tex_color_4;
	return blend_color;
}
float3 get_terrain_metallic(DominOut_terrain pin, float4 blend)
{
	//��ȡ�����ɫ
	float4 tex_color_1 = ColorTexture_pack_metallic_0.Sample(samTex, pin.tex2);
	float4 tex_color_2 = ColorTexture_pack_metallic_1.Sample(samTex, pin.tex2);
	float4 tex_color_3 = ColorTexture_pack_metallic_2.Sample(samTex, pin.tex2);
	float4 tex_color_4 = ColorTexture_pack_metallic_3.Sample(samTex, pin.tex2);
	float4 blend_color = blend.r * tex_color_1 + blend.g * tex_color_2 + blend.b * tex_color_3 + blend.a * tex_color_4;
	return blend_color;
}
float3 get_terrain_roughness(DominOut_terrain pin, float4 blend)
{
	//��ȡ�����ɫ
	float4 tex_color_1 = ColorTexture_pack_roughness_0.Sample(samTex, pin.tex2);
	float4 tex_color_2 = ColorTexture_pack_roughness_1.Sample(samTex, pin.tex2);
	float4 tex_color_3 = ColorTexture_pack_roughness_2.Sample(samTex, pin.tex2);
	float4 tex_color_4 = ColorTexture_pack_roughness_3.Sample(samTex, pin.tex2);
	float4 blend_color = blend.r * tex_color_1 + blend.g * tex_color_2 + blend.b * tex_color_3 + blend.a * tex_color_4;
	return blend_color;
}

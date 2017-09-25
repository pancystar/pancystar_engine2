float count_Water_height(float depth_sample)
{
	return (2.0f*depth_sample - 1.0f) * 0.001f;
}
cbuffer cbShading
{
	// Water-reflected sky color
	float3		g_SkyColor			: packoffset(c0.x);
	// The color of bottomless water body
	float3		g_WaterbodyColor	: packoffset(c1.x);

	// The strength, direction and color of sun streak
	float		g_Shineness : packoffset(c1.w);
	float3		g_SunDir			: packoffset(c2.x);
	float3		g_SunColor			: packoffset(c3.x);

	// The parameter is used for fixing an artifact
	float3		g_BendParam			: packoffset(c4.x);

	// Perlin noise for distant wave crest
	float		g_PerlinSize : packoffset(c4.w);
	float3		g_PerlinAmplitude	: packoffset(c5.x);
	float3		g_PerlinOctave		: packoffset(c6.x);
	float3		g_PerlinGradient	: packoffset(c7.x);

	// Constants for calculating texcoord from position
	float		g_TexelLength_x2 : packoffset(c7.w);
	float		g_UVScale : packoffset(c8.x);
	float		g_UVOffset : packoffset(c8.y);
};
cbuffer cbChangePerCall
{
	// Transform matrices
	float4x4	g_matLocal;
	float4x4    scal_matrix;     //缩放变换
	float4x4    final_matrix;     //总变换
	float4x4    ssao_matrix;      //ssao变换

	// Misc per draw call constants
	float2		g_UVBase;
	float2		g_PerlinMovement;
	float3		g_LocalEye;
}
SamplerState g_samplerDisplacement
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};
// Perlin noise for composing distant waves, W for height field, XY for gradient
SamplerState g_samplerPerlin
{
	Filter = ANISOTROPIC;
	AddressU = WRAP;
	AddressV = WRAP;
	AddressW = WRAP;
	MaxAnisotropy=4;
};
// FFT wave gradient map, converted to normal value in PS
SamplerState g_samplerGradient
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 8;
	AddressU = WRAP;
	AddressV = WRAP;
	AddressW = WRAP;
};
// Fresnel factor lookup table
SamplerState g_samplerFresnel
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
	AddressW = WRAP;
};
// A small sky cubemap for reflection
SamplerState g_samplerCube
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

Texture2D	g_texDisplacement;// FFT wave displacement map in VS
Texture2D	g_texPerlin; // FFT wave gradient map in PS
Texture2D	g_texGradient; // Perlin wave displacement & gradient map in both VS & PS
Texture1D	g_texFresnel; // Fresnel factor lookup table
TextureCube	g_texReflectCube; // A small skybox cube texture for reflection

struct patch_tess
{
	float edge_tess[4]: SV_TessFactor;
	float inside_tess[2] : SV_InsideTessFactor;
};
struct Vertex_IN_Water
{
	float2  tex1    : TEXCOORD;     //纹理坐标
};
struct VertexOut_Water
{
	float2 tex1            : TEXCOORD;     //纹理坐标
};
struct hull_out_Water
{
	float2 tex1            : TEXCOORD;       //深度纹理坐标
};
struct domin_out_Water
{
	float4 position        : SV_POSITION;    //变换后的顶点坐标
	float2 pos_uv          : TEXCOORD;
	float4 pos_ssao        : POSITION;       //光栅顶点坐标
};
struct PixelOut
{
	float4 final_color;
	float4 reflect_message;
};
VertexOut_Water VS_Water(Vertex_IN_Water vin)
{
	VertexOut_Water vout;
	vout.tex1 = vin.tex1;
	return vout;
}
patch_tess ConstantHS(InputPatch<VertexOut_Water, 4> patch, uint PatchID:SV_PrimitiveID)
{
	patch_tess pt;
	pt.edge_tess[0] = 64;
	pt.edge_tess[1] = 64;
	pt.edge_tess[2] = 64;
	pt.edge_tess[3] = 64;

	pt.inside_tess[0] = 64;
	pt.inside_tess[1] = 64;

	return pt;
}
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
hull_out_Water HS(
	InputPatch<VertexOut_Water, 4> patch,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
	hull_out_Water hout;
	hout.tex1 = patch[i].tex1;
	return hout;
}
[domain("quad")]
domin_out_Water DS(
	patch_tess patchTess,
	float2 uv : SV_DomainLocation,
	const OutputPatch<hull_out_Water, 4> quard
	)
{
	domin_out_Water rec;
	//贴图坐标插值
	float2 v1_tex1 = lerp(quard[0].tex1, quard[1].tex1, uv.x);
	float2 v2_tex1 = lerp(quard[2].tex1, quard[3].tex1, uv.x);
	float2 tex1_need = lerp(v1_tex1, v2_tex1, uv.y);
	//读取法线图
	float3 position_before = mul(float4(tex1_need, 0,1 ),scal_matrix ).xyz;
	float2 uv_local = position_before.xy * g_UVScale + g_UVOffset;

	
	//float3 normal;
	//normal.x = g_texGradient.SampleLevel(samTex_liner, uv_local, 0).x;
	//normal.y = g_texGradient.SampleLevel(samTex_liner, uv_local, 0).y;
	//normal.z = g_TexelLength_x2;
	//采样位移贴图
	float3 sample_Displacement = g_texDisplacement.SampleLevel(g_samplerDisplacement, uv_local,0).xyz;
	position_before += sample_Displacement;
	//地形纹理坐标插值
	float4 aopos_before = mul(float4(position_before, 1.0f), ssao_matrix);
	//生成新的顶点
	rec.position = mul(float4(position_before, 1.0f), final_matrix);
	rec.pos_ssao = float4(position_before,1);
	rec.pos_uv = uv_local;
	return rec;
};
PixelOut PS_Water(domin_out_Water pin) :SV_TARGET
{
	//pin.pos_ssao /= pin.pos_ssao.w + (pin.position / pin.position.w) *0.001f;
	//pin.pos_ssao /= pin.pos_ssao.w;

	//float4 tex_color = texture_Water_diffuse.Sample(samTex, float3(pin.tex, pin.texid[1]));
	//clip(tex_color.a - 0.6f);
	//float4 ambient = 0.5f*float4(1.0f, 1.0f, 1.0f, 0.0f) * texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	//float4 diffuse = material_need.diffuse * texture_light_diffuse.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);      //漫反射光
	//float4 spec = material_need.specular * texture_light_specular.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);       //镜面反射光
	//float4 final_color = tex_color *(ambient + diffuse) + spec;
	//final_color.a = tex_color.a;
	float2 grad = g_texGradient.Sample(g_samplerGradient, pin.pos_uv).xy;
	// Calculate normal here.
	float3 normal = normalize(float3(grad, g_TexelLength_x2));
	//normal.x = max(0, normal.x);
	//normal.y = max(0, normal.y);
	//float3 normal = g_texGradient.Sample(samTex_liner, pin.pos_uv).xyz;
	float3 eye_vec = g_LocalEye - pin.pos_ssao;
	float3 eye_dir = normalize(eye_vec);
	float3 reflect_vec = reflect(-eye_dir, normal);
	float cos_angle = dot(normal, eye_dir);
	// A coarse way to handle transmitted light
	float3 body_color = g_WaterbodyColor;


	//float3 h_vec = normalize((g_SunDir + eye_dir) / 2.0f);
	//float cos_vh = dot(eye_dir, h_vec);
	//float4 ramp = 0.3 + (1 - 0.3)*pow(2, (-5.55473*cos_vh - 6.98316)*cos_vh);
	float4 ramp = g_texFresnel.Sample(g_samplerFresnel, cos_angle).xyzw;

	if (reflect_vec.z < g_BendParam.x)
		ramp = lerp(ramp, g_BendParam.z, (g_BendParam.x - reflect_vec.z) / (g_BendParam.x - g_BendParam.y));
	reflect_vec.z = max(0, reflect_vec.z);

	float3 reflection = g_texReflectCube.Sample(g_samplerCube, reflect_vec).xyz;
	// dot(N, V)
	// Hack bit: making higher contrast
	reflection = reflection * reflection * 2.5f;

	// Blend with predefined sky color
	float3 reflected_color = lerp(g_SkyColor, reflection, ramp.y);

	// Combine waterbody color and reflected color
	float3 water_color = lerp(body_color, reflected_color, ramp.x);



	// --------------- Sun spots

	float cos_spec = clamp(dot(reflect_vec, g_SunDir), 0, 1);
	float sun_spot = pow(cos_spec, g_Shineness);
	water_color += g_SunColor * sun_spot;

	//water_color = g_WaterbodyColor+ dot(normal, g_SunDir)* g_WaterbodyColor*4;
	//water_color = pow(water_color.rgb, float3(2.2, 2.2, 2.2));
	//water_color = normal;
	PixelOut ans_pix;
	ans_pix.final_color = float4(water_color,1);
	ans_pix.reflect_message = float4(0.0, 0.0f, 0.0f, 1.0f);
	//ans_pix.final_color = final_color;
	//ans_pix.reflect_message = material_need.reflect;
	return ans_pix;
}
technique11 LightWater
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Water()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Water()));
	}
}
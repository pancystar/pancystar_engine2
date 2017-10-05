#define PI 3.141592653
cbuffer perframe
{
	float2  HalfPixel;
	float	Face;
	float	MipIndex;
};
TextureCube Cubemap;
SamplerState SamplerAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 16;
	AddressU = Wrap;
	AddressV = Wrap;
};
struct VSInput
{
	float4 Position 	: POSITION;
	float2 TexCoord 	: TEXCOORD0;
};

struct VSOutput
{
	float4 Position			: SV_Position;
	float2 TexCoord			: TEXCOORD0;
};
struct PSInput
{
	float4 Position			: SV_Position;
	float2 TexCoord			: TEXCOORD0;
};
//=================================================================================================
// Vertex Shader
//=================================================================================================
VSOutput VS(VSInput input)
{
	VSOutput output;
	output.Position = float4(input.Position.xyz, 1.0f);
	output.TexCoord = input.TexCoord;
	return output;
}
float3 GetNormal(uint face, float2 uv)
{
	float2 debiased = uv * 2.0f - 1.0f;

	float3 dir = 0;

	switch (face)
	{
		case 0: dir = float3(1, -debiased.y, -debiased.x); 
			break;

		case 1: dir = float3(-1, -debiased.y, debiased.x); 
			break;

		case 2: dir = float3(debiased.x, 1, debiased.y); 
			break;

		case 3: dir = float3(debiased.x, -1, -debiased.y); 
			break;

		case 4: dir = float3(debiased.x, -debiased.y, 1); 
			break;

		case 5: dir = float3(-debiased.x, -debiased.y, -1); 
			break;
	};

	return normalize(dir);
}
// ================================================================================================
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf 
// ================================================================================================
unsigned int ReverseBits32(unsigned int bits)
{
	bits = (bits << 16) | (bits >> 16);
	bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
	bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
	bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
	bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
	return bits;
}
float2 Hammersley(int Index, int NumSamples)
{
	float t1 = 0.00125125889;
	float E1 = 1.0 * Index / NumSamples + t1;
	E1 = E1 - int(E1);
	float E2 = float(ReverseBits32(Index)) * 2.3283064365386963e-10;
	return float2(E1, E2);
}
float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N)
{
	float a = Roughness * Roughness; // DISNEY'S ROUGHNESS [see Burley'12 siggraph]

	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt((1 - Xi.y) / (1 + (a * a - 1) * Xi.y));
	float SinTheta = sqrt(1 - CosTheta * CosTheta);

	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;

	float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);

	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}
float3 PrefilterEnvMap(float Roughness, float3 R, TextureCube EnvMap)
{
	float TotalWeight = 0.0000001f;

	float3 N = R;
	float3 V = R;
	float3 PrefilteredColor = 0;

	const uint NumSamples = 1024;

	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = Hammersley(i, NumSamples);
		float3 H = ImportanceSampleGGX(Xi, Roughness, N);
		float3 L = 2 * dot(V, H) * H - V;
		float NoL = saturate(dot(N, L));

		if (NoL > 0)
		{
			PrefilteredColor += EnvMap.SampleLevel(SamplerAnisotropic, L, 0).rgb * NoL;
			TotalWeight += NoL;
		}
	}

	return PrefilteredColor / TotalWeight;
}


float3 ImportanceSample_diffuse(float2 Xi, float3 N)
{
	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt(1 - Xi.y);
	float SinTheta = sqrt(1 - CosTheta * CosTheta);

	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;

	float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);

	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}
float3 Prefilter_DiffuseEnvMap(float3 R, TextureCube EnvMap)
{
	float TotalWeight = 0.0000001f;
	float3 N = R;
	float3 PrefilteredColor = 0;
	const uint NumSamples = 2048;

	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = Hammersley(i, NumSamples);
		float3 L = ImportanceSample_diffuse(Xi, N);
		float NoL = saturate(dot(N, L));

		if (NoL > 0)
		{
			PrefilteredColor += EnvMap.SampleLevel(SamplerAnisotropic, L, 0).rgb * NoL;
			TotalWeight += NoL;
		}
	}

	return PrefilteredColor / TotalWeight;
}
//=================================================================================================
// Pixel Shader
//=================================================================================================
float4 PS(PSInput input) : SV_TARGET
{
	float3 normal = GetNormal(Face, input.TexCoord);
	float  roughness = saturate(MipIndex / 6.0f); // Mip level is in [0, 6] range and roughness is [0, 1]

	return float4(PrefilterEnvMap(roughness, normal, Cubemap), 1);
}
float4 PS_diffuse(PSInput input) : SV_TARGET
{
	float3 normal = GetNormal(Face, input.TexCoord);
	return float4(Prefilter_DiffuseEnvMap(normal, Cubemap), 1);
}
technique11 IBL_Specular
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
technique11 IBL_Diffuse
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_diffuse()));
	}
}
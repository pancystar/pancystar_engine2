cbuffer cbPerFrame
{
	float gTexelWidth;
	float gTexelHeight;
};
cbuffer cbSettings
{
	float gWeights[11] =
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f
	};
};
cbuffer cbFixed
{
	static const int gBlurRadius = 5;
};
Texture2D gNormalDepthMap;
Texture2D gdepth_map;
Texture2D gInputImage;
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// Set a very far depth value if sampling outside of the NormalDepth map
	// so we do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(1e5f, 0.0f, 0.0f, 1e5f);
};
SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};
struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};
struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float2 Tex   : TEXCOORD;
};
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = float4(vin.PosL, 1.0f);
	vout.Tex = vin.Tex;
	return vout;
}
float4 PS(VertexOut pin, uniform bool gHorizontalBlur) : SV_Target
{
	float2 texOffset;
	if (gHorizontalBlur)
	{
		texOffset = float2(gTexelWidth, 0.0f);
	}
	else
	{
		texOffset = float2(0.0f, gTexelHeight);
	}
	//先以中心点的遮蔽值作为基础混合值
	float4 center_color = gInputImage.SampleLevel(samInputImage, pin.Tex, 0.0);
	float4 color = gWeights[5] * center_color;
	float totalWeight = gWeights[5];
	//中心点的深度采样
	float4 centerNormalDepth = gNormalDepthMap.SampleLevel(samNormalDepth, pin.Tex, 0.0f);
	float center_depth = gdepth_map.SampleLevel(samNormalDepth, pin.Tex, 0.0f).r;
	for (float i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		if (i == 0)
			continue;
		float2 tex = pin.Tex + i*texOffset;
		//采集边界点的深度信息用于判断是否应当被贡献
		float4 neighborNormalDepth = gNormalDepthMap.SampleLevel(samNormalDepth, tex, 0.0f);
		float neighbor_depth = gdepth_map.SampleLevel(samNormalDepth, tex, 0.0f).r;
		//采集边界点的遮蔽值用于计算模糊贡献值
		float4 neighborcolor = gInputImage.SampleLevel(samInputImage, tex, 0.0);
		//计算当前点的遮蔽情况
		float weight = gWeights[i + gBlurRadius];
		if (abs(dot(neighborNormalDepth.xyz, centerNormalDepth.xyz)) >= 0.8f &&abs(neighbor_depth - center_depth) <= 0.2f)
		{
			//发现当前点与中心点属于可混合的部分(首先是颜色差不大，然后就是深度差很小说明是同一物体，向量差也小说明是同一面，这种情况下才能够混合)
			color += weight*neighborcolor;
			totalWeight += weight;
		}
	}
	return color / totalWeight;
}

technique11 HorzBlur
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(true)));
	}
}

technique11 VertBlur
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(false)));
	}
}
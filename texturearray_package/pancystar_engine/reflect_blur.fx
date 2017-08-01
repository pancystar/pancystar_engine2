cbuffer cbPerFrame
{
	float4 tex_range_color_normal;
	uint4 blur_sample_level;
};
cbuffer cbSettings
{
	
	float gWeights[21] =
	{
		0.0476f, 0.0476f, 0.0476f, 0.0476f, 0.0476f, 0.0476f, 0.0476f, 0.0476f, 0.0476f, 0.0476f, 0.0476f,0.0476f, 0.0476f, 0.0476f, 0.0476f, 0.0476f, 0.0476f, 0.0476f, 0.0476f, 0.0476f, 0.0476f
	};
	float gWeights2[11] =
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f
	};
	/*
	float gWeights[11] =
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f
	};
	
	float gWeights[7] =
	{
	0.0289952651318174,0.103818351249974,0.223173360742083,0.288026045752251,0.223173360742083,0.103818351249974,0.0289952651318174
	};
	
	float gWeights[5] =
	{
	0.0545,0.2442,0.4026,0.2442,0.0545
	};*/
	
};
cbuffer cbFixed
{
	static const int gBlurRadius = 10;
	static const int gBlurRadius2 = 5;
};
Texture2D gInputImage;
Texture2DArray gInputImagearray;
Texture2D gInputMask;
Texture2D normal_tex;
Texture2D depth_tex;
//Texture2D normal_tex;
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// Set a very far depth value if sampling outside of the NormalDepth map
	// so we do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 1e5f);
};
SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 1e5f);
};
struct VertexIn//普通顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float2  tex1    : TEXCOORD;     //顶点纹理坐标
};
struct VertexOut
{
	float4 PosH       : SV_POSITION; //渲染管线必要顶点
	float2 Tex        : TEXCOORD1;   //纹理坐标
};
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = float4(vin.pos, 1.0f);
	vout.Tex = vin.tex1;
	return vout;
}
float4 PS_color(VertexOut pin, uniform bool gHorizontalBlur) : SV_Target
{
	float2 texOffset;
	float2 texoffset_normal;
	if (gHorizontalBlur)
	{
		texOffset = float2(tex_range_color_normal.x, 0.0f);
		texoffset_normal = float2(tex_range_color_normal.z, 0.0f);
	}
	else
	{
		texOffset = float2(0.0f, tex_range_color_normal.y);
		texoffset_normal = float2(0.0f, tex_range_color_normal.w);
	}
	float2 position_edge[8] =
	{
		float2(-1.0f,1.0f) , float2(0.0f,1.0f) ,   float2(1.0f,1.0f) ,
		float2(-1.0f,0.0f) ,                       float2(1.0f,0.0f) ,
		float2(-1.0f,-1.0f), float2(0.0f,-1.0f) ,  float2(1.0f,-1.0f)
	};
	//先以中心点的颜色值作为基础混合值
	float4 center_color = gInputImage.SampleLevel(samInputImage, pin.Tex, 0);
	float4 centerNormalDepth = normal_tex.SampleLevel(samNormalDepth, pin.Tex, 0.0f);
	float4 center_mask = gInputMask.SampleLevel(samNormalDepth, pin.Tex, 0.0f);
	return center_color;
	if ((center_color.r+ center_color.g+ center_color.b) < 0.01f)
	{
		float4 round_color = float4(0.0f,0.0f,0.0f,0.0f);
		float count = 0.0f;
		for (int i = 0; i < 8; ++i)
		{
			float2 tex_check = pin.Tex + gBlurRadius * position_edge[i] * tex_range_color_normal.xy;
			float4 color = gInputImage.SampleLevel(samNormalDepth, tex_check, 0.0f);
			if ((color.r + color.g + color.b) > 0.05f)
			{
				round_color += color;
				count += 1.0f;
			}
		}
		center_color = round_color / count;
	}
	/*
	float center_depth = depth_tex.SampleLevel(samNormalDepth, pin.Tex, 0.0f).r;
	//float4 center_normal = normal_tex.SampleLevel(samInputImage, pin.Tex, 0);
	float4 color = gWeights2[gBlurRadius2] * center_color;
	float total_weight = gWeights2[gBlurRadius2];
	float check_blur = 0.0f;
	float all_blur = 0.0f;
	
	//~~~~~~~~~模糊模板掩码~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	for (float i = -gBlurRadius2; i <= gBlurRadius2; ++i)
	{
		if (i == 0)
			continue;
		float2 tex = pin.Tex + i*texOffset;
		float2 texnormal = pin.Tex + i*texoffset_normal;
		//采集边界点的颜色
		float4 neighborcolor = gInputImage.SampleLevel(samInputImage, tex, 0);
		float4 neighborNormalDepth = normal_tex.SampleLevel(samNormalDepth, texnormal, 0.0f);
		float neighbor_depth = depth_tex.SampleLevel(samNormalDepth, texnormal, 0.0f).r;
		if (abs(dot(neighborNormalDepth.xyz, centerNormalDepth.xyz)) >= 0.6f &&abs(neighbor_depth - center_depth) <= 0.2f && (neighborcolor.r + neighborcolor.g + neighborcolor.b) > 0.1f)
		{
			float weight = gWeights2[i + gBlurRadius2];
			color += weight*neighborcolor;
			total_weight += weight;
			all_blur += 1.0f;
		}
	}
	color /= total_weight;
	*/
	return center_color;
}
float4 PS_mipblur(VertexOut pin, uniform bool gHorizontalBlur) : SV_Target
{
	float2 texOffset;
	float2 texoffset_normal;
	if (gHorizontalBlur)
	{
		texOffset = float2(tex_range_color_normal.x, 0.0f);
		texoffset_normal = float2(tex_range_color_normal.z, 0.0f);
	}
	else
	{
		texOffset = float2(0.0f, tex_range_color_normal.y);
		texoffset_normal = float2(0.0f, tex_range_color_normal.w);
	}
	//先以中心点的遮蔽值作为基础混合值
	float4 center_color = gInputImagearray.Sample(samInputImage, float3(pin.Tex, blur_sample_level.x));
	float4 color = gWeights[gBlurRadius] * center_color;
	float totalWeight = gWeights[gBlurRadius];
	//中心点的深度采样
	float4 centerNormalDepth = normal_tex.SampleLevel(samNormalDepth, pin.Tex, 0.0f);
	float center_depth = depth_tex.SampleLevel(samNormalDepth, pin.Tex, 0.0f).r;
	float all_blur = 0.0f;
	for (float i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		if (i == 0)
			continue;
		float2 tex = pin.Tex + i*texOffset;
		float2 texnormal = pin.Tex + i*texoffset_normal;
		//采集边界点的颜色
		float4 neighborcolor = gInputImagearray.Sample(samInputImage, float3(tex, blur_sample_level.x));
		float4 neighborNormalDepth = normal_tex.SampleLevel(samNormalDepth, texnormal, 0.0f);
		float neighbor_depth = depth_tex.SampleLevel(samNormalDepth, texnormal, 0.0f).r;
		if (abs(dot(neighborNormalDepth.xyz, centerNormalDepth.xyz)) >= 0.6f &&abs(neighbor_depth - center_depth) <= 0.2f && (neighborcolor.r + neighborcolor.g + neighborcolor.b) > 0.1f)
		{
			
			float weight = gWeights[i + gBlurRadius];
			totalWeight += weight;
			all_blur += 1.0f;
			color += weight*neighborcolor;
			
		}
	}
	return color / totalWeight;
}
technique11 HorzBlur_color
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_color(true)));
	}
}

technique11 VertBlur_color
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_color(false)));
	}
}

technique11 HorzBlur_mipmap
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_mipblur(true)));
	}
}

technique11 VertBlur_color_mipmap
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_mipblur(false)));
	}
}
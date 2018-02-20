/*延迟光照效果*/
#include"light_count.hlsli"
#include"atmosphere_renderfunc.hlsli"
cbuffer per_environment 
{
	float exposure;
	float3 white_point_in;
	float3 earth_center;
	float2 sun_size;
	float4x4 view_from_clip;
};
cbuffer perframe
{
	pancy_light_basic   sun_light;
	uint3               sun_light_num;
	float4              depth_devide;
	float4x4            sunlight_shadowmat[6];
	pancy_light_basic   light_need[100];      //光源
	float4x4            shadowmap_matrix[20]; //阴影贴图变换
	float4x4            view_matrix;          //取景变换
	float4x4            invview_matrix;       //取景变换的逆
	uint3               light_num;            //光源数量
	uint3               shadow_num;           //阴影数量
	float4              gFrustumCorners[4];   //3D重建的四个角，用于借助光栅化插值
	float3              proj_desc;            //投影参数，用于还原深度信息
	float3              camera;               //视点位置
}; 
Texture2DArray texture_sunshadow;//太阳光阴影贴图
Texture2DArray texture_shadow;   //阴影贴图
Texture2D gNormalspecMap;        //屏幕空间法线&镜面反射强度纹理
Texture2D gSpecRoughnessMap;     //屏幕空间镜面&粗糙度纹理
Texture2D gdepth_map;            //屏幕空间深度纹理
Texture2D render_mask;           //大气散射掩码
SamplerState samTex
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
	AddressU = WRAP;
	AddressV = WRAP;
};
SamplerComparisonState samShadow
{
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	ComparisonFunc = LESS_EQUAL;
};
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};
float CalcShadowFactor(SamplerComparisonState samShadow, Texture2DArray shadowMap, int shadowtex_num, float4 shadowPosH)
{
	//归一化
	shadowPosH.xyz /= shadowPosH.w;

	//采集光源投影后的深度
	float depth = shadowPosH.z-0.0005f;

	//阴影贴图的步长
	const float dx = 1.0f / 1024.0f;
	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		float2 rec_pos = shadowPosH.xy + offsets[i];
		if (rec_pos.x > 1.0f || rec_pos.x < 0.0f || rec_pos.y > 1.0f || rec_pos.y < 0.0f)
		{
			percentLit += 1.0f;
		}
		else
		{
			/*
			float depth_rec = shadowMap.Sample(samNormalDepth, float3(rec_pos, shadowtex_num)).r; //Sample(samShadow, float3(rec_pos, shadowtex_num)).r;
			if (depth_rec < depth)
			{
			percentLit += 1;
			}
			*/
			percentLit += shadowMap.SampleCmpLevelZero(samShadow, float3(rec_pos, shadowtex_num), depth).r;
		}
	}
	//return 1.0f;
	return percentLit /= 9.0f;
}
struct VertexIn//普通顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float2  tex1    : TEXCOORD;     //顶点纹理坐标
};
struct VertexOut
{
	float4 PosH       : SV_POSITION; //渲染管线必要顶点
	float3 ToFarPlane : TEXCOORD0;   //用于3D重建
	float2 Tex        : TEXCOORD1;   //纹理坐标
	float3 view_ray   : TEXCOORD2;   //纹理坐标
};
struct PixelOut_high
{
	float4 diffuse            : SV_TARGET0;
	float4 specular           : SV_TARGET1;
	float4 atmosphereblend    : SV_TARGET2;
};

static const float3 kSphereAlbedo = float3(0.8, 0.8, 0.8);
static const float3 kGroundAlbedo = float3(0.0, 0.0, 0.04);
//大气散射光照运算
#ifdef USE_LUMINANCE
#define GetSolarRadiance GetSolarLuminance
#define GetSkyRadiance GetSkyLuminance
#define GetSkyRadianceToPoint GetSkyLuminanceToPoint
#define GetSunAndSkyIrradiance GetSunAndSkyIlluminance
#endif
float3 count_normal_sphereradiance(float pz,VertexOut pin, float render_check, float fragment_angular_size, out float sphere_alpha,out float3 transmittance,out float3 in_scatter)
{
	float ray_sphere_angular_distance = 10;
	sphere_alpha = min(ray_sphere_angular_distance / fragment_angular_size, render_check);
	//还原点的世界坐标
	float4 normalDepth = gNormalspecMap.Sample(samNormalDepth, pin.Tex);
	float3 n = normalDepth.xyz;
	float3 p = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;
	float3 point_in = mul(float4(p, 1.0f), invview_matrix).xyz;
	float3 normal = mul(float4(n, 0.0f), invview_matrix).xyz;
	// Compute the radiance reflected by the sphere.
	float3 sky_irradiance;
	float3 rec_dir = sun_light.dir;
	rec_dir.x = -rec_dir.x;
	rec_dir.y = -rec_dir.y;
	float minus_hack = 1.0f;
	if (rec_dir.y < 0.0f)
	{
		minus_hack = max(0.0f, 0.2f + rec_dir.y);
		rec_dir.y = 0.0f;
	}
	float3 sun_irradiance = GetSunAndSkyIrradiance(point_in - earth_center, normal, rec_dir, sky_irradiance);
	//sphere_radiance = sun_irradiance;

	//float3 sphere_radiance = kSphereAlbedo * (1.0 / PI) * (sun_irradiance + sky_irradiance)
	float3 sphere_radiance = sun_irradiance + sky_irradiance;
	//float shadow_length = max(0.0, min(shadow_out, distance_to_intersection) - shadow_in) * lightshaft_fadein_hack;
	float shadow_length = 0.5f;
	//float3 transmittance;
	in_scatter = GetSkyRadianceToPoint(float3(0, 2, 5) - earth_center, point_in - earth_center, shadow_length, rec_dir, transmittance);
	//sphere_radiance = sphere_radiance * transmittance + in_scatter;
	return minus_hack * sphere_radiance * render_check;
	
}
float3 count_sky_radiance(float3 view_direction)
{
	float3 rec_dir = sun_light.dir;
	rec_dir.x = -rec_dir.x;
	rec_dir.y = -rec_dir.y;
	float minus_hack = 1.0f;
	if (rec_dir.y < 0.0f) 
	{
		minus_hack = max(0.0f,1.0f + 3.0f*rec_dir.y);
		rec_dir.y = 0.0f;
	}
	float shadow_length = 0.0f;;
	float3 transmittance;
	//+camera.y*0.0014f;
	float3 radiance = GetSkyRadiance(float3(0,camera.y,5) - earth_center, normalize(float3(view_direction.x, view_direction.y, view_direction.z)), shadow_length, rec_dir, transmittance);
	//float3 radiance2 = GetSkyRadiance(float3(0, 2, 5) - earth_center, normalize(float3(view_direction.x, view_direction.y + camera.y*0.0014f, view_direction.z)), shadow_length, rec_dir, transmittance);
	//if (dot(view_direction, rec_dir) > sun_size.y)
	//{
	//	radiance = radiance + transmittance * GetSolarRadiance();
	//}
	float3 rgb_color = pow(float3(1.0f, 1.0f, 1.0f) - exp(-radiance / white_point_in * exposure), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
	//float3 rgb_color = float3(1.0f, 1.0f, 1.0f) - exp(-radiance / white_point_in * exposure);
	return minus_hack * rgb_color;
}
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	//因为是前一个shader光栅化完毕的像素点，不需要做任何变化
	vout.PosH = float4(vin.pos, 1.0f);
	//把四个顶点设置完毕（四个角点的次序存储在法线的x分量里面）
	vout.ToFarPlane = gFrustumCorners[vin.normal.x].xyz;
	//记录下纹理坐标
	vout.Tex = vin.tex1;
	//记录光线追踪射线
	float4 pos_view = float4(mul(float4(vin.pos, 1.0f), view_from_clip).xyz, 0.0f);
	vout.view_ray = mul(pos_view, invview_matrix).xyz;
	return vout;
}


PixelOut_high count_common_lighting(VertexOut pin, float pz, float shadow_mask)
{
	PixelOut_high pout;
	pout.diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	pout.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//还原点的世界坐标
	float4 normalspec = gNormalspecMap.Sample(samNormalDepth, pin.Tex);
	float3 normal_need = normalspec.xyz;
	float3 position_need = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;
	pancy_material material_need;
	material_need.ambient = float4(1.0f, 1.0f, 1.0f, 0.0f);
	material_need.diffuse = float4(1.0f, 1.0f, 1.0f, 0.0f);
	material_need.specular = float4(1.0f, 1.0f, 1.0f, normalspec.a);
	int count_shadow = 0;
	//~~~~~~~~~~~~~~~~~~~视线空间着色~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//[unroll]
	float4 A = 0.0f, D = 0.0f, S = 0.0f;
	//先计算太阳光
	pancy_light_basic rec_sunlight = sun_light;
	float3 position_view_sun = float3(0.0f, 0.0f, 0.0f);
	float3 eye_direct_sun = normalize(position_view_sun - position_need);
	rec_sunlight.dir = mul(float4(sun_light.dir, 0.0f), view_matrix).xyz;
	//计算大气光散射
	pout.atmosphereblend = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 view_direction = normalize(pin.view_ray);
	float fragment_angular_size = length(ddx(pin.view_ray) + ddy(pin.view_ray)) / length(pin.view_ray);
	float sphere_alpha = 0.0;
	//float3 sphere_radiance = float3(0.0f, 0.0f, 0.0f);
	float render_check = render_mask.Sample(samNormalDepth, pin.Tex).r;
	float3 transmittance_out;
	float3 in_scatter_out;
	rec_sunlight.diffuse.rgb = count_normal_sphereradiance(pz, pin, render_check, fragment_angular_size, sphere_alpha, transmittance_out, in_scatter_out);
	D.rgb = D.rgb * transmittance_out + in_scatter_out;
	//S.rgb = S.rgb * transmittance_out + in_scatter_out;
	pout.atmosphereblend.r = sphere_alpha;
	rec_sunlight.diffuse.a = 1.0f;
	rec_sunlight.specular = rec_sunlight.diffuse;
	//计算太阳光
	compute_dirlight(material_need, rec_sunlight, normal_need, eye_direct_sun, A, D, S);

	int count_sunlight;
	if (position_need.z < depth_devide.x)
	{
		count_sunlight = 0;
	}
	else if (position_need.z < depth_devide.y)
	{
		count_sunlight = 1;
	}
	else if (position_need.z < depth_devide.z)
	{
		count_sunlight = 2;
	}
	else
	{
		count_sunlight = 3;
	}
	float4 pos_shadow_world = mul(float4(position_need, 1.0f), invview_matrix);
	float4 pos_shadow = mul(pos_shadow_world, sunlight_shadowmat[count_sunlight]);
	float rec_shadow = max(CalcShadowFactor(samShadow, texture_sunshadow, count_sunlight, pos_shadow), shadow_mask);
	pout.diffuse += rec_shadow*D;
	pout.specular += rec_shadow*S;
	//再计算普通光
	int count_all = 0;
	for (uint i = 0; i < shadow_num.x; ++i)
	{
		pancy_light_basic rec_light = light_need[count_all];
		rec_light.dir = mul(float4(light_need[count_all].dir, 0.0f), view_matrix).xyz;
		float3 position_view = float3(0.0f, 0.0f, 0.0f);
		float3 eye_direct = normalize(position_view - position_need);
		compute_dirlight(material_need, rec_light, normal_need, eye_direct, A, D, S);

		float4 pos_shadow_world = mul(float4(position_need, 1.0f), invview_matrix);
		float4 pos_shadow = mul(pos_shadow_world, shadowmap_matrix[count_all]);
		float rec_shadow = max(CalcShadowFactor(samShadow, texture_shadow, count_all, pos_shadow), shadow_mask);
		pout.diffuse += (0.2f + 0.8f*rec_shadow)*D;
		pout.specular += (0.2f + 0.8f*rec_shadow)*S;
		count_all += 1;
	}
	for (uint i = 0; i < shadow_num.z; ++i)
	{
		pancy_light_basic rec_light = light_need[count_all];
		rec_light.position = mul(float4(light_need[count_all].position, 1.0f), view_matrix).xyz;
		rec_light.dir = mul(float4(light_need[count_all].dir, 0.0f), view_matrix).xyz;
		float3 position_view = float3(0.0f, 0.0f, 0.0f);
		float3 eye_direct = normalize(position_view - position_need);
		compute_spotlight(material_need, rec_light, position_need, normal_need, eye_direct, A, D, S);

		float4 pos_shadow_world = mul(float4(position_need, 1.0f), invview_matrix);
		float4 pos_shadow = mul(pos_shadow_world, shadowmap_matrix[count_all]);
		float rec_shadow = max(CalcShadowFactor(samShadow, texture_shadow, count_all, pos_shadow), shadow_mask);
		pout.diffuse += (0.2f + 0.8f*rec_shadow)*D;
		pout.specular += (0.2f + 0.8f*rec_shadow)*S;
		count_all += 1;
	}
	for (uint i = 0; i < light_num.x; ++i)
	{
		pancy_light_basic rec_light = light_need[count_all];
		rec_light.position = mul(float4(light_need[count_all].position, 1.0f), view_matrix).xyz;
		rec_light.dir = mul(float4(light_need[count_all].dir, 0.0f), view_matrix).xyz;
		float3 position_view = float3(0.0f, 0.0f, 0.0f);
		float3 eye_direct = normalize(position_view - position_need);
		compute_dirlight(material_need, rec_light, normal_need, eye_direct, A, D, S);
		pout.diffuse += D;
		pout.specular += S;
		count_all += 1;
	}
	for (uint i = 0; i < light_num.y; ++i)
	{
		pancy_light_basic rec_light = light_need[count_all];
		rec_light.position = mul(float4(light_need[count_all].position, 1.0f), view_matrix).xyz;
		rec_light.dir = mul(float4(light_need[count_all].dir, 0.0f), view_matrix).xyz;
		float3 position_view = float3(0.0f, 0.0f, 0.0f);
		float3 eye_direct = normalize(position_view - position_need);
		compute_pointlight(material_need, rec_light, position_need, normal_need, position_view, A, D, S);
		pout.diffuse += D;
		pout.specular += S;
		count_all += 1;
	}
	for (uint i = 0; i < light_num.z; ++i)
	{
		pancy_light_basic rec_light = light_need[count_all];
		rec_light.position = mul(float4(light_need[count_all].position, 1.0f), view_matrix).xyz;
		rec_light.dir = mul(float4(light_need[count_all].dir, 0.0f), view_matrix).xyz;
		float3 position_view = float3(0.0f, 0.0f, 0.0f);
		float3 eye_direct = normalize(position_view - position_need);
		compute_spotlight(material_need, rec_light, position_need, normal_need, eye_direct, A, D, S);
		pout.diffuse += D;
		pout.specular += S;
		count_all += 1;
	}
	return pout;
}
PixelOut_high count_pbr_lighting(VertexOut pin, float pz,float shadow_mask) 
{
	PixelOut_high pout;
	pout.diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	pout.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//还原点的世界坐标
	float4 normalspec = gNormalspecMap.Sample(samNormalDepth, pin.Tex);
	float4 specrough = gSpecRoughnessMap.Sample(samNormalDepth, pin.Tex);
	float3 normal_need = normalspec.xyz;
	float3 position_need = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;
	pancy_material material_need;
	material_need.ambient = float4(1.0f, 1.0f, 1.0f, 0.0f);
	material_need.diffuse = float4(1.0f, 1.0f, 1.0f, 0.0f);
	material_need.specular = float4(1.0f, 1.0f, 1.0f, normalspec.a);
	int count_shadow = 0;
	//~~~~~~~~~~~~~~~~~~~视线空间着色~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//[unroll]
	float4 A = 0.0f, D = 0.0f, S = 0.0f;
	//先计算太阳光
	pancy_light_basic rec_sunlight = sun_light;
	float3 position_view_sun = float3(0.0f, 0.0f, 0.0f);
	float3 eye_direct_sun = normalize(position_view_sun - position_need);
	rec_sunlight.dir = mul(float4(sun_light.dir, 0.0f), view_matrix).xyz;
	//计算大气光散射
	pout.atmosphereblend = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 view_direction = normalize(pin.view_ray);
	float fragment_angular_size = length(ddx(pin.view_ray) + ddy(pin.view_ray)) / length(pin.view_ray);
	float sphere_alpha = 0.0;
	//float3 sphere_radiance = float3(0.0f, 0.0f, 0.0f);
	float render_check = render_mask.Sample(samNormalDepth, pin.Tex).r;

	float3 transmittance_out;
	float3 in_scatter_out;
	rec_sunlight.diffuse.rgb = count_normal_sphereradiance(pz,pin, render_check, fragment_angular_size, sphere_alpha, transmittance_out, in_scatter_out);
	//rec_sunlight.diffuse.rgb = float3(0.8f, 0.8f, 0.8f);
	pout.atmosphereblend.rgb = count_sky_radiance(view_direction);
	//pout.atmosphereblend.rgb = view_direction;
	pout.atmosphereblend.a = sphere_alpha;

	rec_sunlight.diffuse.a = 1.0f;
	rec_sunlight.specular = rec_sunlight.diffuse;

	//pout.diffuse = rec_sunlight.diffuse;
	//pout.specular = rec_sunlight.diffuse;
	//return pout;
	//计算太阳光pbr
	compute_dirlight_pbr(specrough.rgb, normalspec.a, specrough.a, rec_sunlight, normal_need, eye_direct_sun, D, S);
	float3 light_dir_rec = normalize(rec_sunlight.dir);
	float diffuse_angle = dot(-light_dir_rec, normal_need); //漫反射夹角
	//D.rgb = D.rgb * transmittance_out + in_scatter_out;
	//S.rgb = S.rgb * transmittance_out + in_scatter_out;
	int count_sunlight;
	if (position_need.z < depth_devide.x)
	{
		count_sunlight = 0;
	}
	else if (position_need.z < depth_devide.y)
	{
		count_sunlight = 1;
	}
	else if (position_need.z < depth_devide.z)
	{
		count_sunlight = 2;
	}
	else
	{
		count_sunlight = 3;
	}
	float4 pos_shadow_world = mul(float4(position_need, 1.0f), invview_matrix);
	float4 pos_shadow = mul(pos_shadow_world, sunlight_shadowmat[count_sunlight]);
	float rec_shadow = max(CalcShadowFactor(samShadow, texture_sunshadow, count_sunlight, pos_shadow), shadow_mask);

	
	pout.diffuse += 0.5f * (0.2f + 0.8f * rec_shadow) * (D / diffuse_angle);
	pout.specular += 0.5f *(0.2f + 0.8f * rec_shadow) * (S / diffuse_angle);
	//再计算普通光
	int count_all = 0;
	for (uint i = 0; i < shadow_num.x; ++i)
	{
		pancy_light_basic rec_light = light_need[count_all];
		rec_light.dir = mul(float4(light_need[count_all].dir, 0.0f), view_matrix).xyz;
		float3 position_view = float3(0.0f, 0.0f, 0.0f);
		float3 eye_direct = normalize(position_view - position_need);
		//compute_dirlight(material_need, rec_light, normal_need, eye_direct, A, D, S);
		compute_dirlight_pbr(specrough.rgb, normalspec.a, specrough.a, rec_light, normal_need, eye_direct_sun, D, S);
		float4 pos_shadow_world = mul(float4(position_need, 1.0f), invview_matrix);
		float4 pos_shadow = mul(pos_shadow_world, shadowmap_matrix[count_all]);
		float rec_shadow = max(CalcShadowFactor(samShadow, texture_shadow, count_all, pos_shadow), shadow_mask);
		pout.diffuse += (0.2f + 0.8f*rec_shadow)*D;
		pout.specular += (0.2f + 0.8f*rec_shadow)*S;
		count_all += 1;
	}
	for (uint i = 0; i < shadow_num.z; ++i)
	{
		pancy_light_basic rec_light = light_need[count_all];
		rec_light.position = mul(float4(light_need[count_all].position, 1.0f), view_matrix).xyz;
		rec_light.dir = mul(float4(light_need[count_all].dir, 0.0f), view_matrix).xyz;
		float3 position_view = float3(0.0f, 0.0f, 0.0f);
		float3 eye_direct = normalize(position_view - position_need);
		compute_spotlight_pbr(specrough.rgb, normalspec.a, specrough.a, rec_light, position_need, normal_need, eye_direct, D, S);

		float4 pos_shadow_world = mul(float4(position_need, 1.0f), invview_matrix);
		float4 pos_shadow = mul(pos_shadow_world, shadowmap_matrix[count_all]);
		float rec_shadow = max(CalcShadowFactor(samShadow, texture_shadow, count_all, pos_shadow), shadow_mask);
		pout.diffuse += (0.2f + 0.8f*rec_shadow) * D;
		pout.specular += (0.2f + 0.8f*rec_shadow) * S;
		count_all += 1;
	}
	for (uint i = 0; i < light_num.x; ++i)
	{
		pancy_light_basic rec_light = light_need[count_all];
		rec_light.position = mul(float4(light_need[count_all].position, 1.0f), view_matrix).xyz;
		rec_light.dir = mul(float4(light_need[count_all].dir, 0.0f), view_matrix).xyz;
		float3 position_view = float3(0.0f, 0.0f, 0.0f);
		float3 eye_direct = normalize(position_view - position_need);
		//compute_dirlight(material_need, rec_light, normal_need, eye_direct, A, D, S);
		compute_dirlight_pbr(specrough.rgb, normalspec.a, specrough.a, rec_light, normal_need, eye_direct, D, S);
		pout.diffuse += D;
		pout.specular += S;
		count_all += 1;
	}
	for (uint i = 0; i < light_num.y; ++i)
	{
		pancy_light_basic rec_light = light_need[count_all];
		rec_light.position = mul(float4(light_need[count_all].position, 1.0f), view_matrix).xyz;
		rec_light.dir = mul(float4(light_need[count_all].dir, 0.0f), view_matrix).xyz;
		float3 position_view = float3(0.0f, 0.0f, 0.0f);
		float3 eye_direct = normalize(position_view - position_need);
		//compute_pointlight(material_need, rec_light, position_need, normal_need, position_view, A, D, S);
		compute_pointlight_pbr(specrough.rgb, normalspec.a, specrough.a, rec_light, position_need, normal_need, position_view, D, S);
		pout.diffuse += D;
		pout.specular += S;
		count_all += 1;
	}
	for (uint i = 0; i < light_num.z; ++i)
	{
		pancy_light_basic rec_light = light_need[count_all];
		rec_light.position = mul(float4(light_need[count_all].position, 1.0f), view_matrix).xyz;
		rec_light.dir = mul(float4(light_need[count_all].dir, 0.0f), view_matrix).xyz;
		float3 position_view = float3(0.0f, 0.0f, 0.0f);
		float3 eye_direct = normalize(position_view - position_need);
		//compute_spotlight(material_need, rec_light, position_need, normal_need, eye_direct, A, D, S);
		compute_spotlight_pbr(specrough.rgb, normalspec.a, specrough.a, rec_light, position_need, normal_need, eye_direct, D, S);
		pout.diffuse += D;
		pout.specular += S;
		count_all += 1;
	}
	return pout;
}

PixelOut_high PS(VertexOut pin)
{
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	return count_common_lighting(pin, pz,0.0f);
}
PixelOut_high PS_PBR(VertexOut pin) 
{
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	return count_pbr_lighting(pin, pz, 0.0f);
}
PixelOut_high PS_withoutMSAA(VertexOut pin)
{
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	pz = 1.0f / (pz * proj_desc.x + proj_desc.y);
	return count_common_lighting(pin, pz, 0.0f);
}
PixelOut_high PS_PBR_withoutMSAA(VertexOut pin)
{
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	pz = 1.0f / (pz * proj_desc.x + proj_desc.y);
	float3 position_need = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;
	return count_pbr_lighting(pin, pz, 0.0f);
}

PixelOut_high PS_withoutshadow(VertexOut pin)
{
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	return count_common_lighting(pin, pz, 1.0f);
}
PixelOut_high PS_PBR_withoutshadow(VertexOut pin) 
{
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	return count_pbr_lighting(pin, pz, 1.0f);
}
PixelOut_high PS_withoutshadowMSAA(VertexOut pin)
{
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	pz = 1.0f / (pz * proj_desc.x + proj_desc.y);
	return count_common_lighting(pin, pz, 1.0f);
}
PixelOut_high PS_PBR_withoutshadowMSAA(VertexOut pin)
{
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	pz = 1.0f / (pz * proj_desc.x + proj_desc.y);
	float3 position_need = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;
	return count_pbr_lighting(pin, pz, 1.0f);
}

technique11 draw_common
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
technique11 draw_withoutshadow
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withoutshadow()));
	}
}
technique11 draw_common_pbr
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_PBR()));
	}
}
technique11 draw_pbr_withoutshadow
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_PBR_withoutshadow()));
	}
}

technique11 draw_withoutMSAA
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withoutMSAA()));
	}
}
technique11 draw_withoutshadowMSAA
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withoutshadowMSAA()));
	}
}
technique11 draw_pbr_withoutMSAA
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_PBR_withoutMSAA()));
	}
}
technique11 draw_pbr_withoutshadowMSAA
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_PBR_withoutshadowMSAA()));
	}
}

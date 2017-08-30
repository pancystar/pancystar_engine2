#include"atmosphere_renderfunc.hlsli"



cbuffer per_frame
{
	float3 camera;
	float exposure;
	float3 white_point_in;
	float3 earth_center;
	float3 sun_direction;
	float2 sun_size;
	float4x4 model_from_view;
	float4x4 view_from_clip;
	float4   gFrustumCorners[4];   //3D重建的四个角，用于借助光栅化插值
};
Texture2D render_mask;
Texture2D gNormalDepthMap;
Texture2D gdepth_map;
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// Set a very far depth value if sampling outside of the NormalDepth map
	// so we do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(1e5f, 0.0f, 0.0f, 1e5f);
};

static const float3 kSphereCenter = float3(0.0, 0.0, 1.0);
static const float kSphereRadius = 1.0;
static const float3 kSphereAlbedo = float3(0.8, 0.8, 0.8);
static const float3 kGroundAlbedo = float3(0.0, 0.0, 0.04);

#ifdef USE_LUMINANCE
#define GetSolarRadiance GetSolarLuminance
#define GetSkyRadiance GetSkyLuminance
#define GetSkyRadianceToPoint GetSkyLuminanceToPoint
#define GetSunAndSkyIrradiance GetSunAndSkyIlluminance
#endif

//float3 GetSolarRadiance();
//float3 GetSkyRadiance(float3 camera, float3 view_ray, float shadow_length, float3 sun_direction, out float3 transmittance);
//float3 GetSkyRadianceToPoint(float3 camera, float3 point_in, float shadow_length, float3 sun_direction, out float3 transmittance);
//float3 GetSunAndSkyIrradiance(float3 p, float3 normal, float3 sun_direction, out float3 sky_irradiance);

float GetSunVisibility(float3 point_in, float3 sun_direction) {
	float3 p = point_in - kSphereCenter;
	float p_dot_v = dot(p, sun_direction);
	float p_dot_p = dot(p, p);
	float ray_sphere_center_squared_distance = p_dot_p - p_dot_v * p_dot_v;
	float distance_to_intersection = -p_dot_v - sqrt(
		kSphereRadius * kSphereRadius - ray_sphere_center_squared_distance);
	if (distance_to_intersection > 0.0) {
		// Compute the distance between the view ray and the sphere, and the
		// corresponding (tangent of the) subtended angle. Finally, use this to
		// compute an approximate sun visibility.
		float ray_sphere_distance =
			kSphereRadius - sqrt(ray_sphere_center_squared_distance);
		float ray_sphere_angular_distance = -ray_sphere_distance / p_dot_v;
		return smoothstep(1.0, 0.0, ray_sphere_angular_distance / sun_size.x);
	}
	return 1.0;
}

/*
<p>The sphere also partially occludes the sky light, and we approximate this
effect with an ambient occlusion factor. The ambient occlusion factor due to a
sphere is given in <a href=
"http://webserver.dmt.upm.es/~isidoro/tc3/Radiation%20View%20factors.pdf"
>Radiation View Factors</a> (Isidoro Martinez, 1995). In the simple case where
the sphere is fully visible, it is given by the following function:
*/

float GetSkyVisibility(float3 point_in)
{
	float3 p = point_in - kSphereCenter;
	float p_dot_p = dot(p, p);
	return 1.0 + p.z / sqrt(p_dot_p) * kSphereRadius * kSphereRadius / p_dot_p;
}
void GetSphereShadowInOut(float3 view_direction, float3 sun_direction,
	out float d_in, out float d_out) {
	float3 pos = camera - kSphereCenter;
	float pos_dot_sun = dot(pos, sun_direction);
	float view_dot_sun = dot(view_direction, sun_direction);
	float k = sun_size.x;
	float l = 1.0 + k * k;
	float a = 1.0 - l * view_dot_sun * view_dot_sun;
	float b = dot(pos, view_direction) - l * pos_dot_sun * view_dot_sun -
		k * kSphereRadius * view_dot_sun;
	float c = dot(pos, pos) - l * pos_dot_sun * pos_dot_sun -
		2.0 * k * kSphereRadius * pos_dot_sun - kSphereRadius * kSphereRadius;
	float discriminant = b * b - a * c;
	if (discriminant > 0.0) {
		d_in = max(0.0, (-b - sqrt(discriminant)) / a);
		d_out = (-b + sqrt(discriminant)) / a;
		// The values of d for which delta is equal to 0 and kSphereRadius / k.
		float d_base = -pos_dot_sun / view_dot_sun;
		float d_apex = -(pos_dot_sun + kSphereRadius / k) / view_dot_sun;
		if (view_dot_sun > 0.0) {
			d_in = max(d_in, d_apex);
			d_out = a > 0.0 ? min(d_out, d_base) : d_base;
		}
		else {
			d_in = a > 0.0 ? max(d_in, d_base) : d_base;
			d_out = min(d_out, d_apex);
		}
	}
	else {
		d_in = 0.0;
		d_out = 0.0;
	}
}
struct VertexIn
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float2  tex1    : TEXCOORD;     //顶点纹理坐标
};
struct VertexOut
{
	float4 PosH       : SV_POSITION; //渲染管线必要顶点
	float2 Tex        : TEXCOORD0;   //纹理坐标
	float3 view_ray   : TEXCOORD1;   //光线追踪射线
	float3 ToFarPlane : TEXCOORD2;   //用于3D重建
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	float3 now_pos;
	vout.PosH = float4(vin.pos, 1.0f);
	float4 pos_view = float4(mul(float4(vin.pos, 1.0f), view_from_clip).xyz, 0.0f);
	vout.view_ray = mul(pos_view, model_from_view).xyz;
	//float rec = vout.view_ray.z;
	//vout.view_ray.z = -vout.view_ray.y;
	//vout.view_ray.y = rec;
	vout.Tex = vin.tex1;
	vout.ToFarPlane = gFrustumCorners[vin.normal.x].xyz;
	return vout;
}
float3 count_normal_sphereradiance(VertexOut pin,float render_check, float ray_sphere_center_squared_distance,float fragment_angular_size,out float sphere_alpha)
{
	//if (render_check < 0.2f) 
	//{
	//	sphere_alpha = 0.0f;
	//	return float3(0.0f,0.0f,0.0f);
	//}
	//float ray_sphere_distance = kSphereRadius - sqrt(ray_sphere_center_squared_distance);
	float ray_sphere_angular_distance = 10;
	sphere_alpha = min(ray_sphere_angular_distance / fragment_angular_size, render_check);
	//还原点的世界坐标
	float4 normalDepth = gNormalDepthMap.Sample(samNormalDepth, pin.Tex);
	float3 n = normalDepth.xyz;
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	//pz = 0.1f / (1.0f - pz);
	float3 p = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;
	float3 point_in = mul(float4(p, 1.0f), model_from_view).xyz;
	float3 normal = mul(float4(n, 0.0f), model_from_view).xyz;
	// Compute the radiance reflected by the sphere.
	float3 sky_irradiance;
	float3 sun_irradiance = GetSunAndSkyIrradiance(point_in - earth_center, normal, sun_direction, sky_irradiance);
	//sphere_radiance = sun_irradiance;
	
	float3 sphere_radiance = kSphereAlbedo * (1.0 / PI) * (sun_irradiance + sky_irradiance);
	//float shadow_length = max(0.0, min(shadow_out, distance_to_intersection) - shadow_in) * lightshaft_fadein_hack;
	float shadow_length = 0.0f;
	float3 transmittance;
	float3 in_scatter = GetSkyRadianceToPoint(camera - earth_center, point_in - earth_center, shadow_length, sun_direction, transmittance);
	sphere_radiance = sphere_radiance * transmittance + in_scatter;
	return sphere_radiance * render_check;
}
float4 PS_atmosphere(VertexOut pin) : SV_Target
{
	// Normalized view direction vector.
	float3 view_direction = normalize(pin.view_ray);
	// Tangent of the angle subtended by this fragment.
	float fragment_angular_size = length(ddx(pin.view_ray) + ddy(pin.view_ray)) / length(pin.view_ray);
	
	//float shadow_in;
	//float shadow_out;
	//GetSphereShadowInOut(view_direction, sun_direction, shadow_in, shadow_out);
	// Hack to fade out light shafts when the Sun is very close to the horizon.
	float lightshaft_fadein_hack = smoothstep(0.02, 0.04, dot(normalize(camera - earth_center), sun_direction));

	float3 p = camera - kSphereCenter;
	float p_dot_v = dot(p, view_direction);
	float p_dot_p = dot(p, p);
	float ray_sphere_center_squared_distance = p_dot_p - p_dot_v * p_dot_v;
	float distance_to_intersection = -p_dot_v - sqrt(kSphereRadius * kSphereRadius - ray_sphere_center_squared_distance);
	
	// Compute the radiance reflected by the sphere, if the ray intersects it.
	float sphere_alpha = 0.0;
	float3 sphere_radiance = float3(0.0f,0.0f,0.0f);
	
	float render_check = render_mask.Sample(samNormalDepth, pin.Tex).r;
	sphere_radiance = count_normal_sphereradiance(pin,render_check,ray_sphere_center_squared_distance,fragment_angular_size,sphere_alpha);
	/*
	if (render_check > 0.5f)
	{
		float ray_sphere_distance = kSphereRadius - sqrt(ray_sphere_center_squared_distance);
		float ray_sphere_angular_distance = 10;
		sphere_alpha = min(ray_sphere_angular_distance / fragment_angular_size, 1.0);
		//还原点的世界坐标
		float4 normalDepth = gNormalDepthMap.Sample(samNormalDepth, pin.Tex);
		float3 n = normalDepth.xyz;
		float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
		pz = 0.1f / (1.0f - pz);
		float3 p = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;
		float3 point_in = mul(float4(p,1.0f), model_from_view).xyz;
		float3 normal = mul(float4(n, 0.0f), model_from_view).xyz;
		// Compute the radiance reflected by the sphere.
		float3 sky_irradiance;
		float3 sun_irradiance = GetSunAndSkyIrradiance(point_in - earth_center, normal, sun_direction, sky_irradiance);
		//sphere_radiance = sun_irradiance;
		sphere_radiance = kSphereAlbedo * (1.0 / PI) * (sun_irradiance + sky_irradiance);
		//float shadow_length = max(0.0, min(shadow_out, distance_to_intersection) - shadow_in) * lightshaft_fadein_hack;
		float shadow_length = 0.0f;
		float3 transmittance;
		float3 in_scatter = GetSkyRadianceToPoint(camera - earth_center,point_in - earth_center, shadow_length, sun_direction, transmittance);
		sphere_radiance = sphere_radiance * transmittance + in_scatter;
	}
*/
	p = camera - earth_center;
	p_dot_v = dot(p, view_direction);
	p_dot_p = dot(p, p);
	float ray_earth_center_squared_distance = p_dot_p - p_dot_v * p_dot_v;
	distance_to_intersection = -p_dot_v - sqrt(earth_center.z * earth_center.z - ray_earth_center_squared_distance);

	// Compute the radiance reflected by the ground, if the ray intersects it.
	float ground_alpha = 0.0;
	float3 ground_radiance = float3(0.0f,0.0f,0.0f);
	if (distance_to_intersection > 0.0)
	{
		float3 point_in = camera + view_direction * distance_to_intersection;
		float3 normal = normalize(point_in - earth_center);

		// Compute the radiance reflected by the ground.
		float3 sky_irradiance;
		float3 sun_irradiance = GetSunAndSkyIrradiance(point_in - earth_center, normal, sun_direction, sky_irradiance);
		ground_radiance = kGroundAlbedo * (1.0 / PI) * (sun_irradiance * GetSunVisibility(point_in, sun_direction) + sky_irradiance * GetSkyVisibility(point_in));

		//float shadow_length = max(0.0, min(shadow_out, distance_to_intersection) - shadow_in) * lightshaft_fadein_hack;
		float shadow_length = 0.0f;
		float3 transmittance;
		float3 in_scatter = GetSkyRadianceToPoint(camera - earth_center,point_in - earth_center, shadow_length, sun_direction, transmittance);
		ground_radiance = ground_radiance * transmittance + in_scatter;
		ground_alpha = 1.0;
	}

	// Compute the radiance of the sky.
	float shadow_length = 0.0f;//max(0.0, shadow_out - shadow_in) * lightshaft_fadein_hack;
	float3 transmittance;
	float3 radiance = GetSkyRadiance(camera - earth_center, view_direction, shadow_length, sun_direction,transmittance);

	// If the view ray intersects the Sun, add the Sun radiance.
	if (dot(view_direction, sun_direction) > sun_size.y)
	{
		radiance = radiance + transmittance * GetSolarRadiance();
	}

	//radiance = radiance;
	radiance = lerp(radiance, ground_radiance, ground_alpha);
	radiance = lerp(radiance, sphere_radiance, sphere_alpha);
	float3 rgb_color = pow(float3(1.0f,1.0f,1.0f) - exp(-radiance / white_point_in * exposure), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
	//return float4(radiance, 1.0f);
	return float4(rgb_color, sphere_alpha);
}
technique11 draw_atmosphere
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_atmosphere()));
	}
}
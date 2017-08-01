//基本灯光结构
struct pancy_light_basic
{
	//光照强度
	float4    ambient;
	float4    diffuse;
	float4    specular;
	//光照位置，方向及衰减
	float3    dir;
	float     spot;
	//聚光灯属性
	float3    position;
	float     theta;
	//衰减及范围
	float3    decay;
	float     range;
	//光照类型
	uint4   type;
};
struct pancy_material
{
	float4   ambient;   //材质的环境光反射系数
	float4   diffuse;   //材质的漫反射系数
	float4   specular;  //材质的镜面反射系数
	float4   reflect;   //材质的反射系数
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~phong光照下非物理brdf~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void compute_dirlight(
	pancy_material mat,
	pancy_light_basic light_dir,
	float3 normal,
	float3 direction_view,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	ambient = mat.ambient * light_dir.ambient;         //环境光
	float3 light_dir_rec = normalize(light_dir.dir);
	float diffuse_angle = dot(-light_dir_rec, normal); //漫反射夹角
	if (diffuse_angle > 0.0f)
	{
		float3 v = normalize(reflect(light_dir_rec, normal));
		float spec_angle = pow(max(dot(v, direction_view), 0.0f), 10);

		diffuse = diffuse_angle * mat.diffuse * light_dir.diffuse;//漫反射光

		spec = spec_angle * mat.specular * light_dir.specular;    //镜面反射光
	}
	else
	{
		diffuse = 0.0f;//漫反射光
		spec = 0.0f;;    //镜面反射光
	}
}

void compute_pointlight(
	pancy_material mat,
	pancy_light_basic light_point,
	float3 pos,
	float3 normal,
	float3 position_view,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 lightVec = light_point.position - pos;
	float d = length(lightVec);
	ambient = mat.ambient * light_point.ambient;         //环境光
	float3 eye_direct = normalize(position_view - pos);
	if (d > light_point.range)
	{
		return;
	}
	//光照方向          
	lightVec = lightVec /= d;
	//漫反射夹角
	float diffuse_angle = dot(lightVec, normal);
	//直线衰减效果
	float4 distance_need;
	distance_need = float4(1.0f, d, d*d, 0.0f);
	float decay_final = 1.0 / dot(distance_need, float4(light_point.decay, 0.0f));
	//镜面反射
	if (diffuse_angle > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float spec_angle = pow(max(dot(v, eye_direct), 0.0f), mat.specular.w);
		diffuse = decay_final * diffuse_angle * mat.diffuse * light_point.diffuse;//漫反射光
		spec = decay_final * spec_angle * mat.specular * light_point.specular;    //镜面反射光
	}
}

void compute_spotlight(
	pancy_material mat,
	pancy_light_basic light_spot,
	float3 pos,
	float3 normal,
	float3 direction_view,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 lightVec = light_spot.position - pos;
	float d = length(lightVec);
	//光照方向
	lightVec /= d;
	light_spot.dir = normalize(light_spot.dir);
	ambient = mat.ambient * light_spot.ambient;//环境光
	float tmp = -dot(lightVec, light_spot.dir);//照射向量与光线向量的夹角
	if (tmp < cos(light_spot.theta))//聚光灯方向之外
	{
		return;
	}
	if (d > light_spot.range)//聚光灯范围之外
	{
		return;
	}
	//漫反射夹角
	float diffuse_angle = dot(lightVec, normal);
	//直线衰减效果
	float4 distance_need;
	distance_need = float4(1.0f, d, d*d, 0.0f);
	float decay_final = 1.0 / dot(distance_need, float4(light_spot.decay, 0.0f));
	//环形衰减效果
	float decay_spot = pow(tmp, light_spot.spot);
	//镜面反射
	if (diffuse_angle > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float spec_angle = pow(max(dot(v, direction_view), 0.0f), 10.0f);
		diffuse = decay_spot*decay_final * diffuse_angle * mat.diffuse * light_spot.diffuse;//漫反射光
		spec = decay_spot*decay_final * spec_angle * mat.specular * light_spot.specular;    //镜面反射光
	}
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~phong光照下微表面brdf~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//计算微表面brdf
void count_brdf_reflect(
	float3 specular_F0,
	float  tex_matallic,
	float  tex_roughness,
	float3 light_dir_in,
	float3 normal,
	float3 direction_view,
	float diffuse_angle,
	out float  diffuse_out,
	out float3 specular_out
	)
{
	float3 h_vec = (light_dir_in + direction_view) / 2.0f;
	diffuse_out = (1 - tex_matallic);
	float pi = 3.141592653;
	float view_angle = dot(direction_view, normal);//视线夹角
	float cos_vh = dot(direction_view, h_vec);
	//菲涅尔项
	float3 fresnel = specular_F0 + (float4(1.0f, 1.0f, 1.0f, 1.0f) - specular_F0)*(1.0f - pow(cos_vh, 5.0f));
	//NDF法线扰乱项
	float alpha = tex_roughness * tex_roughness;
	float nh_mul = dot(normal, h_vec);
	float divide_ndf1 = nh_mul*nh_mul * (alpha * alpha - 1.0f) + 1.0f;
	float divide_ndf2 = pi * divide_ndf1 *divide_ndf1;
	float ndf = (alpha*alpha) / divide_ndf2;
	//GGX遮挡项
	float ggx_k = (tex_roughness + 1.0f) * (tex_roughness + 1.0f) / 8.0f;
	float ggx_v = view_angle / (view_angle*(1 - ggx_k) + ggx_k);
	float ggx_l = diffuse_angle / (diffuse_angle*(1 - ggx_k) + ggx_k);
	float ggx = ggx_v * ggx_l;
	//最终的镜面反射项
	specular_out = (fresnel * ndf * ggx) / (4 * view_angle * diffuse_angle);
}
//方向光pbr
void compute_dirlight_pbr(
	float3 specular_F0,
	float  tex_matallic,
	float  tex_roughness,
	pancy_light_basic light_dir,
	float3 normal,
	float3 direction_view,
	out float4 diffuse,
	out float4 spec)
{
	float3 light_dir_rec = normalize(light_dir.dir);
	float diffuse_angle = dot(-light_dir_rec, normal); //漫反射夹角
	if (diffuse_angle > 0.0f)
	{
		//计算brdf
		float mat_diffuse;
		float3 mat_specular;
		float diffuse_angle = dot(-light_dir.dir, normal); //漫反射夹角
		count_brdf_reflect(specular_F0, tex_matallic, tex_roughness, -light_dir_rec, normal, direction_view, diffuse_angle, mat_diffuse, mat_specular);
		//计算反射光
		diffuse = diffuse_angle * mat_diffuse * light_dir.diffuse;//漫反射光
		spec = diffuse_angle * float4(mat_specular, 1.0f) * light_dir.specular;    //镜面反射光
	}
	else
	{
		diffuse = 0.0f;//漫反射光
		spec = 0.0f;;    //镜面反射光
	}
}
//点光源pbr
void compute_pointlight_pbr(
	float3 specular_F0,
	float  tex_matallic,
	float  tex_roughness,
	pancy_light_basic light_point,
	float3 pos,
	float3 normal,
	float3 position_view,
	out float4 diffuse,
	out float4 spec)
{
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 lightVec = light_point.position - pos;
	float d = length(lightVec);
	float3 eye_direct = normalize(position_view - pos);
	if (d > light_point.range)
	{
		return;
	}
	//光照方向          
	lightVec = lightVec /= d;
	//漫反射夹角
	float diffuse_angle = dot(lightVec, normal);
	//镜面反射
	if (diffuse_angle > 0.0f)
	{
		//直线衰减效果
		float4 distance_need;
		distance_need = float4(1.0f, d, d*d, 0.0f);
		float decay_final = 1.0 / dot(distance_need, float4(light_point.decay, 0.0f));
		//计算brdf
		float mat_diffuse;
		float3 mat_specular;
		count_brdf_reflect(specular_F0, tex_matallic, tex_roughness, lightVec, normal, eye_direct, diffuse_angle, mat_diffuse, mat_specular);
		//计算反射光
		diffuse = decay_final * diffuse_angle * mat_diffuse * light_point.diffuse;//漫反射光
		spec = decay_final * diffuse_angle * float4(mat_specular, 1.0f) * light_point.specular;    //镜面反射光
	}
}
//聚光灯pbr
void compute_spotlight_pbr(
	float3 specular_F0,
	float  tex_matallic,
	float  tex_roughness,
	pancy_light_basic light_spot,
	float3 pos,
	float3 normal,
	float3 direction_view,
	out float4 diffuse,
	out float4 spec)
{
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 lightVec = light_spot.position - pos;
	float d = length(lightVec);
	//光照方向
	lightVec /= d;
	light_spot.dir = normalize(light_spot.dir);
	float tmp = -dot(lightVec, light_spot.dir);//照射向量与光线向量的夹角
	if (tmp < cos(light_spot.theta))//聚光灯方向之外
	{
		return;
	}
	if (d > light_spot.range)//聚光灯范围之外
	{
		return;
	}
	//漫反射夹角
	float diffuse_angle = dot(lightVec, normal);
	//镜面反射
	if (diffuse_angle > 0.0f)
	{
		//直线衰减效果
		float4 distance_need;
		distance_need = float4(1.0f, d, d*d, 0.0f);
		float decay_final = 1.0 / dot(distance_need, float4(light_spot.decay, 0.0f));
		//环形衰减效果
		float decay_spot = pow(tmp, light_spot.spot);
		//计算brdf
		float mat_diffuse;
		float3 mat_specular;
		count_brdf_reflect(specular_F0, tex_matallic, tex_roughness, lightVec, normal, direction_view, diffuse_angle, mat_diffuse, mat_specular);
		//计算反射光
		diffuse = decay_spot * decay_final * diffuse_angle * mat_diffuse * light_spot.diffuse;//漫反射光
		spec = decay_spot * decay_final * diffuse_angle * float4(mat_specular, 1.0f) * light_spot.specular;    //镜面反射光
	}
}
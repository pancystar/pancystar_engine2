StructuredBuffer<float3> input_point;
uint4 offset_num;
uint4 offset_num_list[300];
struct Vertex_IN_anim
{
	uint   vertIndex: SV_VertexID;  //顶点索引号
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float3	tangent : TANGENT;      //顶点切向量
	uint4   texid   : TEXINDICES;   //纹理索引
	float4  tex1    : TEXDIFFNORM;  //顶点纹理坐标(漫反射及法线纹理)
	float4  tex2    : TEXOTHER;     //顶点纹理坐标(其它纹理)
};
struct Vertex_IN_anim_instance
{
	uint vertIndex	: SV_VertexID;  //顶点索引号
	uint InstanceId : SV_InstanceID;//instace索引号
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float3	tangent : TANGENT;      //顶点切向量
	uint4   texid   : TEXINDICES;   //纹理索引
	float4  tex1    : TEXDIFFNORM;  //顶点纹理坐标(漫反射及法线纹理)
	float4  tex2    : TEXOTHER;     //顶点纹理坐标(其它纹理)
};
float3 get_anim_point(Vertex_IN_anim vin)
{
	int offset_ID = vin.vertIndex + offset_num.x + offset_num.y * offset_num.z;
	float3 data = input_point[offset_ID];
	return data;
}
float3 get_anim_point(Vertex_IN_anim_instance vin)
{
	int offset_ID = vin.vertIndex + offset_num.x + offset_num.y * offset_num.z;
	float3 data = input_point[offset_ID];
	return data;
}
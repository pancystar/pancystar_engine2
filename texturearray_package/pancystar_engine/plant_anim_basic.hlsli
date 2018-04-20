StructuredBuffer<float3> input_point;
uint4 offset_num;
uint4 offset_num_list[300];
struct Vertex_IN_anim
{
	uint   vertIndex: SV_VertexID;  //����������
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float4  tex1    : TEXDIFFNORM;  //������������(�����估��������)
	float4  tex2    : TEXOTHER;     //������������(��������)
};
struct Vertex_IN_anim_instance
{
	uint vertIndex	: SV_VertexID;  //����������
	uint InstanceId : SV_InstanceID;//instace������
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float4  tex1    : TEXDIFFNORM;  //������������(�����估��������)
	float4  tex2    : TEXOTHER;     //������������(��������)
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
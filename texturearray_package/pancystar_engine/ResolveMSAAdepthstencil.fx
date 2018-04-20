Texture2DMS<float> gdepth_map;
float4 window_size;
float3 proj_desc;//ͶӰ���������ڻ�ԭ�����Ϣ
struct VertexIn//��ͨ����
{
	float3	pos 	: POSITION;     //����λ��
	float2  tex     : TEXCOORD;     //������������
};
struct VertexOut
{
	float4 PosH     : SV_POSITION; //��Ⱦ���߱�Ҫ����
	float2 tex      : TEXCOORD;     //������������
};
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	//�����ͶӰ����(���ڹ�դ��)
	vout.PosH = float4(vin.pos, 1.0f);
	//�������������(����msaa����)
	vout.tex = vin.tex;
	return vout;
}
//----------------------------------------------------------------------------------
float PS(VertexOut IN) : SV_TARGET
{
	float z = gdepth_map.Load(int2(IN.tex.x * window_size.x,IN.tex.y * window_size.y), 0);
	return 1.0f / (z * proj_desc.x + proj_desc.y);
}
technique11 resolove_msaa
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

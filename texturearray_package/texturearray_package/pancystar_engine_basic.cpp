#include"pancystar_engine_basic.h"
using namespace engine_basic;
//错误信息
engine_fail_reason::engine_fail_reason() 
{
	if_succeed = true;
	windows_result = S_OK;
	failed_reason = "";
}
engine_fail_reason::engine_fail_reason(std::string failed_reason) 
{
	set_failed_reason_common(failed_reason);
}
engine_fail_reason::engine_fail_reason(HRESULT windows_result_need, std::string failed_reason_need) 
{
	set_failed_reason_windows(windows_result_need, failed_reason_need);
}
void engine_fail_reason::set_failed_reason_common(std::string failed_reason_need)
{
	if_succeed = false;
	failed_reason = failed_reason_need;
}
void engine_fail_reason::set_failed_reason_windows(HRESULT windows_result_need, std::string failed_reason_need)
{
	if_succeed = false;
	windows_result = windows_result_need;
	failed_reason = failed_reason_need;
}
void engine_fail_reason::show_failed_reason() 
{
	size_t len = strlen(failed_reason.c_str()) + 1;
	size_t converted = 0;
	wchar_t *data_output;
	data_output = (wchar_t*)malloc(len*sizeof(wchar_t));
	mbstowcs_s(&converted, data_output, len, failed_reason.c_str(), _TRUNCATE);
	MessageBox(0, data_output,L"error",MB_OK);
}
//数学运算
DirectX::XMFLOAT3 engine_mathmatic::vec3_plus(DirectX::XMFLOAT3 vec1_in, DirectX::XMFLOAT3 vec2_in)
{
	DirectX::XMFLOAT3 vec_out;
	vec_out.x = vec1_in.x + vec2_in.x;
	vec_out.y = vec1_in.y + vec2_in.y;
	vec_out.z = vec1_in.z + vec2_in.z;
	return vec_out;
}
DirectX::XMFLOAT3 engine_mathmatic::vec3_minus(DirectX::XMFLOAT3 vec1_in, DirectX::XMFLOAT3 vec2_in)
{
	DirectX::XMFLOAT3 vec_out;
	vec_out.x = vec1_in.x - vec2_in.x;
	vec_out.y = vec1_in.y - vec2_in.y;
	vec_out.z = vec1_in.z - vec2_in.z;
	return vec_out;
}
DirectX::XMFLOAT3 engine_mathmatic::vec3_multi(DirectX::XMFLOAT3 vec_in, float scal_num)
{
	DirectX::XMFLOAT3 vec_out;
	vec_out.x = vec_in.x * scal_num;
	vec_out.y = vec_in.y * scal_num;
	vec_out.z = vec_in.z * scal_num;
	return vec_out;
}
DirectX::XMFLOAT3 engine_mathmatic::vec3_divide(DirectX::XMFLOAT3 vec_in, float scal_num)
{
	DirectX::XMFLOAT3 vec_out;
	vec_out.x = vec_in.x / scal_num;
	vec_out.y = vec_in.y / scal_num;
	vec_out.z = vec_in.z / scal_num;
	return vec_out;
}
DirectX::XMFLOAT3 engine_mathmatic::vec3_normalize(DirectX::XMFLOAT3 vec_in)
{
	float normalize_divide = sqrt(vec_in.x * vec_in.x + vec_in.y * vec_in.y + vec_in.z * vec_in.z);
	DirectX::XMFLOAT3 vec_out;
	vec_out.x = vec_in.x / normalize_divide;
	vec_out.y = vec_in.y / normalize_divide;
	vec_out.z = vec_in.z / normalize_divide;
	return vec_out;
}
DirectX::XMFLOAT3 engine_mathmatic::vec3_linear_inter(DirectX::XMFLOAT3 vec1_in, DirectX::XMFLOAT3 vec2_in, float distance)
{
	DirectX::XMFLOAT3 vec_out;
	vec_out.x = vec1_in.x + distance * (vec2_in.x - vec1_in.x);
	vec_out.y = vec1_in.y + distance * (vec2_in.y - vec1_in.y);
	vec_out.z = vec1_in.z + distance * (vec2_in.z - vec1_in.z);
	return vec_out;
}
DirectX::XMFLOAT3 engine_mathmatic::vec3_nilinear_inter(DirectX::XMFLOAT3 vec1_in, DirectX::XMFLOAT3 vec2_in, DirectX::XMFLOAT3 vec3_in, DirectX::XMFLOAT3 vec4_in, float distance_1, float distance_2)
{
	DirectX::XMFLOAT3 vec_mid1 = vec3_linear_inter(vec1_in, vec2_in, distance_1);
	DirectX::XMFLOAT3 vec_mid2 = vec3_linear_inter(vec3_in, vec4_in, distance_1);
	DirectX::XMFLOAT3 vec_out = vec3_linear_inter(vec_mid1, vec_mid2, distance_2);
	return vec_out;
}
//透视投影
perspective_message::perspective_message() 
{
	width_project = 800.0f;
	height_project = 600.0f;
	perspective_near_plane = 0.1f;
	perspective_far_plane = 1000.0f;
	perspective_angle = DirectX::XM_PIDIV4;
}
DirectX::XMFLOAT4X4 perspective_message::get_proj_matrix() 
{
	DirectX::XMMATRIX perspective_matrix = DirectX::XMMatrixPerspectiveFovLH(perspective_angle, width_project / height_project, perspective_near_plane, perspective_far_plane);
	DirectX::XMFLOAT4X4 mat_out;
	DirectX::XMStoreFloat4x4(&mat_out,perspective_matrix);
	return mat_out;
}
void perspective_message::reset_perpective_message(int wind_width_need, int wind_height_need, float near_plane, float far_plane, float angle) 
{
	width_project = static_cast<float>(wind_width_need);
	height_project = static_cast<float>(wind_height_need);
	perspective_near_plane = near_plane;
	perspective_far_plane = far_plane;
	perspective_angle = angle;
}
void perspective_message::update_windowsize(int wind_width_need, int wind_height_need)
{
	width_project = static_cast<float>(wind_width_need);
	height_project = static_cast<float>(wind_height_need);
}
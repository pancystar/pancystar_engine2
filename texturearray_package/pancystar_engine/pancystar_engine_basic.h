#pragma once
#include<windows.h>
#include<iostream>
#include<memory>
#include<D3D11.h>
#include<assert.h>
#include<vector>
#include<d3dx11effect.h>
#include<directxmath.h>
#include<list>
#include<ShellScalingAPI.h>
#include<DirectXTex.h>
#include<ScreenGrab.h>
#pragma comment ( lib, "D3D11.lib")
#pragma comment ( lib, "dxgi.lib")
#pragma comment ( lib, "Shcore.lib")
namespace engine_basic
{
	//观察者
	class window_size_observer 
	{
	public:
		virtual void update_windowsize(int wind_width_need, int wind_height_need) = 0;
	};
	//被观察对象
	class window_size_subject 
	{
	protected:
		std::list<window_size_observer*> observer_list;
	public:
		virtual void attach(window_size_observer*) = 0;
		virtual void detach(window_size_observer*) = 0;
		virtual void notify(int wind_width_need, int wind_height_need) = 0;
	};
	class engine_fail_reason
	{
		bool if_succeed;
		HRESULT windows_result;
		std::string failed_reason;
	public:
		engine_fail_reason();
		engine_fail_reason(std::string failed_reason);
		engine_fail_reason(HRESULT windows_result_need, std::string failed_reason_need);
		void show_failed_reason();
		//返回false代表出现错误，返回true表示成功
		bool check_if_failed() { return if_succeed; };
	private:
		void set_failed_reason_common(std::string failed_reason_need);
		void set_failed_reason_windows(HRESULT windows_result_need, std::string failed_reason_need);
	};
	class extra_perspective_message 
	{
		float width_project;
		float height_project;
		float perspective_near_plane;
		float perspective_far_plane;
		float perspective_angle;
		DirectX::XMFLOAT4 FrustumFarCorner[4];         //投影视截体的远截面的四个角点
		DirectX::XMFLOAT4 FrustumNearCorner[4];         //投影视截体的近截面的四个角点
	public:
		extra_perspective_message(int wind_width_need, int wind_height_need, float near_plane, float far_plane, float angle);
		void get_FrustumFarCorner(DirectX::XMFLOAT4 *FrustumFarCorner_out);
		void get_FrustumNearCorner(DirectX::XMFLOAT4 *FrustumNearCorner_out);
		DirectX::XMFLOAT4X4 get_proj_matrix();
		
		float get_perspective_near_plane() { return perspective_near_plane; };
		float get_perspective_far_plane() { return perspective_far_plane; };
		float get_perspective_angle() { return perspective_angle; };
	private:
		void BuildFrustumNearFarCorners(float fovy, float farZ);
	};
	class perspective_message : public window_size_observer
	{
		float width_project;
		float height_project;
		float perspective_near_plane;
		float perspective_far_plane;
		float perspective_angle;
		DirectX::XMFLOAT4 FrustumFarCorner[4];         //投影视截体的远截面的四个角点
		DirectX::XMFLOAT4 FrustumNearCorner[4];         //投影视截体的近截面的四个角点
	private:
		perspective_message();
	public:
		static perspective_message* get_instance()
		{
			static perspective_message* this_instance;
			if (this_instance == NULL) 
			{
				this_instance = new perspective_message();
			}
			return this_instance;
		}
		void get_FrustumFarCorner(DirectX::XMFLOAT4 *FrustumFarCorner_out);
		void get_FrustumNearCorner(DirectX::XMFLOAT4 *FrustumNearCorner_out);
		DirectX::XMFLOAT4X4 get_proj_matrix();
		DirectX::XMFLOAT4X4 get_proj_shadow_matrix();
		void reset_perpective_message(int wind_width_need,int wind_height_need,float near_plane,float far_plane,float angle);
		void BuildFrustumNearFarCorners(float fovy, float farZ);
		void update_windowsize(int wind_width_need, int wind_height_need);
		float get_perspective_near_plane() { return perspective_near_plane; };
		float get_perspective_far_plane() { return perspective_far_plane; };
		float get_perspective_angle() { return perspective_angle; };
	};
	class engine_mathmatic
	{
	public:
		static DirectX::XMFLOAT2 vec2_plus(DirectX::XMFLOAT2 vec1_in, DirectX::XMFLOAT2 vec2_in);
		static DirectX::XMFLOAT2 vec2_mul(DirectX::XMFLOAT2 vec1_in, DirectX::XMFLOAT2 vec2_in);
		static DirectX::XMFLOAT3 vec3_plus(DirectX::XMFLOAT3 vec1_in, DirectX::XMFLOAT3 vec2_in);
		static DirectX::XMFLOAT4 vec4_plus(DirectX::XMFLOAT4 vec1_in, DirectX::XMFLOAT4 vec2_in);
		static DirectX::XMFLOAT4 vec4mulmat(DirectX::XMFLOAT4 vec1_in, DirectX::XMFLOAT4X4 mat_in);
		static float vec4dot(DirectX::XMFLOAT4 vec1_in, DirectX::XMFLOAT4 vec2_in);
		static DirectX::XMFLOAT3 vec3_minus(DirectX::XMFLOAT3 vec1_in, DirectX::XMFLOAT3 vec2_in);
		static DirectX::XMFLOAT3 vec3_multi(DirectX::XMFLOAT3 vec_in, float scal_num);
		static DirectX::XMFLOAT3 vec3_divide(DirectX::XMFLOAT3 vec_in, float scal_num);
		static DirectX::XMFLOAT3 vec3_normalize(DirectX::XMFLOAT3 vec_in);
		static float vec3_length(DirectX::XMFLOAT3 vec_in);
		static DirectX::XMFLOAT3 vec3_linear_inter(DirectX::XMFLOAT3 vec1_in, DirectX::XMFLOAT3 vec2_in, float distance);
		static DirectX::XMFLOAT3 vec3_nilinear_inter(DirectX::XMFLOAT3 vec1_in, DirectX::XMFLOAT3 vec2_in, DirectX::XMFLOAT3 vec3_in, DirectX::XMFLOAT3 vec4_in, float distance_1, float distance_2);
	};
}
#pragma once
#include <windows.h>
#include<D3D11.h>
#include<assert.h>
#include<d3dx11effect.h>
#include<directxmath.h>
#include<string.h>
#include<stdlib.h>
#include<Dinput.h>
#include<iostream>
#include"pancystar_engine_basic.h"
#pragma comment(lib, "Dinput8.lib") 
using namespace std;
class pancy_input
{
	LPDIRECTINPUT8             pancy_dinput;                                 //DirectInput的设备接口    
	LPDIRECTINPUTDEVICE8       dinput_keyboard;                              //键盘设备接口
	LPDIRECTINPUTDEVICE8       dinput_mouse;                                 //鼠标设备接口
	char                       key_buffer[256];                              //键盘按键信息的缓存
	DIMOUSESTATE               mouse_buffer;                                 //鼠标控制信息的缓存
private:
	pancy_input();         //构造函数
public:
	static pancy_input *pancy_input_pInstance;
	static engine_basic::engine_fail_reason single_create(HWND hwnd, HINSTANCE hinst)
	{
		if (pancy_input_pInstance != NULL)
		{
			engine_basic::engine_fail_reason failed_reason("the d3d input instance have been created before");
			return failed_reason;
		}
		else
		{
			pancy_input_pInstance = new pancy_input();
			engine_basic::engine_fail_reason check_failed = pancy_input_pInstance->init(hwnd, hinst);
			return check_failed;
		}
	}
	static pancy_input * GetInstance()
	{
		return pancy_input_pInstance;
	}
	~pancy_input();                                                          //析构函数
	void  get_input();                                                       //获取外设输入
	bool  check_keyboard(int key_value);                                     //检测键盘上的某个键按下与否
	bool  check_mouseDown(int mouse_value);                                  //检测鼠标上的某个键按下与否
	float MouseMove_X();                                                     //获取鼠标在x轴的移动量
	float MouseMove_Y();                                                     //获取鼠标在y轴的移动量
	float MouseMove_Z();                                                     //获取鼠标在z轴的移动量
	engine_basic::engine_fail_reason init(HWND hwnd, HINSTANCE hinst);
private:
	engine_basic::engine_fail_reason dinput_clear(HWND hwnd,DWORD keyboardCoopFlags, DWORD mouseCoopFlags);//初始化函数
};
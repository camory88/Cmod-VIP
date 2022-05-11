#pragma once
#define WIN32_LEAN_AND_MEAN
#include "sdk.hpp"
#include "XorStr.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include <d3d9.h>
#include <dwmapi.h>
#include <Windows.h>
#include <random>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")

extern HWND overlayWindow;
extern IDirect3D9Ex* p_Object;
extern IDirect3DDevice9Ex* p_Device;
extern D3DPRESENT_PARAMETERS p_Params;


namespace overlay
{
	bool InitWindow();
	bool DirectXInit();
	void Render();
	void DirectXShutdown();
	//void UpdateWindowPos();
}

namespace cheats
{
	void NoRecoil();
	void Aimbot();
	void glow();
}
#include "overlay.h"
#include <string>
#include <filesystem>
#include <thread>
// set up these variables here in case we need them later
HWND hWnd = FindWindowA(NULL, "Apex Legends");
HWND overlayWindow;
IDirect3D9Ex* p_Object;
IDirect3DDevice9Ex* p_Device;
D3DPRESENT_PARAMETERS p_Params;
RECT rc;
MARGINS margin = { -1 };
ImFont* titleFont;
ImFont* bigFont;
MSG message;
IDirect3DTexture9* ksk;
IDirect3DTexture9* unknownCtrl;
float oldDelay = 0;
//globalVars globals;

ImColor on = ImColor(0, 255, 0, 255);
ImColor off = ImColor(255, 0, 0, 255);

//visal stuff
float vr = 200, vg = 20, vb = 20 , va = 255;

//players
int tr = 200, tg = 20, tb = 20, ta = 255;
int er = 200, eg = 20, eb = 20, ea = 255;

// winprochandler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message)
	{
	case WM_SIZE:
		if (p_Device != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_Params.BackBufferWidth = LOWORD(lParam);
			p_Params.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = p_Device->Reset(&p_Params);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;

	case WM_DESTROY:
		overlay::DirectXShutdown();
		PostQuitMessage(0);
		return 0;
		break;

	default:
		return DefWindowProcW(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void HandleMessages(std::wstring lClassName, std::wstring lWindowName)
{
	// create the window and set window attributes
	overlayWindow = CreateWindowExW(/*WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED*/NULL, lClassName.c_str(), lWindowName.c_str(), WS_POPUP | WS_VISIBLE, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 0, 0, 0, 0);
	SetWindowLongA(overlayWindow, GWL_EXSTYLE, (WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT));
	DwmExtendFrameIntoClientArea(overlayWindow, &margin);
	ShowWindow(overlayWindow, SW_SHOW);
	UpdateWindow(overlayWindow);

	while (true)
	{
		if (PeekMessageW(&message, overlayWindow, 0, 0, PM_REMOVE))
		{
			DispatchMessageW(&message);
			TranslateMessage(&message);
		}

		HWND hwnd_active = GetForegroundWindow();

		if (hwnd_active == hWnd)
		{
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(overlayWindow, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		Sleep(1);
	}
}

std::string RandomString(const int len)
{
	const std::string alpha_numeric("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890");

	std::default_random_engine generator{ std::random_device{}() };
	const std::uniform_int_distribution< std::string::size_type > distribution{ 0, alpha_numeric.size() - 1 };

	std::string str(len, 0);
	for (auto& it : str)
		it = alpha_numeric[distribution(generator)];

	return str;
}

bool overlay::InitWindow()
{
	// create a window class
	std::string r1 = RandomString(16);
	std::string r2 = RandomString(16);

	std::wstring lClassName = std::wstring(r1.begin(), r1.end());
	std::wstring lWindowName = std::wstring(r2.begin(), r2.end());

	WNDCLASSEXW wClass;
	wClass.cbSize = sizeof(WNDCLASSEXW);
	wClass.style = 0;
	wClass.lpfnWndProc = WinProc;
	wClass.cbClsExtra = NULL;
	wClass.cbWndExtra = NULL;
	wClass.hInstance = nullptr;
	wClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	wClass.hCursor = LoadCursor(0, IDC_ARROW);
	wClass.hbrBackground = nullptr;
	wClass.lpszMenuName = lWindowName.c_str();
	wClass.lpszClassName = lClassName.c_str();
	wClass.hIconSm = LoadIcon(0, IDI_APPLICATION);

	if (!RegisterClassExW(&wClass))
		return false;

	std::thread msgThread(HandleMessages, lClassName, lWindowName);
	msgThread.detach();

	return true;
}
POINT windowWH{ 1920,1080 };
POINT screenMid{ 1920 / 2, 1080 / 2 };
bool overlay::DirectXInit()
{
	// initialize directx
	while (!overlayWindow) {}

	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		return false;

	GetClientRect(hWnd, &rc);

	windowWH = { rc.right - rc.left, rc.bottom - rc.top };

	ZeroMemory(&p_Params, sizeof(p_Params));
	p_Params.Windowed = TRUE;
	p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_Params.hDeviceWindow = overlayWindow;
	p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_Params.BackBufferWidth = windowWH.x;
	p_Params.BackBufferHeight = windowWH.y;
	p_Params.EnableAutoDepthStencil = TRUE;
	p_Params.AutoDepthStencilFormat = D3DFMT_D16;
	p_Params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, overlayWindow, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device)))
		return false;



	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();

	// add default font and merge fontawesome icons
	//io.Fonts->AddFontFromMemoryTTF(comfortaaRegular, sizeof(comfortaaRegular), 13.f);

	// you can uncomment this if you want to load the icon font/bold font
	//static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	//ImFontConfig icons_config;
	//icons_config.MergeMode = true;
	//icons_config.PixelSnapH = true;
	//icons_config.GlyphOffset = ImVec2(0.f, 2.f);
	//io.Fonts->AddFontFromMemoryTTF(font_awesome_data, sizeof(font_awesome_data), 16.f, &icons_config, icons_ranges); // icons font

	//// add title font and merge fontawesome icons
	//ImFontConfig titleConfig;
	//titleConfig.GlyphOffset = ImVec2(0.f, -2.f);
	//titleFont = io.Fonts->AddFontFromMemoryTTF(comfortaaBold, sizeof(comfortaaBold), 19.f, &titleConfig); // bold font

	//io.Fonts->AddFontFromMemoryTTF(font_awesome_data, sizeof(font_awesome_data), 21.f, &icons_config, icons_ranges);

	//// add big font and merge fontawesome icons
	//bigFont = io.Fonts->AddFontFromMemoryTTF(comfortaaRegular, sizeof(comfortaaRegular), 16.f);

	//icons_config.GlyphOffset = ImVec2(0.f, 1.f);
	//io.Fonts->AddFontFromMemoryTTF(font_awesome_data, sizeof(font_awesome_data), 18.f, &icons_config, icons_ranges);

	ImGui_ImplWin32_Init(overlayWindow);
	ImGui_ImplDX9_Init(p_Device);

	return true;
}

void InputHandler()
{
	if ((GetKeyState(VK_INSERT) & 0x8000))
	{
		if (globals.menuActive == false)
			globals.menuActive = true;
		else if (globals.menuActive == true)
			globals.menuActive = false;
		Sleep(100);
	}
}

void DrawImGui()
{
	// pass input to imgui
	InputHandler();

	// create an imgui frame

	// set the right window flags
	if (globals.menuActive)
		SetWindowLongA(overlayWindow, GWL_EXSTYLE, (WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT));
	else
		SetWindowLongA(overlayWindow, GWL_EXSTYLE, (WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT));

	if (globals.menuActive)
	{
		// imgui stuff

		ImGui::SetNextWindowSize(ImVec2(600, 700));
		ImGui::Begin("menu", (bool*)0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
		static int tabb = 0;
		//top bar
		static int switchTabs = 3;

		if (ImGui::Button(xorstr_("Visuals"), ImVec2(100.0f, 0.0f)))
			switchTabs = 0;
		ImGui::SameLine(0.0, 2.0f);
		if (ImGui::Button(xorstr_("Aimbot"), ImVec2(100.0f, 0.0f)))
			switchTabs = 1;
		ImGui::SameLine(0.0, 2.0f);
		if (ImGui::Button(xorstr_("Misc"), ImVec2(100.0f, 0.0f)))
			switchTabs = 2;
		ImGui::SameLine(0.0, 2.0f);
		if (ImGui::Button(xorstr_("Rage"), ImVec2(100.0f, 0.0f)))
			switchTabs = 3;

		switch (switchTabs) {
		case 0:
			//Do stuff for Aim tab, next for vis, and last for misc tab
			ImGui::Checkbox(xorstr_("ESP"), &globals.esp);
			ImGui::Spacing();
			ImGui::Checkbox(xorstr_("Enemies"), &globals.enemy); ImGui::SameLine(100.0, 2.0f); ImGui::Checkbox(xorstr_("Team"), &globals.team); ImGui::SameLine(200.0, 2.0f); ImGui::Checkbox(xorstr_("NPC"), &globals.NPCesp);
			ImGui::Checkbox(xorstr_("Skelliton"), &globals.skeliton); ImGui::SameLine(100.0, 2.0f); ImGui::Checkbox(xorstr_("Box"), &globals.skeliton); ImGui::SameLine(200.0, 2.0f); ImGui::Checkbox(xorstr_("Line"), &globals.skeliton);
			ImGui::SliderFloat(xorstr_("esp Distance"), &globals.espDist, 10, 9000, NULL, NULL);
			break;
		case 1:
			ImGui::Checkbox(xorstr_("AimBot"), &globals.aimbot);
			ImGui::Checkbox(xorstr_("Draw FOV"), &globals.drawFOV);
			//ImGui::ColorEdit3(xorstr_("FOV color"), );
			ImGui::SliderFloat(xorstr_("Aimbot rsc"), &globals.rcsaimbot, 0, 100.f, "%.1f%%", NULL);
			ImGui::SliderFloat(xorstr_("Aimbot FOV"), &globals.aimbotFOV, 10, 2000.f, "%.1f", NULL);
			ImGui::Checkbox(xorstr_("Rainbow Fov"), &globals.RainBow);
			break;
		case 2:
			ImGui::Checkbox("NoRecoil", &globals.noRecoil);
			ImGui::SliderFloat(xorstr_("rsc"), &globals.rcs, 0, 100, NULL, NULL);
			break;
		case 3:
			break;
		}

		ImGui::Spacing();
		ImGui::End();
	}
	if (globals.noRecoil == true)
		cheats::NoRecoil();
	if (globals.aimbot == true)
		cheats::Aimbot();
		
	if (globals.esp == true)
		cheats::glow();

}

auto Rainbow(float delay)
{
	static uint32_t cnt = 0;
	float freq = delay;

	if (++cnt >= (uint32_t)-1)
	{
		cnt = 0;
	}
	if (delay != oldDelay)
	{
		cnt = 0;
		oldDelay = delay;
	}

	return std::make_tuple(std::sin(freq * cnt + 0) * 2.f, std::sin(freq * cnt + 2) * 2.3f, std::sin(freq * cnt + 4) * 2.6f);
}
void DrawFilledRectangle(int x, int y, int w, int h, D3DCOLOR color)
{
	D3DRECT rect = { x, y, x + w, y + h };
	p_Device->Clear(1, &rect, D3DCLEAR_TARGET, color, 0, 0);
}


void DrawBorderBox(int x, int y, int x2, int y2, int thickness, D3DCOLOR color)
{
	DrawFilledRectangle(x, y, x2, thickness, color); // top
	DrawFilledRectangle(x, y + y2, x2, thickness, color); // bottom
	DrawFilledRectangle(x, y, thickness, y2, color); // left
	DrawFilledRectangle(x + x2, y, thickness, y2 + thickness, color); // right
}

void overlay::Render()
{
	static ImDrawList* drawList;

	while (!GetAsyncKeyState(VK_END))
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		DrawImGui();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::Begin("##scene", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);
		ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);

		drawList = ImGui::GetWindowDrawList();

		p_Device->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.f, 0);
		p_Device->BeginScene();
		
		//DrawBorderBox(100, 100, 200, 200, 2, ImColor(1, 200, 1));
		auto draw = ImGui::GetBackgroundDrawList();
		if (globals.drawFOV == true)
			draw->AddCircle(ImVec2(screenMid.x, screenMid.y), globals.aimbotFOV, ImColor(vr,vg,vb,va), 100, 1.0f);
		//draw->AddLine(ImVec2(screenMid.x, screenMid.y), ImVec2(100, 1000), ImColor(255,20,20,255), 1.0f);

		if (globals.aimbot == false)
			draw->AddText(ImVec2(5, 5), off, xorstr_("AimBot\t"), NULL);
		else
			draw->AddText(ImVec2(5, 5), on, xorstr_("AimBot\t"), NULL);

		if (globals.noRecoil == false)
			draw->AddText(ImVec2(5, 25), off, xorstr_("NoRecoil\t"), NULL);
		else
			draw->AddText(ImVec2(5, 25), on, xorstr_("NoRecoil\t"), NULL);

		if (globals.esp == false)
			draw->AddText(ImVec2(5, 45), off, xorstr_("ESP\t"), NULL);
		else
			draw->AddText(ImVec2(5, 45), on, xorstr_("ESP\t"), NULL);

		bool crossHair = true;
		if (crossHair == true)
		{
			draw->AddLine(ImVec2(screenMid.x - 30 , screenMid.y ), ImVec2(screenMid.x + 30, screenMid.y), ImColor(255, 255, 255, 100));
			draw->AddLine(ImVec2(screenMid.x , screenMid.y - 30), ImVec2(screenMid.x , screenMid.y + 30), ImColor(255, 255, 255, 100));
			draw->AddLine(ImVec2(screenMid.x - 30, screenMid.y - 30), ImVec2(screenMid.x - 30, screenMid.y ), ImColor(255, 255, 255, 100));
			draw->AddLine(ImVec2(screenMid.x - 30, screenMid.y + 30), ImVec2(screenMid.x, screenMid.y + 30), ImColor(255, 255, 255, 100));
			draw->AddLine(ImVec2(screenMid.x + 30, screenMid.y + 30), ImVec2(screenMid.x + 30, screenMid.y), ImColor(255, 255, 255, 100));
			draw->AddLine(ImVec2(screenMid.x + 30, screenMid.y - 30), ImVec2(screenMid.x, screenMid.y - 30), ImColor(255, 255, 255, 100));
		}

		drawList->PushClipRectFullScreen();
		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		p_Device->EndScene();
		p_Device->PresentEx(0, 0, 0, 0, 0);
	}
}

void overlay::DirectXShutdown()
{
	// destroy everything
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();

	p_Object->Release();
	p_Device->Release();

	DestroyWindow(overlayWindow);
}
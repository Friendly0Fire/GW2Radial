#include "main.h"
#include <tchar.h>
#include <imgui.h>
#include <examples\directx9_example\imgui_impl_dx9.h>
#include <sstream>
#include "UnitQuad.h"
#include <d3dx9.h>
#include "Config.h"
#include "Utility.h"
#include <functional>
#include "minhook/include/MinHook.h"
#include <Shlwapi.h>
#include <d3d9.h>
#include "inputs.h"

void PreReset();

const float BaseSpriteSize = 0.4f;
const float CircleRadiusScreen = 256.f / 1664.f * BaseSpriteSize * 0.5f;

Config Cfg;
HWND GameWindow = 0;

// Active state
bool DisplayMountOverlay = false;
bool DisplayOptionsWindow = false;

struct KeybindSettingsMenu
{
	char DisplayString[256];
	bool Setting = false;
	std::function<void(const std::set<uint>&)> SetCallback;

	void SetDisplayString(const std::set<uint>& keys)
	{
		std::string keybind = "";
		for (const auto& k : keys)
		{
			keybind += GetKeyName(k) + std::string(" + ");
		}

		strcpy_s(DisplayString, (keybind.size() > 0 ? keybind.substr(0, keybind.size() - 3) : keybind).c_str());
	}

	void CheckSetKeybind(const std::set<uint>& keys, bool apply)
	{
		if (Setting)
		{
			SetDisplayString(keys);
			if (apply)
			{
				Setting = false;
				SetCallback(keys);
			}
		}
	}
};
KeybindSettingsMenu MainKeybind;
KeybindSettingsMenu MainLockedKeybind;
KeybindSettingsMenu MountKeybinds[5];

D3DXVECTOR2 OverlayPosition;
mstime OverlayTime, MountHoverTime;

enum CurrentMountHovered_t
{
	CMH_NONE = -1,
	CMH_RAPTOR = 0,
	CMH_SPRINGER = 1,
	CMH_SKIMMER = 2,
	CMH_JACKAL = 3,
	CMH_GRIFFON = 4
};
CurrentMountHovered_t CurrentMountHovered = CMH_NONE;

ImVec4 operator/(const ImVec4& v, float f)
{
	return ImVec4(v.x / f, v.y / f, v.z / f, v.w / f);
}

void ImGuiKeybindInput(const std::string& name, KeybindSettingsMenu& setting)
{
	std::string suffix = "##" + name;

	float windowWidth = ImGui::GetWindowWidth();

	ImGui::PushItemWidth(windowWidth * 0.3f);

	int popcount = 1;
	if (setting.Setting)
	{
		popcount = 3;
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(201, 215, 255, 200) / 255.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0, 0, 0, 1));
	}
	else
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1));

	ImGui::InputText(suffix.c_str(), setting.DisplayString, 256, ImGuiInputTextFlags_ReadOnly);

	ImGui::PopItemWidth();

	ImGui::PopStyleColor(popcount);

	ImGui::SameLine();

	if (!setting.Setting && ImGui::Button(("Set" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)))
	{
		setting.Setting = true;
		setting.DisplayString[0] = '\0';
	}
	else if (setting.Setting && ImGui::Button(("Clear" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)))
	{
		setting.Setting = false;
		setting.DisplayString[0] = '\0';
		setting.SetCallback(std::set<uint>());
	}

	ImGui::SameLine();

	ImGui::PushItemWidth(windowWidth * 0.5f);

	ImGui::Text(name.c_str());

	ImGui::PopItemWidth();
}

const char* GetMountName(CurrentMountHovered_t m)
{
	switch (m)
	{
	case CMH_RAPTOR:
		return "Raptor";
	case CMH_SPRINGER:
		return "Springer";
	case CMH_SKIMMER:
		return "Skimmer";
	case CMH_JACKAL:
		return "Jackal";
	case CMH_GRIFFON:
		return "Griffon";
	default:
		return "[Unknown]";
	}
}



WNDPROC BaseWndProc;
HMODULE DllModule = nullptr;

// Rendering
uint ScreenWidth, ScreenHeight;
std::unique_ptr<UnitQuad> Quad;
ID3DXEffect* MainEffect = nullptr;
IDirect3DTexture9* MountsTexture = nullptr;
IDirect3DTexture9* MountTextures[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
IDirect3DTexture9* BgTexture = nullptr;

void LoadMountTextures(IDirect3DDevice9* dev)
{
	D3DXCreateTextureFromResource(dev, DllModule, MAKEINTRESOURCE(IDR_BG), &BgTexture);
	D3DXCreateTextureFromResource(dev, DllModule, MAKEINTRESOURCE(IDR_MOUNTS), &MountsTexture);
	for (uint i = 0; i < 6; i++)
		D3DXCreateTextureFromResource(dev, DllModule, MAKEINTRESOURCE(IDR_MOUNT1 + i), &MountTextures[i]);
}

void UnloadMountTextures()
{
	COM_RELEASE(MountsTexture);
	COM_RELEASE(BgTexture);

	for (uint i = 0; i < 6; i++)
		COM_RELEASE(MountTextures[i]);
}

typedef DWORD(WINAPI *GetModuleFileNameW_t)(
	_In_opt_ HMODULE hModule,
	_Out_    LPTSTR  lpFilename,
	_In_     DWORD   nSize
	);
GetModuleFileNameW_t real_GetModuleFileNameW = nullptr;

DWORD WINAPI hook_GetModuleFileNameW(
	_In_opt_ HMODULE hModule,
	_Out_    LPTSTR  lpFilename,
	_In_     DWORD   nSize
)
{
	if(hModule == DllModule || hModule == nullptr)
		return real_GetModuleFileNameW(hModule, lpFilename, nSize);

	wchar_t cpath[MAX_PATH];
	DWORD r = real_GetModuleFileNameW(hModule, cpath, MAX_PATH);
	if (r == ERROR_INSUFFICIENT_BUFFER)
		return r;

	if (StrStrIW(cpath, L"bin64") == NULL)
		return r;

	const wchar_t* query = L"d3d9_mchain.dll";
	const wchar_t* replacement = L"d3d9.dll";

	std::wstring path = cpath;

	size_t index = path.find(query, 0);
	if (index != std::string::npos)
		path.replace(index, wcslen(query), replacement);

	wcscpy_s(lpFilename, nSize, path.c_str());

	return min(r, (DWORD)path.length());
}

void Shutdown()
{
	PreReset();
	ImGui_ImplDX9_Shutdown();
}

bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		DllModule = hModule;

		// Add an extra reference count to the library so it persists through GW2's load-unload routine
		// without which problems start arising with ReShade
		{
			TCHAR selfpath[MAX_PATH];
			GetModuleFileName(DllModule, selfpath, MAX_PATH);
			LoadLibrary(selfpath);
		}

		MH_Initialize();

		MH_CreateHook(&GetModuleFileNameW, hook_GetModuleFileNameW, reinterpret_cast<LPVOID*>(&real_GetModuleFileNameW));
		MH_EnableHook(MH_ALL_HOOKS);

		Cfg.Load();

		MainKeybind.SetDisplayString(Cfg.MountOverlayKeybind());
		MainKeybind.SetCallback = [](const std::set<uint>& val) { Cfg.MountOverlayKeybind(val); };
		MainLockedKeybind.SetDisplayString(Cfg.MountOverlayLockedKeybind());
		MainLockedKeybind.SetCallback = [](const std::set<uint>& val) { Cfg.MountOverlayLockedKeybind(val); };
		for (uint i = 0; i < 5; i++)
		{
			MountKeybinds[i].SetDisplayString(Cfg.MountKeybind(i));
			MountKeybinds[i].SetCallback = [i](const std::set<uint>& val) { Cfg.MountKeybind(i, val); };
		}

		break;
	}
	case DLL_PROCESS_DETACH:
		// We'll just leak a bunch of things and let the driver/OS take care of it, since we have no clean exit point
		// and calling FreeLibrary in DllMain causes deadlocks
		break;
	}

	return true;
}

void DetermineHoveredMount()
{
	const auto io = ImGui::GetIO();

	D3DXVECTOR2 MousePos;
	MousePos.x = io.MousePos.x / (float)ScreenWidth;
	MousePos.y = io.MousePos.y / (float)ScreenHeight;
	MousePos -= OverlayPosition;

	CurrentMountHovered_t LastMountHovered = CurrentMountHovered;

	if (D3DXVec2LengthSq(&MousePos) > CircleRadiusScreen * CircleRadiusScreen)
	{
		if (MousePos.x < 0 && abs(MousePos.x) > abs(MousePos.y)) // Raptor, 0
			CurrentMountHovered = CMH_RAPTOR;
		else if (MousePos.x > 0 && abs(MousePos.x) > abs(MousePos.y)) // Jackal, 3
			CurrentMountHovered = CMH_JACKAL;
		else if (MousePos.y < 0 && abs(MousePos.x) < abs(MousePos.y)) // Springer, 1
			CurrentMountHovered = CMH_SPRINGER;
		else if (MousePos.y > 0 && abs(MousePos.x) < abs(MousePos.y)) // Skimmer, 2
			CurrentMountHovered = CMH_SKIMMER;
		else
			CurrentMountHovered = CMH_NONE;
	}
	else
		CurrentMountHovered = CMH_GRIFFON;

	if (LastMountHovered != CurrentMountHovered)
		MountHoverTime = timeInMS();
}

void Shutdown();

extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct EventKey
	{
		uint vk : 31;
		bool down : 1;
	};

	std::list<EventKey> eventKeys;

	// Generate our EventKey list for the current message
	{
		bool eventDown = false;
		switch (msg)
		{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			eventDown = true;
		case WM_SYSKEYUP:
		case WM_KEYUP:
			if (msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP)
			{
				if (((lParam >> 29) & 1) == 1)
					eventKeys.push_back({ VK_MENU, true });
				else
					eventKeys.push_back({ VK_MENU, false });
			}

			eventKeys.push_back({ (uint)wParam, eventDown });
			break;

		case WM_LBUTTONDOWN:
			eventDown = true;
		case WM_LBUTTONUP:
			eventKeys.push_back({ VK_LBUTTON, eventDown });
			break;
		case WM_MBUTTONDOWN:
			eventDown = true;
		case WM_MBUTTONUP:
			eventKeys.push_back({ VK_MBUTTON, eventDown });
			break;
		case WM_RBUTTONDOWN:
			eventDown = true;
		case WM_RBUTTONUP:
			eventKeys.push_back({ VK_RBUTTON, eventDown });
			break;
		case WM_XBUTTONDOWN:
			eventDown = true;
		case WM_XBUTTONUP:
			eventKeys.push_back({ (uint)(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2), eventDown });
			break;
		}
	}

	// Apply key events now
	for (const auto& k : eventKeys)
		if (k.down)
			DownKeys.insert(k.vk);
		else
			DownKeys.erase(k.vk);

	// Detect hovered section of the radial menu, if visible
	if (DisplayMountOverlay && msg == WM_MOUSEMOVE)
		DetermineHoveredMount();

	bool isMenuKeybind = false;

	// Only run these for key down/key up (incl. mouse buttons) events
	if (!eventKeys.empty())
	{
		// Very exclusive test: *only* consider the menu keybind to be activated if they're the *only* keys currently down
		// This minimizes the likelihood of the menu randomly popping up when it shouldn't
		isMenuKeybind = DownKeys == Cfg.SettingsKeybind();

		if(isMenuKeybind)
			DisplayOptionsWindow = true;
		else
		{
			bool oldMountOverlay = DisplayMountOverlay;

			bool mountOverlay = !Cfg.MountOverlayKeybind().empty() && std::includes(DownKeys.begin(), DownKeys.end(), Cfg.MountOverlayKeybind().begin(), Cfg.MountOverlayKeybind().end());
			bool mountOverlayLocked = !Cfg.MountOverlayLockedKeybind().empty() && std::includes(DownKeys.begin(), DownKeys.end(), Cfg.MountOverlayLockedKeybind().begin(), Cfg.MountOverlayLockedKeybind().end());

			DisplayMountOverlay = mountOverlayLocked || mountOverlay;

			if (DisplayMountOverlay && !oldMountOverlay)
			{
				// Mount overlay is turned on

				if (mountOverlayLocked)
				{
					OverlayPosition.x = OverlayPosition.y = 0.5f;

					// Attempt to move the cursor to the middle of the screen
					if (Cfg.ResetCursorOnLockedKeybind())
					{
						RECT rect = { 0 };
						if (GetWindowRect(GameWindow, &rect))
						{
							if (SetCursorPos((rect.right - rect.left) / 2 + rect.left, (rect.bottom - rect.top) / 2 + rect.top))
							{
								auto& io = ImGui::GetIO();
								io.MousePos.x = ScreenWidth * 0.5f;
								io.MousePos.y = ScreenHeight * 0.5f;
							}
						}
					}
				}
				else
				{
					const auto& io = ImGui::GetIO();
					OverlayPosition.x = io.MousePos.x / (float)ScreenWidth;
					OverlayPosition.y = io.MousePos.y / (float)ScreenHeight;
				}

				OverlayTime = timeInMS();

				DetermineHoveredMount();
			}
			else if (!DisplayMountOverlay && oldMountOverlay)
			{
				// Mount overlay is turned off, send the keybind
				if (CurrentMountHovered != CMH_NONE)
					SendKeybind(Cfg.MountKeybind((uint)CurrentMountHovered));

				CurrentMountHovered = CMH_NONE;
			}

			{
				// If a key was lifted, we consider the key combination *prior* to this key being lifted as the keybind
				bool keyLifted = false;
				auto fullKeybind = DownKeys;
				for (const auto& ek : eventKeys)
				{
					if (!ek.down)
					{
						fullKeybind.insert(ek.vk);
						keyLifted = true;
					}
				}

				// Explicitly filter out M1 (left mouse button) from keybinds since it breaks too many things
				fullKeybind.erase(VK_LBUTTON);

				MainKeybind.CheckSetKeybind(fullKeybind, keyLifted);
				MainLockedKeybind.CheckSetKeybind(fullKeybind, keyLifted);

				for (uint i = 0; i < 5; i++)
					MountKeybinds[i].CheckSetKeybind(fullKeybind, keyLifted);
			}
		}
	}

#if 0
	if (input_key_down || input_key_up)
	{
		std::string keybind = "";
		for (const auto& k : DownKeys)
		{
			keybind += GetKeyName(k) + std::string(" + ");
		}
		keybind = keybind.substr(0, keybind.size() - 2) + "\n";

		OutputDebugStringA(("Current keys down: " + keybind).c_str());

		char buf[1024];
		sprintf_s(buf, "msg=%u wParam=%u lParam=%u\n", msg, (uint)wParam, (uint)lParam);
		OutputDebugStringA(buf);
	}
#endif

	ImGui_ImplDX9_WndProcHandler(hWnd, msg, wParam, lParam);

	// Prevent game from receiving the settings menu keybind
	if (!eventKeys.empty() && isMenuKeybind)
		return true;

	// Prevent game cursor/camera from moving when the overlay is displayed
	if (DisplayMountOverlay && Cfg.LockCameraWhenOverlayed())
	{
		switch (msg)
		{
		case WM_MOUSEMOVE:
			return true;
		case WM_INPUT:
		{
			UINT dwSize = 40;
			static BYTE lpb[40];

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
				lpb, &dwSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE)
				return true;

			break;
		}
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDBLCLK:
		{
			const auto& io2 = ImGui::GetIO();

			short mx, my;
			mx = (short)io2.MousePos.x;
			my = (short)io2.MousePos.y;
			lParam = MAKELPARAM(mx, my);
			break;
		}
		}
	}

	// Prevent game from receiving input if ImGui requests capture
	const auto& io = ImGui::GetIO();
	switch (msg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
		if (io.WantCaptureMouse)
			return true;
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		if (io.WantCaptureKeyboard)
			return true;
		break;
	case WM_CHAR:
		if (io.WantTextInput)
			return true;
		break;
	}

	// Whatever's left should be sent to the game
	return CallWindowProc(BaseWndProc, hWnd, msg, wParam, lParam);
}

void PreCreateDevice(HWND hFocusWindow)
{
	GameWindow = hFocusWindow;

	// Hook WndProc
	if (!BaseWndProc)
	{
		BaseWndProc = (WNDPROC)GetWindowLongPtr(hFocusWindow, GWLP_WNDPROC);
		SetWindowLongPtr(hFocusWindow, GWLP_WNDPROC, (LONG_PTR)&WndProc);
	}
}

void PostCreateDevice(IDirect3DDevice9* temp_device, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	// Init ImGui
	auto& imio = ImGui::GetIO();
	imio.IniFilename = Cfg.ImGuiConfigLocation();

	// Setup ImGui binding
	ImGui_ImplDX9_Init(GameWindow, temp_device);

	// Initialize graphics
	ScreenWidth = pPresentationParameters->BackBufferWidth;
	ScreenHeight = pPresentationParameters->BackBufferHeight;
	try
	{
		Quad = std::make_unique<UnitQuad>(temp_device);
	}
	catch (...)
	{
		Quad = nullptr;
	}
	ID3DXBuffer* errorBuffer = nullptr;
	D3DXCreateEffectFromResource(temp_device, DllModule, MAKEINTRESOURCE(IDR_SHADER), nullptr, nullptr, 0, nullptr, &MainEffect, &errorBuffer);
	COM_RELEASE(errorBuffer);
	LoadMountTextures(temp_device);
}

void PreReset()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	Quad.reset();
	UnloadMountTextures();
	COM_RELEASE(MainEffect);
}

void PostReset(IDirect3DDevice9* dev, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	ScreenWidth = pPresentationParameters->BackBufferWidth;
	ScreenHeight = pPresentationParameters->BackBufferHeight;

	ImGui_ImplDX9_CreateDeviceObjects();
	ID3DXBuffer* errorBuffer = nullptr;
	D3DXCreateEffectFromResource(dev, DllModule, MAKEINTRESOURCE(IDR_SHADER), nullptr, nullptr, 0, nullptr, &MainEffect, &errorBuffer);
	COM_RELEASE(errorBuffer);
	LoadMountTextures(dev);
	try
	{
		Quad = std::make_unique<UnitQuad>(dev);
	}
	catch (...)
	{
		Quad = nullptr;
	}
}

void Draw(IDirect3DDevice9* dev)
{
	// This is the closest we have to a reliable "update" function, so use it as one
	SendQueuedInputs();

	// We have to use Present rather than hooking EndScene because the game seems to do final UI compositing after EndScene
	// This unfortunately means that we have to call Begin/EndScene before Present so we can render things, but thankfully for modern GPUs that doesn't cause bugs
	dev->BeginScene();

	ImGui_ImplDX9_NewFrame();

	if (DisplayOptionsWindow)
	{
		ImGui::Begin("Mounts Options Menu", &DisplayOptionsWindow);

		ImGuiKeybindInput("Overlay Keybind", MainKeybind);
		ImGuiKeybindInput("Overlay Keybind (Center Locked)", MainLockedKeybind);

		if (Cfg.ResetCursorOnLockedKeybind() != ImGui::Checkbox("Reset cursor to center with Center Locked keybind", &Cfg.ResetCursorOnLockedKeybind()))
			Cfg.ResetCursorOnLockedKeybindSave();
		if (Cfg.LockCameraWhenOverlayed() != ImGui::Checkbox("Lock camera when overlay is displayed", &Cfg.LockCameraWhenOverlayed()))
			Cfg.LockCameraWhenOverlayedSave();

		ImGui::Separator();
		ImGui::Text("Mount Keybinds");
		ImGui::Text("(set to relevant game keybinds)");

		for (uint i = 0; i < 5u; i++)
			ImGuiKeybindInput(GetMountName((CurrentMountHovered_t)i), MountKeybinds[i]);

		ImGui::End();
	}

	ImGui::Render();

	if (DisplayMountOverlay && MainEffect && Quad)
	{
		auto currentTime = timeInMS();

		uint passes = 0;

		Quad->Bind();

		// Setup viewport
		D3DVIEWPORT9 vp;
		vp.X = vp.Y = 0;
		vp.Width = (DWORD)ScreenWidth;
		vp.Height = (DWORD)ScreenHeight;
		vp.MinZ = 0.0f;
		vp.MaxZ = 1.0f;
		dev->SetViewport(&vp);

		D3DXVECTOR4 screenSize((float)ScreenWidth, (float)ScreenHeight, 1.f / ScreenWidth, 1.f / ScreenHeight);

		D3DXVECTOR4 baseSpriteDimensions;
		baseSpriteDimensions.x = OverlayPosition.x;
		baseSpriteDimensions.y = OverlayPosition.y;
		baseSpriteDimensions.z = BaseSpriteSize * screenSize.y * screenSize.z;
		baseSpriteDimensions.w = BaseSpriteSize;

		D3DXVECTOR4 overlaySpriteDimensions = baseSpriteDimensions;
		D3DXVECTOR4 direction;

		if (CurrentMountHovered != CMH_NONE)
		{
			if (CurrentMountHovered == CMH_RAPTOR)
			{
				overlaySpriteDimensions.x -= 0.5f * BaseSpriteSize * 0.5f * screenSize.y * screenSize.z;
				overlaySpriteDimensions.z *= 0.5f;
				overlaySpriteDimensions.w = BaseSpriteSize * 1024.f / 1664.f;
				direction = D3DXVECTOR4(-1.f, 0, 0.f, 0.f);
			}
			else if (CurrentMountHovered == CMH_JACKAL)
			{
				overlaySpriteDimensions.x += 0.5f * BaseSpriteSize * 0.5f * screenSize.y * screenSize.z;
				overlaySpriteDimensions.z *= 0.5f;
				overlaySpriteDimensions.w = BaseSpriteSize * 1024.f / 1664.f;
				direction = D3DXVECTOR4(1.f, 0, 0.f, 0.f);
			}
			else if (CurrentMountHovered == CMH_SPRINGER)
			{
				overlaySpriteDimensions.y -= 0.5f * BaseSpriteSize * 0.5f;
				overlaySpriteDimensions.w *= 0.5f;
				overlaySpriteDimensions.z = BaseSpriteSize * 1024.f / 1664.f * screenSize.y * screenSize.z;
				direction = D3DXVECTOR4(0, -1.f, 0.f, 0.f);
			}
			else if (CurrentMountHovered == CMH_SKIMMER)
			{
				overlaySpriteDimensions.y += 0.5f * BaseSpriteSize * 0.5f;
				overlaySpriteDimensions.w *= 0.5f;
				overlaySpriteDimensions.z = BaseSpriteSize * 1024.f / 1664.f * screenSize.y * screenSize.z;
				direction = D3DXVECTOR4(0, 1.f, 0.f, 0.f);
			}
			else if (CurrentMountHovered == CMH_GRIFFON)
			{
				overlaySpriteDimensions.z *= 512.f / 1664.f;
				overlaySpriteDimensions.w *= 512.f / 1664.f;
				direction = D3DXVECTOR4(0, 0.f, 0.f, 0.f);
			}

			direction.z = fmod(currentTime / 1000.f, 60000.f);
		}

		if (CurrentMountHovered != CMH_NONE)
		{
			D3DXVECTOR4 highlightSpriteDimensions = baseSpriteDimensions;
			if (CurrentMountHovered == CMH_GRIFFON)
			{
				highlightSpriteDimensions.z *= 512.f / 1664.f;
				highlightSpriteDimensions.w *= 512.f / 1664.f;
			}
			highlightSpriteDimensions.z *= 1.5f;
			highlightSpriteDimensions.w *= 1.5f;

			MainEffect->SetTechnique(CurrentMountHovered == CMH_GRIFFON ? "MountImageHighlightGriffon" : "MountImageHighlight");
			MainEffect->SetFloat("g_fTimer", sqrt(min(1.f, (currentTime - MountHoverTime) / 1000.f * 6)));
			MainEffect->SetTexture("texMountImage", BgTexture);
			MainEffect->SetVector("g_vSpriteDimensions", &highlightSpriteDimensions);
			MainEffect->SetVector("g_vDirection", &direction);

			MainEffect->Begin(&passes, 0);
			MainEffect->BeginPass(0);
			Quad->Draw();
			MainEffect->EndPass();
			MainEffect->End();
		}

		MainEffect->SetTechnique("MountImage");
		MainEffect->SetVector("g_vScreenSize", &screenSize);
		MainEffect->SetFloat("g_fTimer", min(1.f, (currentTime - OverlayTime) / 1000.f * 6));

		{
			D3DXVECTOR4 griffonSpriteDimensions = baseSpriteDimensions;
			griffonSpriteDimensions.z *= 512.f / 1664.f;
			griffonSpriteDimensions.w *= 512.f / 1664.f;

			MainEffect->SetVector("g_vSpriteDimensions", &griffonSpriteDimensions);
			MainEffect->SetTexture("texMountImage", MountTextures[5]);

			MainEffect->Begin(&passes, 0);
			MainEffect->BeginPass(0);
			Quad->Draw();
			MainEffect->EndPass();
			MainEffect->End();
		}

		MainEffect->SetVector("g_vSpriteDimensions", &baseSpriteDimensions);
		MainEffect->SetTexture("texMountImage", MountsTexture);

		MainEffect->Begin(&passes, 0);
		MainEffect->BeginPass(0);
		Quad->Draw();
		MainEffect->EndPass();
		MainEffect->End();

		if (CurrentMountHovered != CMH_NONE)
		{
			MainEffect->SetTechnique("MountImage");
			MainEffect->SetFloat("g_fTimer", sqrt(min(1.f, (currentTime - MountHoverTime) / 1000.f * 6)));
			MainEffect->SetTexture("texMountImage", MountTextures[CurrentMountHovered]);
			MainEffect->SetVector("g_vSpriteDimensions", &overlaySpriteDimensions);

			MainEffect->Begin(&passes, 0);
			MainEffect->BeginPass(0);
			Quad->Draw();
			MainEffect->EndPass();
			MainEffect->End();
		}

		{
			const auto& io = ImGui::GetIO();

			MainEffect->SetTechnique("Cursor");
			MainEffect->SetFloat("g_fTimer", fmod(currentTime / 1010.f, 55000.f));
			MainEffect->SetTexture("texMountImage", BgTexture);
			MainEffect->SetVector("g_vSpriteDimensions", &D3DXVECTOR4(io.MousePos.x * screenSize.z, io.MousePos.y * screenSize.w, 0.05f  * screenSize.y * screenSize.z, 0.05f));

			MainEffect->Begin(&passes, 0);
			MainEffect->BeginPass(0);
			Quad->Draw();
			MainEffect->EndPass();
			MainEffect->End();
		}
	}

	dev->EndScene();
}
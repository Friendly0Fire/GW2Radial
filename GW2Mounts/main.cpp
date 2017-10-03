#include "main.h"
#include "d3d9.h"
#include <tchar.h>
#include <imgui.h>
#include <examples\directx9_example\imgui_impl_dx9.h>
#include "simpleini\SimpleIni.h"
#include <set>
#include <sstream>
#include <Shlobj.h>
#include <iomanip>
#include <locale>
#include <codecvt>
#include "UnitQuad.h"
#include <d3dx9.h>

mstime timeInMS()
{
	mstime iCount;
	QueryPerformanceCounter((LARGE_INTEGER*)&iCount);
	mstime iFreq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&iFreq);
	return 1000 * iCount / iFreq;
}

bool LoadedFromGame = true;

// Config file settings
CSimpleIniA ini;
tstring ConfigFolder;
static const TCHAR* ConfigName = TEXT("config.ini");
static const TCHAR* ImGuiConfigName = TEXT("imgui_config.ini");
TCHAR ConfigLocation[MAX_PATH];
char ImGuiConfigLocation[MAX_PATH];

// Config data
std::set<uint> MountOverlayKeybind;
std::set<uint> MountKeybinds[5];
bool ShowGriffon = false;

// Active state
std::set<uint> DownKeys;
bool DisplayMountOverlay = false;
bool DisplayOptionsWindow = false;

char KeybindDisplayString[256];
bool SettingKeybind = false;
D3DXVECTOR2 OverlayPosition;
mstime OverlayTime;

WNDPROC BaseWndProc;
HMODULE OriginalD3D9 = nullptr;
HMODULE DllModule = nullptr;
IDirect3DDevice9* RealDevice = nullptr;

// Rendering
uint ScreenWidth, ScreenHeight;
std::unique_ptr<UnitQuad> Quad;
ID3DXEffect* MainEffect = nullptr;

/// <summary>
/// Converts a std::string into the equivalent std::wstring.
/// </summary>
/// <param name="str">The std::string.</param>
/// <returns>The converted std::wstring.</returns>
inline std::wstring s2ws(const std::string& str)
{
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

/// <summary>
/// Converts a std::wstring into the equivalent std::string. Possible loss of character information.
/// </summary>
/// <param name="wstr">The std::wstring.</param>
/// <returns>The converted std::string.</returns>
inline std::string ws2s(const std::wstring& wstr)
{
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

std::string GetKeyName(unsigned int virtualKey)
{
	unsigned int scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);

	// because MapVirtualKey strips the extended bit for some keys
	switch (virtualKey)
	{
	case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
	case VK_PRIOR: case VK_NEXT: // page up and page down
	case VK_END: case VK_HOME:
	case VK_INSERT: case VK_DELETE:
	case VK_DIVIDE: // numpad slash
	case VK_NUMLOCK:
		scanCode |= 0x100; // set extended bit
		break;
	}

	char keyName[50];
	if (GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName)) != 0)
		return keyName;
	else
		return "[Error]";
}

void SplitFilename(const tstring& str, tstring* folder, tstring* file)
{
	size_t found = str.find_last_of(TEXT("/\\"));
	if(folder) *folder = str.substr(0, found);
	if(file) *file = str.substr(found + 1);
}

void SetKeybindDisplayString(const std::set<uint>& keys)
{
	std::string keybind = "";
	for (const auto& k : keys)
	{
		keybind += GetKeyName(k) + std::string(" + ");
	}

	strcpy_s(KeybindDisplayString, (keybind.size() > 0 ? keybind.substr(0, keybind.size() - 3) : keybind).c_str());
}

IDirect3D9 *WINAPI Direct3DCreate9(UINT SDKVersion)
{
	if (!OriginalD3D9)
	{
		TCHAR path[MAX_PATH];
		GetSystemDirectory(path, MAX_PATH);
		_tcscat_s(path, TEXT("\\d3d9.dll"));
		OriginalD3D9 = LoadLibrary(path);
	}
	orig_Direct3DCreate9 = (D3DC9)GetProcAddress(OriginalD3D9, "Direct3DCreate9");

	if(LoadedFromGame)
		return new f_iD3D9(orig_Direct3DCreate9(SDKVersion));
	else
		return orig_Direct3DCreate9(SDKVersion);
}

bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		DllModule = hModule;

		// Create folders
		TCHAR exeFullPath[MAX_PATH];
		GetModuleFileName(0, exeFullPath, MAX_PATH);
		tstring exeFolder;
		SplitFilename(exeFullPath, &exeFolder, nullptr);
		ConfigFolder = exeFolder + TEXT("\\addons\\mounts\\");
		_tcscpy_s(ConfigLocation, (ConfigFolder + ConfigName).c_str());
#if _UNICODE
		strcpy_s(ImGuiConfigLocation, ws2s(ConfigFolder + ImGuiConfigName).c_str());
#else
		strcpy_s(ImGuiConfigLocation, (ConfigFolder + ImGuiConfigName).c_str());
#endif
		SHCreateDirectoryEx(nullptr, ConfigFolder.c_str(), nullptr);

		// Load INI settings
		ini.SetUnicode();
		ini.LoadFile(ConfigLocation);
		ShowGriffon = _stricmp(ini.GetValue("General", "show_fifth_mount", "false"), "true") == 0;
		const char* keys = ini.GetValue("Keybinds", "mount_wheel", nullptr);
		if (keys)
		{
			std::stringstream ss(keys);
			std::vector<std::string> result;

			while (ss.good())
			{
				std::string substr;
				std::getline(ss, substr, ',');
				int val = std::stoi(substr);
				MountOverlayKeybind.insert((uint)val);
			}

			SetKeybindDisplayString(MountOverlayKeybind);
		}
	}
	case DLL_PROCESS_DETACH:
	{
		if (OriginalD3D9)
		{
			FreeLibrary(OriginalD3D9);
			OriginalD3D9 = nullptr;
		}
	}
	}
	return true;
}

extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Transform SYSKEY* messages into KEY* messages instead
	UINT effective_msg = msg;
	if (msg == WM_SYSKEYDOWN)
	{
		effective_msg = WM_KEYDOWN;
		if (((lParam >> 29) & 1) == 1)
			DownKeys.insert(VK_MENU);
		else
			DownKeys.erase(VK_MENU);
	}
	if (msg == WM_SYSKEYUP)
		effective_msg = WM_KEYUP;

	if (effective_msg == WM_KEYDOWN)
		DownKeys.insert((uint)wParam);

	bool isMenuKeybind = GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_MENU) && wParam == 'M';

	if (effective_msg == WM_KEYDOWN || effective_msg == WM_KEYUP)
	{
		bool oldMountOverlay = DisplayMountOverlay;

		DisplayMountOverlay = !MountOverlayKeybind.empty() && DownKeys == MountOverlayKeybind;
		if (effective_msg == WM_KEYUP && MountOverlayKeybind.count((uint)wParam))
			DisplayMountOverlay = false;

		if (DisplayMountOverlay && !oldMountOverlay)
		{
			auto io = ImGui::GetIO();
			OverlayPosition.x = io.MousePos.x / (float)ScreenWidth;
			OverlayPosition.y = io.MousePos.y / (float)ScreenHeight;
			OverlayTime = timeInMS();
		}

		if (isMenuKeybind)
			DisplayOptionsWindow = true;
		else if (SettingKeybind)
		{
			SetKeybindDisplayString(DownKeys);

			switch (wParam)
			{
			case VK_MENU:
			case VK_CONTROL:
			case VK_SHIFT:
				break;
			default:
				SettingKeybind = false;
				MountOverlayKeybind = DownKeys;

				{
					std::string setting_value = "";
					for (const auto& k : MountOverlayKeybind)
						setting_value += std::to_string(k) + ", ";

					ini.SetValue("Keybinds", "mount_wheel", (setting_value.size() > 0 ? setting_value.substr(0, setting_value.size() - 2) : setting_value).c_str());
					ini.SaveFile(ConfigLocation);
				}
			}
		}
	}

	if (msg == WM_SYSKEYUP)
		DownKeys.erase(VK_MENU);
	if (effective_msg == WM_KEYUP)
		DownKeys.erase((uint)wParam);

	if ((effective_msg == WM_KEYDOWN || effective_msg == WM_KEYUP) && isMenuKeybind)
		return true;

	ImGui_ImplDX9_WndProcHandler(hWnd, msg, wParam, lParam);
	ImGuiIO& io = ImGui::GetIO();

	switch (effective_msg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
		if (io.WantCaptureMouse)
			return true;
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		if (io.WantCaptureKeyboard)
			return true;
		break;
	case WM_CHAR:
		if (io.WantTextInput)
			return true;
		break;
	}

	return CallWindowProc(BaseWndProc, hWnd, msg, wParam, lParam);
}

/*************************
Augmented Callbacks
*************************/

ULONG GameRefCount = 1;

HRESULT f_iD3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType,
	HWND hFocusWindow, DWORD BehaviorFlags,
	D3DPRESENT_PARAMETERS *pPresentationParameters,
	IDirect3DDevice9 **ppReturnedDeviceInterface)
{
	// Hook WndProc
	BaseWndProc = (WNDPROC)GetWindowLongPtr(hFocusWindow, GWLP_WNDPROC);
	SetWindowLongPtr(hFocusWindow, GWLP_WNDPROC, (LONG_PTR)&WndProc);

	// Create and initialize device
	IDirect3DDevice9* temp_device = nullptr;
	HRESULT hr = f_pD3D->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &temp_device);
	RealDevice = temp_device;
	*ppReturnedDeviceInterface = new f_IDirect3DDevice9(temp_device);

	// Init ImGui
	ImGuiIO& imio = ImGui::GetIO();
	imio.IniFilename = ImGuiConfigLocation;

	// Setup ImGui binding
	ImGui_ImplDX9_Init(hFocusWindow, temp_device);

	// Initialize graphics
	ScreenWidth = pPresentationParameters->BackBufferWidth;
	ScreenHeight = pPresentationParameters->BackBufferHeight;
	Quad = std::make_unique<UnitQuad>(RealDevice);
	HRESULT hr2 = D3DXCreateEffectFromResource(RealDevice, DllModule, MAKEINTRESOURCE(IDR_RCDATA1), nullptr, nullptr, 0, nullptr, &MainEffect, nullptr);

	// Initialize reference count for device object
	GameRefCount = 1;

	return hr;
}

HRESULT f_IDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	Quad.reset();

	HRESULT hr = f_pD3DDevice->Reset(pPresentationParameters);

	ImGui_ImplDX9_CreateDeviceObjects();
	Quad = std::make_unique<UnitQuad>(RealDevice);

	return hr;
}

HRESULT f_IDirect3DDevice9::EndScene()
{
	ImGui_ImplDX9_NewFrame();

	if (DisplayOptionsWindow)
	{
		ImGui::Begin("Mounts Options Menu", &DisplayOptionsWindow);
		ImGui::InputText("Keybind", KeybindDisplayString, 256, ImGuiInputTextFlags_ReadOnly);
		if (ImGui::Button("Set Keybind"))
		{
			MountOverlayKeybind.clear();
			SettingKeybind = true;
			KeybindDisplayString[0] = '\0';
		}
		if (ImGui::Checkbox("Show 5th mount", &ShowGriffon))
		{
			ini.SetValue("General", "show_fifth_mount", ShowGriffon ? "true" : "false");
			ini.SaveFile(ConfigLocation);
		}
		ImGui::End();
	}

	if (DisplayMountOverlay && MainEffect)
	{
		// Backup the DX9 state
		IDirect3DStateBlock9* d3d9_state_block = NULL;
		if (RealDevice->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) >= 0)
		{
			Quad->Bind();

			// Setup viewport
			D3DVIEWPORT9 vp;
			vp.X = vp.Y = 0;
			vp.Width = (DWORD)ScreenWidth;
			vp.Height = (DWORD)ScreenHeight;
			vp.MinZ = 0.0f;
			vp.MaxZ = 1.0f;
			RealDevice->SetViewport(&vp);

			auto io = ImGui::GetIO();

			D3DXVECTOR4 baseSpriteCenter;
			baseSpriteCenter.x = OverlayPosition.x;
			baseSpriteCenter.y = OverlayPosition.y;

			D3DXVECTOR4 screenSize(ScreenWidth, ScreenHeight, 1.f / ScreenWidth, 1.f / ScreenHeight);

			// Setup render state: fully shader-based
			MainEffect->SetTechnique("MountImage");
			MainEffect->SetVector("g_vScreenSize", &screenSize);

			for (uint j = 0; j < 4; j++)
			{
				auto spriteCenter = baseSpriteCenter;
				spriteCenter.x += ((j % 2 == 0) ? 0.2f : -0.2f) * screenSize.y * screenSize.z;
				spriteCenter.y += (j / 2 == 1) ? 0.2f : -0.2f;

				MainEffect->SetVector("g_vSpriteCenter", &spriteCenter);
				uint passes = 0;
				MainEffect->Begin(&passes, 0);

				for (uint i = 0; i < passes; i++)
				{
					MainEffect->BeginPass(i);

					Quad->Draw();

					MainEffect->EndPass();
				}
			}

			/*
			ImGui::Begin("Mounts Selector", &DisplayMountOverlay, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
			ImGui::LabelText("Test", "Test");
			ImGui::End();*/

			// Restore the DX9 state
			d3d9_state_block->Apply();
			d3d9_state_block->Release();
		}
	}

	ImGui::Render();

	return f_pD3DDevice->EndScene();
}

void Shutdown()
{
	ImGui_ImplDX9_Shutdown();

	Quad.reset();
	if (MainEffect)
		MainEffect->Release();
}

ULONG f_IDirect3DDevice9::AddRef()
{
	GameRefCount++;
	return f_pD3DDevice->AddRef();
}

ULONG f_IDirect3DDevice9::Release()
{
	GameRefCount--;
	if (GameRefCount == 0)
		Shutdown();

	return f_pD3DDevice->Release();
}
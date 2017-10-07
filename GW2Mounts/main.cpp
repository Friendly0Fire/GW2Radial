#include "main.h"
#include "d3d9.h"
#include <tchar.h>
#include <imgui.h>
#include <examples\directx9_example\imgui_impl_dx9.h>
#include <set>
#include <sstream>
#include "UnitQuad.h"
#include <d3dx9.h>
#include "Config.h"
#include "Utility.h"
#include <functional>

const float BaseSpriteSize = 0.4f;
const float CircleRadiusScreen = 256.f / 1664.f * BaseSpriteSize * 0.5f;

bool LoadedFromGame = true;
Config Cfg;
HWND GameWindow = 0;

// Active state
std::set<uint> DownKeys;
bool DisplayMountOverlay = false;
bool DisplayOverlayCursor = false;
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

struct DelayedInput
{
	uint msg;
	WPARAM wParam;
	LPARAM lParam;

	mstime t;
};

DelayedInput TransformVKey(uint vk, bool down, mstime t)
{
	DelayedInput i;
	i.t = t;
	if (vk == VK_LBUTTON || vk == VK_MBUTTON || vk == VK_RBUTTON)
	{
		i.wParam = i.lParam = 0;
		if (DownKeys.count(VK_CONTROL))
			i.wParam += MK_CONTROL;
		if (DownKeys.count(VK_SHIFT))
			i.wParam += MK_SHIFT;
		if (DownKeys.count(VK_LBUTTON))
			i.wParam += MK_LBUTTON;
		if (DownKeys.count(VK_RBUTTON))
			i.wParam += MK_RBUTTON;
		if (DownKeys.count(VK_MBUTTON))
			i.wParam += MK_MBUTTON;

		const auto& io = ImGui::GetIO();

		i.lParam = MAKELPARAM(((int)io.MousePos.x), ((int)io.MousePos.y));
	}
	else
	{
		i.wParam = vk;
		i.lParam = 1;
		i.lParam += (MapVirtualKeyEx(vk, MAPVK_VK_TO_VSC, 0) & 0xFF) << 16;
		if (!down)
			i.lParam += (1 << 30) + (1 << 31);
	}

	switch (vk)
	{
	case VK_LBUTTON:
		i.msg = down ? WM_LBUTTONDOWN : WM_LBUTTONUP;
		break;
	case VK_MBUTTON:
		i.msg = down ? WM_MBUTTONDOWN : WM_MBUTTONUP;
		break;
	case VK_RBUTTON:
		i.msg = down ? WM_RBUTTONDOWN : WM_RBUTTONUP;
		break;
	case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
	case VK_PRIOR: case VK_NEXT: // page up and page down
	case VK_END: case VK_HOME:
	case VK_INSERT: case VK_DELETE:
	case VK_DIVIDE: // numpad slash
	case VK_NUMLOCK:
		i.lParam |= 1 << 24; // set extended bit
	default:
		i.msg = down ? WM_KEYDOWN : WM_KEYUP;
		break;
	}

	return i;
}


std::list<DelayedInput> QueuedInputs;

void SendKeybind(const std::set<uint>& vkeys)
{
	if (vkeys.empty())
		return;

	std::list<uint> vkeys_sorted(vkeys.begin(), vkeys.end());
	vkeys_sorted.sort([](uint& a, uint& b) {
		if (a == VK_CONTROL || a == VK_SHIFT || a == VK_MENU)
			return true;
		else
			return a < b;
	});

	mstime currentTime = timeInMS() + 10;

	for (const auto& vk : vkeys_sorted)
	{
		if (DownKeys.count(vk))
			continue;

		DelayedInput i = TransformVKey(vk, true, currentTime);
		QueuedInputs.push_back(i);
		currentTime += 20;
	}

	currentTime += 50;

	for (const auto& vk : reverse(vkeys_sorted))
	{
		if (DownKeys.count(vk))
			continue;

		DelayedInput i = TransformVKey(vk, false, currentTime);
		QueuedInputs.push_back(i);
		currentTime += 20;
	}
}

void SendQueuedInputs()
{
	if (QueuedInputs.empty())
		return;

	mstime currentTime = timeInMS();

	auto& qi = QueuedInputs.front();

	if (currentTime < qi.t)
		return;

	PostMessage(GameWindow, qi.msg, qi.wParam, qi.lParam);

	QueuedInputs.pop_front();
}

WNDPROC BaseWndProc;
HMODULE OriginalD3D9 = nullptr;
HMODULE DllModule = nullptr;
IDirect3DDevice9* RealDevice = nullptr;

// Rendering
uint ScreenWidth, ScreenHeight;
std::unique_ptr<UnitQuad> Quad;
ID3DXEffect* MainEffect = nullptr;
IDirect3DTexture9* MountsTexture = nullptr;
IDirect3DTexture9* MountTextures[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
IDirect3DTexture9* BgTexture = nullptr;

void LoadMountTextures()
{
	D3DXCreateTextureFromResource(RealDevice, DllModule, MAKEINTRESOURCE(IDR_BG), &BgTexture);
	D3DXCreateTextureFromResource(RealDevice, DllModule, MAKEINTRESOURCE(IDR_MOUNTS), &MountsTexture);
	for (uint i = 0; i < 6; i++)
		D3DXCreateTextureFromResource(RealDevice, DllModule, MAKEINTRESOURCE(IDR_MOUNT1 + i), &MountTextures[i]);
}

void UnloadMountTextures()
{
	COM_RELEASE(MountsTexture);
	COM_RELEASE(BgTexture);

	for (uint i = 0; i < 6; i++)
		COM_RELEASE(MountTextures[i]);
}

IDirect3D9 *WINAPI Direct3DCreate9(UINT SDKVersion)
{
	assert(SDKVersion == D3D_SDK_VERSION);
	if (!OriginalD3D9)
	{
		TCHAR path[MAX_PATH];

		// Try to chainload first
		GetCurrentDirectory(MAX_PATH, path);
		_tcscat_s(path, TEXT("\\d3d9_mchain.dll"));

		if (!FileExists(path))
		{
			GetCurrentDirectory(MAX_PATH, path);
			_tcscat_s(path, TEXT("\\bin64\\d3d9_mchain.dll"));
		}

		if (!FileExists(path))
		{
			GetSystemDirectory(path, MAX_PATH);
			_tcscat_s(path, TEXT("\\d3d9.dll"));
		}

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
	else if (Cfg.ShowGriffon())
		CurrentMountHovered = CMH_GRIFFON;
	else
		CurrentMountHovered = CMH_NONE;

	if (LastMountHovered != CurrentMountHovered)
		MountHoverTime = timeInMS();
}

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
		
	// Very exclusive test: *only* consider the menu keybind to be activated if they're the *only* keys currently down
	// This minimizes the likelihood of the menu randomly popping up when it shouldn't
	bool isMenuKeybind = DownKeys == Cfg.SettingsKeybind();

	// Only run these for key down/key up (incl. mouse buttons) events
	if (!eventKeys.empty())
	{
		bool oldMountOverlay = DisplayMountOverlay;

		bool mountOverlay = !Cfg.MountOverlayKeybind().empty() && std::includes(DownKeys.begin(), DownKeys.end(), Cfg.MountOverlayKeybind().begin(), Cfg.MountOverlayKeybind().end());
		bool mountOverlayLocked = !Cfg.MountOverlayLockedKeybind().empty() && std::includes(DownKeys.begin(), DownKeys.end(), Cfg.MountOverlayLockedKeybind().begin(), Cfg.MountOverlayLockedKeybind().end());

		DisplayMountOverlay = mountOverlayLocked || mountOverlay;

		if (DisplayMountOverlay && !oldMountOverlay)
		{
			// Mount overlay is turned on

			DisplayOverlayCursor = mountOverlayLocked;
			if (DisplayOverlayCursor)
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

		if (isMenuKeybind)
			DisplayOptionsWindow = true;
		else
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

/*************************
Augmented Callbacks
*************************/

ULONG GameRefCount = 1;

HRESULT f_iD3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType,
	HWND hFocusWindow, DWORD BehaviorFlags,
	D3DPRESENT_PARAMETERS *pPresentationParameters,
	IDirect3DDevice9 **ppReturnedDeviceInterface)
{
	GameWindow = hFocusWindow;

	// Hook WndProc
	BaseWndProc = (WNDPROC)GetWindowLongPtr(hFocusWindow, GWLP_WNDPROC);
	SetWindowLongPtr(hFocusWindow, GWLP_WNDPROC, (LONG_PTR)&WndProc);

	// Create and initialize device
	IDirect3DDevice9* temp_device = nullptr;
	HRESULT hr = f_pD3D->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &temp_device);
	RealDevice = temp_device;
	*ppReturnedDeviceInterface = new f_IDirect3DDevice9(temp_device);

	// Init ImGui
	auto& imio = ImGui::GetIO();
	imio.IniFilename = Cfg.ImGuiConfigLocation();

	// Setup ImGui binding
	ImGui_ImplDX9_Init(hFocusWindow, temp_device);

	// Initialize graphics
	ScreenWidth = pPresentationParameters->BackBufferWidth;
	ScreenHeight = pPresentationParameters->BackBufferHeight;
	try
	{
		Quad = std::make_unique<UnitQuad>(RealDevice);
	}
	catch (...)
	{
		Quad = nullptr;
	}
	ID3DXBuffer* errorBuffer = nullptr;
	D3DXCreateEffectFromResource(RealDevice, DllModule, MAKEINTRESOURCE(IDR_SHADER), nullptr, nullptr, 0, nullptr, &MainEffect, &errorBuffer);
	COM_RELEASE(errorBuffer);
	LoadMountTextures();

	// Initialize reference count for device object
	GameRefCount = 1;

	return hr;
}

HRESULT f_IDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	Quad.reset();
	UnloadMountTextures();
	COM_RELEASE(MainEffect);

	HRESULT hr = f_pD3DDevice->Reset(pPresentationParameters);

	ScreenWidth = pPresentationParameters->BackBufferWidth;
	ScreenHeight = pPresentationParameters->BackBufferHeight;

	ImGui_ImplDX9_CreateDeviceObjects();
	ID3DXBuffer* errorBuffer = nullptr;
	D3DXCreateEffectFromResource(RealDevice, DllModule, MAKEINTRESOURCE(IDR_SHADER), nullptr, nullptr, 0, nullptr, &MainEffect, &errorBuffer);
	COM_RELEASE(errorBuffer);
	LoadMountTextures();
	try
	{
		Quad = std::make_unique<UnitQuad>(RealDevice);
	}
	catch (...)
	{
		Quad = nullptr;
	}

	return hr;
}


HRESULT f_IDirect3DDevice9::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	// This is the closest we have to a reliable "update" function, so use it as one
	SendQueuedInputs();

	// We have to use Present rather than hooking EndScene because the game seems to do final UI compositing after EndScene
	// This unfortunately means that we have to call Begin/EndScene before Present so we can render things, but thankfully for modern GPUs that doesn't cause bugs
	f_pD3DDevice->BeginScene();

	ImGui_ImplDX9_NewFrame();

	if (DisplayOptionsWindow)
	{
		ImGui::Begin("Mounts Options Menu", &DisplayOptionsWindow);

		ImGuiKeybindInput("Overlay Keybind", MainKeybind);
		ImGuiKeybindInput("Overlay Keybind (Center Locked)", MainLockedKeybind);

		if (Cfg.ShowGriffon() != ImGui::Checkbox("Show 5th mount", &Cfg.ShowGriffon()))
			Cfg.ShowGriffonSave();
		if (Cfg.ResetCursorOnLockedKeybind() != ImGui::Checkbox("Reset cursor to center with Center Locked keybind", &Cfg.ResetCursorOnLockedKeybind()))
			Cfg.ResetCursorOnLockedKeybindSave();

		ImGui::Separator();
		ImGui::Text("Mount Keybinds");
		ImGui::Text("(set to relevant game keybinds)");

		for (uint i = 0; i < (Cfg.ShowGriffon() ? 5u : 4u); i++)
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
		RealDevice->SetViewport(&vp);

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

		if (Cfg.ShowGriffon())
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

		if (DisplayOverlayCursor)
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

	f_pD3DDevice->EndScene();

	return f_pD3DDevice->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

void Shutdown()
{
	ImGui_ImplDX9_Shutdown();

	Quad.reset();
	COM_RELEASE(MainEffect);

	UnloadMountTextures();
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
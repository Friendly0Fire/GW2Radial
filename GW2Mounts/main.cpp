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
#include "imgui_ext.h"
#define _USE_MATH_DEFINES
#include <math.h>

void PreReset();

const float BaseSpriteSize = 0.4f;
const float CircleRadiusScreen = 256.f / 1664.f * BaseSpriteSize * 0.5f;

Config Cfg;
HWND GameWindow = 0;

// Active state
bool DisplayMountOverlay = false;
bool DisplayOptionsWindow = false;

ImGuiKeybind MainKeybind;
ImGuiKeybind MainLockedKeybind;
ImGuiKeybind MountKeybinds[MountTypeCount];

D3DXVECTOR2 OverlayPosition;
mstime OverlayTime, MountHoverTime;

MountType CurrentMountHovered = MountType::NONE;

const char* GetMountName(MountType m)
{
	switch (m)
	{
	case MountType::RAPTOR:
		return "Raptor";
	case MountType::SPRINGER:
		return "Springer";
	case MountType::SKIMMER:
		return "Skimmer";
	case MountType::JACKAL:
		return "Jackal";
	case MountType::BEETLE:
		return "Beetle";
	case MountType::GRIFFON:
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
IDirect3DTexture9* MountTextures[MountTypeCount] = { nullptr, nullptr, nullptr, nullptr, nullptr };
IDirect3DTexture9* BgTexture = nullptr;

void LoadMountTextures(IDirect3DDevice9* dev)
{
	D3DXCreateTextureFromResource(dev, DllModule, MAKEINTRESOURCE(IDR_BG), &BgTexture);
	for (uint i = 0; i < MountTypeCount; i++)
		D3DXCreateTextureFromResource(dev, DllModule, MAKEINTRESOURCE(IDR_MOUNTS + i), &MountTextures[i]);
}

void UnloadMountTextures()
{
	COM_RELEASE(BgTexture);
	for (uint i = 0; i < MountTypeCount; i++)
		COM_RELEASE(MountTextures[i]);
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
#ifdef _DEBUG
		while (!IsDebuggerPresent());
#endif
		DllModule = hModule;

		// Add an extra reference count to the library so it persists through GW2's load-unload routine
		// without which problems start arising with ReShade
		{
			TCHAR selfpath[MAX_PATH];
			GetModuleFileName(DllModule, selfpath, MAX_PATH);
			LoadLibrary(selfpath);
		}

		MH_Initialize();

		Cfg.Load();

		MainKeybind.UpdateDisplayString(Cfg.MountOverlayKeybind());
		MainKeybind.SetCallback = [](const std::set<uint>& val) { Cfg.MountOverlayKeybind(val); };
		MainLockedKeybind.UpdateDisplayString(Cfg.MountOverlayLockedKeybind());
		MainLockedKeybind.SetCallback = [](const std::set<uint>& val) { Cfg.MountOverlayLockedKeybind(val); };
		for (uint i = 0; i < MountTypeCount; i++)
		{
			MountKeybinds[i].UpdateDisplayString(Cfg.MountKeybind(i));
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

std::vector<MountType> GetActiveMounts()
{
	std::vector<MountType> ActiveMounts;
	for (uint i = 0; i < MountTypeCount; i++)
	{
		if (!Cfg.MountKeybind(i).empty())
			ActiveMounts.push_back((MountType)i);
	}

	return ActiveMounts;
}

void DetermineHoveredMount()
{
	const auto io = ImGui::GetIO();

	D3DXVECTOR2 MousePos;
	MousePos.x = io.MousePos.x / (float)ScreenWidth;
	MousePos.y = io.MousePos.y / (float)ScreenHeight;
	MousePos -= OverlayPosition;

	MountType LastMountHovered = CurrentMountHovered;

	// Middle circle does not count as a hover event
	if (D3DXVec2LengthSq(&MousePos) > CircleRadiusScreen * CircleRadiusScreen)
	{
		float MouseAngle = atan2(MousePos.y, MousePos.x);
		if (MouseAngle < 0)
			MouseAngle += float(2 * M_PI);

		std::vector<MountType> ActiveMounts = GetActiveMounts();
		float MountAngle = float(2 * M_PI) / ActiveMounts.size();
		int MountId = int(MouseAngle / MountAngle);

		CurrentMountHovered = ActiveMounts[MountId];
	}
	else
		CurrentMountHovered = MountType::NONE;

	if (LastMountHovered != CurrentMountHovered)
		MountHoverTime = max(OverlayTime + Cfg.OverlayDelayMilliseconds(), timeInMS());
}

void Shutdown();

extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_KILLFOCUS)
	{
		CurrentMountHovered = MountType::NONE;
		DisplayMountOverlay = false;
		DownKeys.clear();
	}
	else
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
				if ((msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP) && wParam != VK_F10)
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

			if (isMenuKeybind)
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
					if (CurrentMountHovered != MountType::NONE)
						SendKeybind(Cfg.MountKeybind((uint)CurrentMountHovered));

					CurrentMountHovered = MountType::NONE;
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

					MainKeybind.UpdateKeybind(fullKeybind, keyLifted);
					MainLockedKeybind.UpdateKeybind(fullKeybind, keyLifted);

					for (uint i = 0; i < MountTypeCount; i++)
						MountKeybinds[i].UpdateKeybind(fullKeybind, keyLifted);
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

void Draw(IDirect3DDevice9* dev, bool FrameDrawn, bool SceneEnded)
{
	// This is the closest we have to a reliable "update" function, so use it as one
	SendQueuedInputs();

	if (FrameDrawn)
		return;

	// We have to use Present rather than hooking EndScene because the game seems to do final UI compositing after EndScene
	// This unfortunately means that we have to call Begin/EndScene before Present so we can render things, but thankfully for modern GPUs that doesn't cause bugs
	if (SceneEnded)
		dev->BeginScene();

	ImGui_ImplDX9_NewFrame();

	if (DisplayOptionsWindow)
	{
		ImGui::Begin("Mounts Options Menu", &DisplayOptionsWindow);

		if (ImGui::SliderInt("Pop-up Delay", &Cfg.OverlayDelayMilliseconds(), 0, 1000, "%.0f ms"))
			Cfg.OverlayDelayMillisecondsSave();

		if (Cfg.ResetCursorOnLockedKeybind() != ImGui::Checkbox("Reset cursor to center with Center Locked keybind", &Cfg.ResetCursorOnLockedKeybind()))
			Cfg.ResetCursorOnLockedKeybindSave();
		if (Cfg.LockCameraWhenOverlayed() != ImGui::Checkbox("Lock camera when overlay is displayed", &Cfg.LockCameraWhenOverlayed()))
			Cfg.LockCameraWhenOverlayedSave();

		ImGui::Separator();

		ImGuiKeybindInput("Overlay Keybind", MainKeybind);
		ImGuiKeybindInput("Overlay Keybind (Center Locked)", MainLockedKeybind);

		ImGui::Separator();
		ImGui::Text("Mount Keybinds");
		ImGui::Text("(set to relevant game keybinds)");

		for (uint i = 0; i < MountTypeCount; i++)
			ImGuiKeybindInput(GetMountName((MountType)i), MountKeybinds[i]);

		ImGui::End();
	}

	ImGui::Render();

	if (DisplayMountOverlay && MainEffect && Quad)
	{
		Quad->Bind();

		auto currentTime = timeInMS();

		if (currentTime >= OverlayTime + Cfg.OverlayDelayMilliseconds())
		{
			uint passes = 0;

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

			MainEffect->SetTechnique("MountImage");
			MainEffect->SetVector("g_vScreenSize", &screenSize);
			MainEffect->SetFloat("g_fTimer", min(1.f, (currentTime - OverlayTime - Cfg.OverlayDelayMilliseconds()) / 1000.f * 6));
			MainEffect->Begin(&passes, 0);
			MainEffect->BeginPass(0);

			auto ActiveMounts = GetActiveMounts();
			int n = 0;
			for (auto it : ActiveMounts)
			{
				D3DXVECTOR4 spriteDimensions = baseSpriteDimensions;
				const float distance = 0.67f;

				float mountAngle = ((float)n + 0.0f) / (float)ActiveMounts.size() * 2 * M_PI;
				D3DXVECTOR2 mountLocation = D3DXVECTOR2(cos(mountAngle - M_PI / 2), sin(mountAngle - M_PI / 2));
				float mountRadius = tan(M_PI / (float)ActiveMounts.size()) * 2 * distance;
				spriteDimensions.z *= mountRadius / (2 * M_PI * distance);
				spriteDimensions.w *= mountRadius / (2 * M_PI * distance);
				spriteDimensions.x += mountLocation.x * spriteDimensions.z;
				spriteDimensions.y += mountLocation.y * spriteDimensions.w;

				int v[3] = { (int)it, n, (int)ActiveMounts.size() };
				MainEffect->SetValue("g_iMountID", v, sizeof(v));
				MainEffect->SetTexture("texMountImage", MountTextures[(uint)it]);
				MainEffect->SetVector("g_vSpriteDimensions", &spriteDimensions);
				MainEffect->CommitChanges();

				Quad->Draw();
				n++;
			}

			MainEffect->EndPass();
			MainEffect->End();

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
	}

	if (SceneEnded)
		dev->EndScene();
}
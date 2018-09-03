#include "main.h"
#include <tchar.h>
#include <imgui.h>
#include <examples\imgui_impl_dx9.h>
#include <examples\imgui_impl_win32.h>
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

const float CircleRadiusBase = 256.f / 1664.f * 0.25f;

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

MountType PreviousMountUsed = MountType::NONE;
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
	ImGui_ImplWin32_Shutdown();
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
		ImGui::CreateContext();

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
		ImGui::DestroyContext();
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

std::vector<MountType> GetAllMounts()
{
	std::vector<MountType> AllMounts;
	for (uint i = 0; i < MountTypeCount; i++)
	{
		AllMounts.push_back((MountType)i);
	}

	return AllMounts;
}

MountType ModifyMountNoneBehavior(MountType in)
{
	if (in != MountType::NONE)
		return in;

	switch (Cfg.OverlayDeadZoneBehavior())
	{
	case 1:
		return PreviousMountUsed;
	case 2:
		return Cfg.FavoriteMount();
	default:
		return MountType::NONE;
	}
}

void DetermineHoveredMount()
{
	const auto io = ImGui::GetIO();

	D3DXVECTOR2 MousePos;
	MousePos.x = io.MousePos.x / (float)ScreenWidth;
	MousePos.y = io.MousePos.y / (float)ScreenHeight;
	MousePos -= OverlayPosition;

	MousePos.y *= (float)ScreenHeight / (float)ScreenWidth;

	MountType LastMountHovered = CurrentMountHovered;

	std::vector<MountType> ActiveMounts = GetActiveMounts();

	// Middle circle does not count as a hover event
	if (!ActiveMounts.empty() && D3DXVec2LengthSq(&MousePos) > SQUARE(Cfg.OverlayScale() * 0.135f * Cfg.OverlayDeadZoneScale()))
	{
		float MouseAngle = atan2(-MousePos.y, -MousePos.x) - 0.5f * (float)M_PI;
		if (MouseAngle < 0)
			MouseAngle += float(2 * M_PI);

		float MountAngle = float(2 * M_PI) / ActiveMounts.size();
		int MountId = int((MouseAngle - MountAngle / 2) / MountAngle + 1) % ActiveMounts.size();

		CurrentMountHovered = ActiveMounts[MountId];
	}
	else
		CurrentMountHovered = MountType::NONE;

	auto modifiedMountHovered = ModifyMountNoneBehavior(CurrentMountHovered);
	auto modifiedLastMountHovered = ModifyMountNoneBehavior(LastMountHovered);

	if (LastMountHovered != CurrentMountHovered && modifiedLastMountHovered != modifiedMountHovered)
		MountHoverTime = max(OverlayTime + Cfg.OverlayDelayMilliseconds(), timeInMS());
}

void Shutdown();

extern IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
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
					MountHoverTime = OverlayTime + Cfg.OverlayDelayMilliseconds();

					DetermineHoveredMount();
				}
				else if (!DisplayMountOverlay && oldMountOverlay)
				{
					// Check for special behavior if no mount is hovered
					CurrentMountHovered = ModifyMountNoneBehavior(CurrentMountHovered);

					// Mount overlay is turned off, send the keybind
					if (CurrentMountHovered != MountType::NONE)
						SendKeybind(Cfg.MountKeybind((uint)CurrentMountHovered));

					PreviousMountUsed = CurrentMountHovered;
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

		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
		if (msg == WM_MOUSEMOVE)
		{
			auto& io = ImGui::GetIO();
			io.MousePos.x = (signed short)(lParam);
			io.MousePos.y = (signed short)(lParam >> 16);
		}

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

	// Convert hook messages back into their original messages
	msg = ConvertHookedMessage(msg);

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
		InitializeHookedMessages();
	}
}

void PostCreateDevice(IDirect3DDevice9* temp_device, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	// Init ImGui
	auto& imio = ImGui::GetIO();
	imio.IniFilename = Cfg.ImGuiConfigLocation();

	// Setup ImGui binding
	ImGui_ImplDX9_Init(temp_device);
	ImGui_ImplWin32_Init(GameWindow);

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

bool FirstFrame = true;
std::vector<const char*> FavoriteMountNames(MountTypeCount);
void Draw(IDirect3DDevice9* dev, bool FrameDrawn, bool SceneEnded)
{
	// This is the closest we have to a reliable "update" function, so use it as one
	SendQueuedInputs();

	if (FrameDrawn)
		return;

	if (!FirstFrame)
	{
		// We have to use Present rather than hooking EndScene because the game seems to do final UI compositing after EndScene
		// This unfortunately means that we have to call Begin/EndScene before Present so we can render things, but thankfully for modern GPUs that doesn't cause bugs
		if (SceneEnded)
			dev->BeginScene();

		if (DisplayOptionsWindow)
		{
			ImGui::Begin("Mounts Options Menu", &DisplayOptionsWindow);

			if (ImGui::SliderInt("Pop-up Delay", &Cfg.OverlayDelayMilliseconds(), 0, 1000, "%d ms"))
				Cfg.OverlayDelayMillisecondsSave();

			if (ImGui::SliderFloat("Overlay Scale", &Cfg.OverlayScale(), 0.f, 4.f))
				Cfg.OverlayScaleSave();

			if (ImGui::SliderFloat("Overlay Dead Zone Scale", &Cfg.OverlayDeadZoneScale(), 0.f, 0.25f))
				Cfg.OverlayDeadZoneScaleSave();

			ImGui::Text("Overlay Dead Zone Behavior:");
			int oldBehavior = Cfg.OverlayDeadZoneBehavior();
			ImGui::RadioButton("Nothing", &Cfg.OverlayDeadZoneBehavior(), 0);
			ImGui::RadioButton("Last Mount", &Cfg.OverlayDeadZoneBehavior(), 1);
			ImGui::RadioButton("Favorite Mount", &Cfg.OverlayDeadZoneBehavior(), 2);
			if (oldBehavior != Cfg.OverlayDeadZoneBehavior())
				Cfg.OverlayDeadZoneBehaviorSave();

			if (Cfg.OverlayDeadZoneBehavior() == 2)
			{
				if (FavoriteMountNames[0] == nullptr)
				{
					auto mounts = GetAllMounts();
					for (uint i = 0; i < mounts.size(); i++)
						FavoriteMountNames[i] = GetMountName(mounts[i]);
				}

				if (ImGui::Combo("Favorite Mount", (int*)&Cfg.FavoriteMount(), FavoriteMountNames.data(), (int)FavoriteMountNames.size()))
					Cfg.FavoriteMountSave();
			}

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
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

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

				auto ActiveMounts = GetActiveMounts();
				if (!ActiveMounts.empty())
				{
					D3DXVECTOR4 baseSpriteDimensions;
					baseSpriteDimensions.x = OverlayPosition.x;
					baseSpriteDimensions.y = OverlayPosition.y;
					baseSpriteDimensions.z = Cfg.OverlayScale() * 0.5f * screenSize.y * screenSize.z;
					baseSpriteDimensions.w = Cfg.OverlayScale() * 0.5f;

					float fadeTimer = min(1.f, (currentTime - (OverlayTime + Cfg.OverlayDelayMilliseconds())) / 1000.f * 6);
					float hoverTimer = min(1.f, (currentTime - max(MountHoverTime, OverlayTime + Cfg.OverlayDelayMilliseconds())) / 1000.f * 6);

					auto mountHovered = ModifyMountNoneBehavior(CurrentMountHovered);

					MainEffect->SetTechnique("BgImage");
					MainEffect->SetTexture("texBgImage", BgTexture);
					MainEffect->SetVector("g_vSpriteDimensions", &baseSpriteDimensions);
					MainEffect->SetFloat("g_fFadeTimer", fadeTimer);
					MainEffect->SetFloat("g_fHoverTimer", hoverTimer);
					MainEffect->SetFloat("g_fTimer", fmod(currentTime / 1010.f, 55000.f));
					MainEffect->SetFloat("g_fDeadZoneScale", Cfg.OverlayDeadZoneScale());
					MainEffect->SetInt("g_iMountCount", (int)ActiveMounts.size());
					MainEffect->SetInt("g_iMountHovered", (int)(std::find(ActiveMounts.begin(), ActiveMounts.end(), mountHovered) - ActiveMounts.begin()));
					MainEffect->SetBool("g_bCenterGlow", mountHovered != CurrentMountHovered);
					MainEffect->Begin(&passes, 0);
					MainEffect->BeginPass(0);
					Quad->Draw();
					MainEffect->EndPass();
					MainEffect->End();

					MainEffect->SetTechnique("MountImage");
					MainEffect->SetTexture("texBgImage", BgTexture);
					MainEffect->SetVector("g_vScreenSize", &screenSize);
					MainEffect->Begin(&passes, 0);
					MainEffect->BeginPass(0);

					int n = 0;
					for (auto it : ActiveMounts)
					{
						D3DXVECTOR4 spriteDimensions = baseSpriteDimensions;

						float mountAngle = (float)n / (float)ActiveMounts.size() * 2 * (float)M_PI;
						if (ActiveMounts.size() == 1)
							mountAngle = 0;
						D3DXVECTOR2 mountLocation = D3DXVECTOR2(cos(mountAngle - (float)M_PI / 2), sin(mountAngle - (float)M_PI / 2)) * 0.25f * 0.66f;

						spriteDimensions.x += mountLocation.x * spriteDimensions.z;
						spriteDimensions.y += mountLocation.y * spriteDimensions.w;

						float mountDiameter = (float)sin((2 * M_PI / (double)ActiveMounts.size()) / 2) * 2.f * 0.2f * 0.66f;
						if (ActiveMounts.size() == 1)
							mountDiameter = 2.f * 0.2f;
						if (it == mountHovered)
							mountDiameter *= lerp(1.f, 1.1f, smoothstep(hoverTimer));
						else
							mountDiameter *= 0.9f;

						switch (ActiveMounts.size())
						{
						case 1:
							spriteDimensions.z *= 0.8f;
							spriteDimensions.w *= 0.8f;
							break;
						case 2:
							spriteDimensions.z *= 0.85f;
							spriteDimensions.w *= 0.85f;
							break;
						case 3:
							spriteDimensions.z *= 0.9f;
							spriteDimensions.w *= 0.9f;
							break;
						case 4:
							spriteDimensions.z *= 0.95f;
							spriteDimensions.w *= 0.95f;
							break;
						}

						spriteDimensions.z *= mountDiameter;
						spriteDimensions.w *= mountDiameter;

						int v[3] = { (int)it, n, (int)ActiveMounts.size() };
						MainEffect->SetValue("g_iMountID", v, sizeof(v));
						MainEffect->SetBool("g_bMountHovered", mountHovered == it);
						MainEffect->SetTexture("texMountImage", MountTextures[(uint)it]);
						MainEffect->SetVector("g_vSpriteDimensions", &spriteDimensions);
						MainEffect->SetValue("g_vColor", MountColors[(uint)it].data(), sizeof(D3DXVECTOR4));
						MainEffect->CommitChanges();

						Quad->Draw();
						n++;
					}

					MainEffect->EndPass();
					MainEffect->End();
				}

				{
					const auto& io = ImGui::GetIO();

					MainEffect->SetTechnique("Cursor");
					MainEffect->SetTexture("texBgImage", BgTexture);
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

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	FirstFrame = false;
}
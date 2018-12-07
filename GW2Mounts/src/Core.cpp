#include <Core.h>
#include <Direct3D9Hooks.h>
#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_dx9.h>
#include <imgui/examples/imgui_impl_win32.h>
#include <Input.h>
#include <ConfigurationFile.h>
#include <UnitQuad.h>

namespace GW2Addons
{

void Core::Init(HMODULE dll)
{
	i()->dllModule_ = dll;
	i()->InternalInit();
}

void Core::Shutdown()
{
	ImGui::DestroyContext();
	// We'll just leak a bunch of things and let the driver/OS take care of it, since we have no clean exit point
	// and calling FreeLibrary in DllMain causes deadlocks
}

Core::~Core()
{
	Direct3D9Hooks::i()->preResetCallback()();
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
}

void Core::InternalInit()
{
	// Add an extra reference count to the library so it persists through GW2's load-unload routine
	// without which problems start arising with ReShade
	{
		TCHAR selfpath[MAX_PATH];
		GetModuleFileName(dllModule_, selfpath, MAX_PATH);
		LoadLibrary(selfpath);
	}
	
	Direct3D9Hooks::i()->preCreateDeviceCallback([this](HWND hWnd){ PreCreateDevice(hWnd); });
	Direct3D9Hooks::i()->postCreateDeviceCallback([this](IDirect3DDevice9* d, D3DPRESENT_PARAMETERS* pp){ PostCreateDevice(d, pp); });
	
	Direct3D9Hooks::i()->preResetCallback([this](){ PreReset(); });
	Direct3D9Hooks::i()->postResetCallback([this](IDirect3DDevice9* d, D3DPRESENT_PARAMETERS* pp){ PostReset(d, pp); });
	
	Direct3D9Hooks::i()->drawOverCallback([this](IDirect3DDevice9* d, bool frameDrawn, bool sceneEnded){ DrawOver(d, frameDrawn, sceneEnded); });
	//Direct3D9Hooks::i()->drawUnderCallback([this](IDirect3DDevice9* d, bool frameDrawn, bool sceneEnded){ DrawUnder(d, frameDrawn, sceneEnded); });

	ImGui::CreateContext();

	MainKeybind.UpdateDisplayString(Cfg.MountOverlayKeybind());
	MainKeybind.SetCallback = [](const std::set<uint>& val) { Cfg.MountOverlayKeybind(val); };
	MainLockedKeybind.UpdateDisplayString(Cfg.MountOverlayLockedKeybind());
	MainLockedKeybind.SetCallback = [](const std::set<uint>& val) { Cfg.MountOverlayLockedKeybind(val); };
	for (uint i = 0; i < MountTypeCount; i++)
	{
		MountKeybinds[i].UpdateDisplayString(Cfg.MountKeybind(i));
		MountKeybinds[i].SetCallback = [i](const std::set<uint>& val) { Cfg.MountKeybind(i, val); };
	}
}

void Core::OnFocusLost()
{
	CurrentMountHovered = MountType::NONE;
	DisplayMountOverlay = false;
	Input::i()->OnFocusLost();
}

extern IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT Core::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_KILLFOCUS)
		i()->OnFocusLost();
	else
	{
		const auto consume = Input::i()->OnInput(msg, wParam, lParam);
		if(consume)
			return 0;
	}

	// Whatever's left should be sent to the game
	return CallWindowProc(i()->baseWndProc_, hWnd, msg, wParam, lParam);
}

void Core::PreCreateDevice(HWND hFocusWindow)
{
	gameWindow_ = hFocusWindow;

	// Hook WndProc
	if (!baseWndProc_)
	{
		baseWndProc_ = (WNDPROC)GetWindowLongPtr(hFocusWindow, GWLP_WNDPROC);
		SetWindowLongPtr(hFocusWindow, GWLP_WNDPROC, (LONG_PTR)&WndProc);
	}
}

void Core::PostCreateDevice(IDirect3DDevice9 *temp_device, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	// Init ImGui
	auto &imio = ImGui::GetIO();
	imio.IniFilename = ConfigurationFile::i()->imguiLocation();

	// Setup ImGui binding
	ImGui_ImplDX9_Init(temp_device);
	ImGui_ImplWin32_Init(gameWindow_);

	// Initialize graphics
	screenWidth_ = pPresentationParameters->BackBufferWidth;
	screenHeight_ = pPresentationParameters->BackBufferHeight;
	try { quad_ = std::make_unique<UnitQuad>(temp_device); }
	catch (...) { quad_ = nullptr; }
	ID3DXBuffer *errorBuffer = nullptr;
	D3DXCreateEffectFromResource(temp_device, dllModule_, MAKEINTRESOURCE(IDR_SHADER), nullptr, nullptr, 0, nullptr,
	                             &mainEffect_, &errorBuffer);
	COM_RELEASE(errorBuffer);
}

void Core::PreReset()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	quad_.reset();
	UnloadMountTextures();
	COM_RELEASE(mainEffect_);
}

void Core::PostReset(IDirect3DDevice9* dev, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	screenWidth_ = pPresentationParameters->BackBufferWidth;
	screenHeight_ = pPresentationParameters->BackBufferHeight;

	ImGui_ImplDX9_CreateDeviceObjects();
	ID3DXBuffer* errorBuffer = nullptr;
	D3DXCreateEffectFromResource(dev, dllModule_, MAKEINTRESOURCE(IDR_SHADER), nullptr, nullptr, 0, nullptr, &mainEffect_, &errorBuffer);
	COM_RELEASE(errorBuffer);
	LoadMountTextures(dev);
	try { quad_ = std::make_unique<UnitQuad>(dev); }
	catch (...) { quad_ = nullptr; }
}

void Core::DrawOver(IDirect3DDevice9* dev, bool FrameDrawn, bool SceneEnded)
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

			if (ImGui::Checkbox("Reset cursor to center with Center Locked keybind", &Cfg.ResetCursorOnLockedKeybind()))
				Cfg.ResetCursorOnLockedKeybindSave();
			if (ImGui::Checkbox("Lock camera when overlay is displayed", &Cfg.LockCameraWhenOverlayed()))
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

			if (!Cfg.LastSaveError().empty() && !ImGui::IsPopupOpen("Configuration Update Error") && Cfg.LastSaveErrorChanged())
				ImGui::OpenPopup("Configuration Update Error");

			if (ImGui::BeginPopup("Configuration Update Error"))
			{
				ImGui::Text("Could not save addon configuration. Reason given was:");
				ImGui::TextWrapped(Cfg.LastSaveError().c_str());
				if (ImGui::Button("OK"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
		}

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		if (DisplayMountOverlay && MainEffect && Quad)
		{
			Quad->Bind();

			auto currentTime = TimeInMilliseconds();

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
							mountDiameter *= Lerp(1.f, 1.1f, SmoothStep(hoverTimer));
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
					D3DXVECTOR4 spriteDimensions(io.MousePos.x * screenSize.z, io.MousePos.y * screenSize.w, 0.05f  * screenSize.y * screenSize.z, 0.05f);
					MainEffect->SetVector("g_vSpriteDimensions", &spriteDimensions);

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

}
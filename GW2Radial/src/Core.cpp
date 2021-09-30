#include <Core.h>
#include <Direct3D9Loader.h>
#include <imgui.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>
#include <Input.h>
#include <ConfigurationFile.h>
#include <UnitQuad.h>
#include <Wheel.h>
#include <Mount.h>
#include <SettingsMenu.h>
#include <Utility.h>
#include <imgui/imgui_internal.h>
#include <Novelty.h>
#include <shellapi.h>
#include <UpdateCheck.h>
#include <ImGuiPopup.h>
#include <Marker.h>
#include <MiscTab.h>
#include <MumbleLink.h>
#include <Effect.h>
#include <Effect_dx12.h>
#include <CustomWheel.h>
#include <GFXSettings.h>
#include <Log.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

LONG WINAPI GW2RadialTopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo);

namespace GW2Radial
{
void Core::Init(HMODULE dll)
{
#ifndef _DEBUG
	if(std::filesystem::exists(GetAddonFolder() / L"minidump.txt"))
#endif
	{
        // Install our own exception handler to automatically log minidumps.
        AddVectoredExceptionHandler(1, GW2RadialTopLevelFilter);
	    SetUnhandledExceptionFilter(GW2RadialTopLevelFilter);
	}

	i().dllModule_ = dll;
	i().InternalInit();
}

void Core::Shutdown()
{
	g_singletonManagerInstance.Shutdown();
}

Core::~Core()
{
	ImGui::DestroyContext();

	Direct3D9Inject::i([&](auto& i) {
		i.preCreateDeviceCallback = nullptr;
		i.postCreateDeviceCallback = nullptr;

		i.preResetCallback = nullptr;
		i.postResetCallback = nullptr;

		i.drawOverCallback = nullptr;
		i.drawUnderCallback = nullptr;
	});

	if(user32_)
		FreeLibrary(user32_);
}

void Core::OnInjectorCreated()
{
	auto& inject = Direct3D9Inject::i();
	
	inject.preCreateDeviceCallback = [this](HWND hWnd){ PreCreateDevice(hWnd); };
	inject.postCreateDeviceCallback = [this](IDirect3DDevice9* d, D3DPRESENT_PARAMETERS* pp){ PostCreateDevice(d, pp); };
	
	inject.preResetCallback = [this](){ PreReset(); };
	inject.postResetCallback = [this](IDirect3DDevice9* d, D3DPRESENT_PARAMETERS* pp){ PostReset(d, pp); };
	
	inject.drawOverCallback = [this](IDirect3DDevice9* d, bool frameDrawn, bool sceneEnded){ DrawOver(d, frameDrawn, sceneEnded); };
	inject.drawUnderCallback = [this](IDirect3DDevice9* d, bool frameDrawn, bool sceneEnded){ DrawUnder(d, frameDrawn, sceneEnded); };
}

void Core::OnInputLanguageChange()
{
	Log::i().Print(Severity::Info, "Input language change detected, reloading...");
	SettingsMenu::i().OnInputLanguageChange();

	for (auto* ilcl : ilcListeners_)
		ilcl->OnInputLanguageChange();
}

UINT Core::GetDpiForWindow(HWND hwnd)
{
	if (getDpiForWindow_)
		return getDpiForWindow_(hwnd);
	else
		return 96;
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

	user32_ = LoadLibrary(L"User32.dll");
	if(user32_)
		getDpiForWindow_ = (GetDpiForWindow_t)GetProcAddress(user32_, "GetDpiForWindow");
	
	imguiContext_ = ImGui::CreateContext();
}

void Core::OnFocusLost()
{
	for (auto& wheel : wheels_)
		if (wheel)
			wheel->OnFocusLost();

	Input::i().OnFocusLost();
}

void Core::OnFocus() {
	mainEffect_->Clear();

	Input::i().OnFocus();

	if(MiscTab::i().reloadOnFocus())
		forceReloadWheels_ = true;
}

LRESULT Core::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_KILLFOCUS)
		i().OnFocusLost();
	else if(msg == WM_SETFOCUS)
		i().OnFocus();
	else if(Input::i().OnInput(msg, wParam, lParam))
		return 0;

	// Whatever's left should be sent to the game
	return CallWindowProc(i().baseWndProc_, hWnd, msg, wParam, lParam);
}

void Core::PreCreateDevice(HWND hFocusWindow)
{
	gameWindow_ = hFocusWindow;

	// Hook WndProc
	if (!baseWndProc_)
	{
		baseWndProc_ = WNDPROC(GetWindowLongPtr(hFocusWindow, GWLP_WNDPROC));
		SetWindowLongPtr(hFocusWindow, GWLP_WNDPROC, LONG_PTR(&WndProc));
	}
}

void Core::PostCreateDevice(IDirect3DDevice9 *device, D3DPRESENT_PARAMETERS *presentationParameters)
{
	// Init ImGui
	auto &imio = ImGui::GetIO();
	imio.IniFilename = nullptr;
	imio.IniSavingRate = 1.0f;
	auto fontCfg = ImFontConfig();
	fontCfg.FontDataOwnedByAtlas = false;

	if(const auto data = LoadResource(IDR_FONT); data.data())
		font_ = imio.Fonts->AddFontFromMemoryTTF(data.data(), int(data.size_bytes()), 25.f, &fontCfg);
	if(const auto data = LoadResource(IDR_FONT_BLACK); data.data())
		fontBlack_ = imio.Fonts->AddFontFromMemoryTTF(data.data(), int(data.size_bytes()), 35.f, &fontCfg);
	if(const auto data = LoadResource(IDR_FONT_ITALIC); data.data())
		fontItalic_ = imio.Fonts->AddFontFromMemoryTTF(data.data(), int(data.size_bytes()), 25.f, &fontCfg);
	if(const auto data = LoadResource(IDR_FONT_DRAW); data.data())
		fontDraw_ = imio.Fonts->AddFontFromMemoryTTF(data.data(), int(data.size_bytes()), 100.f, &fontCfg);
	if (const auto data = LoadResource(IDR_FONT_MONO); data.data())
		fontMono_ = imio.Fonts->AddFontFromMemoryTTF(data.data(), int(data.size_bytes()), 18.f, &fontCfg);
	if(const auto data = LoadResource(IDR_FONT_ICON); data.data()) {
		fontCfg.GlyphMinAdvanceX = 25.f;
		static const ImWchar iconRange[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		fontIcon_ = imio.Fonts->AddFontFromMemoryTTF(data.data(), int(data.size_bytes()), 25.f, &fontCfg, iconRange);
	}

	if(font_)
		imio.FontDefault = font_;

	ImGui_ImplWin32_Init(gameWindow_);

	firstMessageShown_ = std::make_unique<ConfigurationOption<bool>>("", "first_message_shown_v1", "Core", false);
	ignoreRTSS_ = std::make_unique<ConfigurationOption<bool>>("", "ignore_rtss", "Core", false);

	if(!ignoreRTSS_->value())
	{
		const auto rtss = GetModuleHandleA("RTSSHooks64.dll");
		if(rtss)
		{
			const auto retval = MessageBox(nullptr, TEXT("WARNING: RivaTuner Statistics Server has been detected! GW2Radial is incompatible with RTSS, anomalous behavior may occur. Are you sure you want to continue? Continuing will prevent this message from showing again."), TEXT("RTSS Detected"), MB_ICONWARNING | MB_YESNO);
			if(retval == IDNO)
				exit(1);
			else if(retval == IDYES)
				ignoreRTSS_->value(true);
		}
	}

	OnDeviceSet(device, presentationParameters);
}

void Core::OnDeviceSet(IDirect3DDevice9 *device, D3DPRESENT_PARAMETERS *presentationParameters)
{
	// Initialize graphics
	screenWidth_ = presentationParameters->BackBufferWidth;
	screenHeight_ = presentationParameters->BackBufferHeight;
	firstFrame_ = true;

	try { quad_ = std::make_unique<UnitQuad>(device); }
	catch (...) { quad_ = nullptr; }

	//megai2: check for d912pxy
	//D3DRS_ENABLE_D912PXY_API_HACKS == 220
	if (IsD912Pxy(device))
		mainEffect_ = new Effect_dx12(device);
	else 
		mainEffect_ = new Effect(device);

	UpdateCheck::i().CheckForUpdates();
	MiscTab::i();

	wheels_.emplace_back(Wheel::Create<Mount>(IDR_BG, IDR_WIPEMASK, "mounts", "Mounts", device));
	wheels_.emplace_back(Wheel::Create<Novelty>(IDR_BG, IDR_WIPEMASK, "novelties", "Novelties", device));
	wheels_.emplace_back(Wheel::Create<Marker>(IDR_BG, IDR_WIPEMASK, "markers", "Markers", device));
	wheels_.emplace_back(Wheel::Create<ObjectMarker>(IDR_BG, IDR_WIPEMASK, "object_markers", "Object Markers", device));

	ImGui_ImplDX9_Init(device);

	customWheels_ = std::make_unique<CustomWheelsManager>(wheels_, fontDraw_);
}

void Core::OnDeviceUnset()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	quad_.reset();
	customWheels_.reset();
	wheels_.clear();
	if (mainEffect_)
	{
		delete mainEffect_;
		mainEffect_ = NULL;
	}
}

void Core::PreReset()
{
	OnDeviceUnset();
}

void Core::PostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS *presentationParameters)
{
	OnDeviceSet(device, presentationParameters);
}

void Core::DrawUnder(IDirect3DDevice9* device, bool frameDrawn, bool sceneEnded)
{
	if (!firstFrame_ && !frameDrawn)
	{
		if (sceneEnded)
			device->BeginScene();
		
		for (auto& wheel : wheels_)
			if (!wheel->drawOverUI())
				wheel->Draw(device, mainEffect_, quad_.get());

		if (sceneEnded)
			device->EndScene();
	}
}

void Core::OnUpdate()
{
    cref mumble = MumbleLink::i();

	uint map = mumble.mapId();
	if(map != mapId_)
	{
	    for (auto& wheel : wheels_)
		    wheel->OnMapChange(mapId_, map);

	    mapId_ = map;
	}

	if(characterName_ != mumble.characterName())
	{
	    for (auto& wheel : wheels_)
		    wheel->OnCharacterChange(characterName_, mumble.characterName());

		characterName_ = mumble.characterName();
	}
}


void Core::DrawOver(IDirect3DDevice9* device, bool frameDrawn, bool sceneEnded)
{
	// This is the closest we have to a reliable "update" function, so use it as one
	Input::i().OnUpdate();
	
	tickSkip_++;
	if(tickSkip_ >= TickSkipCount)
	{
	    tickSkip_ -= TickSkipCount;
		MumbleLink::i().OnUpdate();
	    OnUpdate();
	}

	longTickSkip_++;
	if(longTickSkip_ >= LongTickSkipCount)
	{
	    longTickSkip_ -= LongTickSkipCount;
	    ConfigurationFile::i().OnUpdate();
		GFXSettings::i().OnUpdate();
	}

	for (auto& wheel : wheels_)
		wheel->OnUpdate();

	UpdateCheck::i().CheckForUpdates();

	if (firstFrame_)
	{
		firstFrame_ = false;
	}
	else
	{
		// We have to use Present rather than hooking EndScene because the game seems to do final UI compositing after EndScene
		// This unfortunately means that we have to call Begin/EndScene before Present so we can render things, but thankfully for modern GPUs that doesn't cause bugs
		if (sceneEnded)
			device->BeginScene();

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		
		for (auto& wheel : wheels_)
			if (wheel->drawOverUI() || !frameDrawn)
				wheel->Draw(device, mainEffect_, quad_.get());
		
		customWheels_->Draw(device);

		SettingsMenu::i().Draw();
		Log::i().Draw();

		if(!firstMessageShown_->value())
			ImGuiPopup("Welcome to GW2Radial!").Position({0.5f, 0.45f}).Size({0.35f, 0.2f}).Display([&](const ImVec2& windowSize)
			{
				ImGui::TextWrapped("Welcome to GW2Radial! This small addon shows a convenient, customizable radial menu overlay to select a mount or novelty on the fly for Guild Wars 2: Path of Fire. "
				"To begin, use the shortcut Shift+Alt+M to open the settings menu and take a moment to bind your keys. If you ever need further assistance, please visit "
				"this project's website at");
				
				ImGui::SetCursorPosX(windowSize.x * 0.1f);

				if(ImGui::Button("https://github.com/Friendly0Fire/GW2Radial", ImVec2(windowSize.x * 0.8f, ImGui::GetFontSize() * 1.3f)))
					ShellExecute(0, 0, L"https://github.com/Friendly0Fire/GW2Radial", 0, 0 , SW_SHOW );
			}, [&]() { firstMessageShown_->value(true); });

		if (!ConfigurationFile::i().lastSaveError().empty() && ConfigurationFile::i().lastSaveErrorChanged())
			ImGuiPopup("Configuration could not be saved!").Position({0.5f, 0.45f}).Size({0.35f, 0.2f}).Display([&](const ImVec2&)
			{
				ImGui::Text("Could not save addon configuration. Reason given was:");
				ImGui::TextWrapped(ConfigurationFile::i().lastSaveError().c_str());
			}, []() { ConfigurationFile::i().lastSaveErrorChanged(false); });

		if(UpdateCheck::i().updateAvailable() && !UpdateCheck::i().updateDismissed())
			ImGuiPopup("Update available!").Position({0.5f, 0.45f}).Size({0.35f, 0.2f}).Display([&](const ImVec2& windowSize)
			{
				ImGui::TextWrapped("A new version of GW2Radial has been released! "
					"Please follow the link below to look at the changes and download the update. "
					"Remember that you can always disable this version check in the settings.");
				
				ImGui::SetCursorPosX(windowSize.x * 0.1f);

				if(ImGui::Button("https://github.com/Friendly0Fire/GW2Radial/releases/latest", ImVec2(windowSize.x * 0.8f, ImGui::GetFontSize() * 1.3f)))
					ShellExecute(0, 0, L"https://github.com/Friendly0Fire/GW2Radial/releases/latest", 0, 0 , SW_SHOW );
			}, []() { UpdateCheck::i().updateDismissed(true); });

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

        customWheels_->DrawOffscreen(device);

		if (sceneEnded)
			device->EndScene();
	}
	
	if(forceReloadWheels_)
	{
	    forceReloadWheels_ = false;
	    customWheels_->MarkReload();
	}
}

}

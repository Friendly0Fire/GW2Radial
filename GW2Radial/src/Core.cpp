#include <Core.h>
#include <Direct3D11Loader.h>
#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <Input.h>
#include <ConfigurationFile.h>
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
#include <ShaderManager.h>
#include <CustomWheel.h>
#include <GFXSettings.h>
#include <Log.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <Version.h>

LONG WINAPI GW2RadialTopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo);

namespace GW2Radial
{
void Core::Init(HMODULE dll)
{
	Log::i().Print(Severity::Info, "This is GW2Radial {}", GW2RADIAL_VER);

#ifndef _DEBUG
	if(auto addonFolder = GetAddonFolder(); addonFolder && std::filesystem::exists(*addonFolder / L"minidump.txt"))
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

	COM_RELEASE(device_);
	COM_RELEASE(context_);

	Direct3D11Inject::i([&](auto& i) {
		i.prePresentSwapChainCallback = nullptr;
		i.postCreateDeviceCallback = nullptr;
		i.preCreateSwapChainCallback = nullptr;
		i.postCreateSwapChainCallback = nullptr;
	});

	if(user32_)
		FreeLibrary(user32_);
}

void Core::OnInjectorCreated()
{
	auto& inject = Direct3D11Inject::i();

	inject.preCreateSwapChainCallback = [this](HWND hWnd) { PreCreateSwapChain(hWnd); };
	inject.postCreateSwapChainCallback = [this](IDXGISwapChain* swc) { PostCreateSwapChain(swc); };
	inject.postCreateDeviceCallback = [this](ID3D11Device* device){ PostCreateDevice(device); };
	
	inject.prePresentSwapChainCallback = [this](){ Draw(); };
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
	ShaderManager::i().ReloadAll();

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

void Core::PreCreateSwapChain(HWND hwnd)
{
	gameWindow_ = hwnd;

	// Hook WndProc
	if (!baseWndProc_)
	{
		baseWndProc_ = WNDPROC(GetWindowLongPtr(hwnd, GWLP_WNDPROC));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, LONG_PTR(&WndProc));
	}
}

void Core::PostCreateSwapChain(IDXGISwapChain* swc)
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

	DXGI_SWAP_CHAIN_DESC desc;
	swc->GetDesc(&desc);

	screenWidth_ = desc.BufferDesc.Width;
	screenHeight_ = desc.BufferDesc.Height;
}

void Core::PostCreateDevice(ID3D11Device* device)
{
	// Initialize graphics
	device_ = device;
	device_->AddRef();
	device->GetImmediateContext(&context_);

	firstFrame_ = true;

	ShaderManager::i(std::make_unique<ShaderManager>(device_));

	UpdateCheck::i().CheckForUpdates();
	MiscTab::i();

	wheels_.emplace_back(Wheel::Create<Mount>(IDR_BG, IDR_WIPEMASK, "mounts", "Mounts", device_));
	wheels_.emplace_back(Wheel::Create<Novelty>(IDR_BG, IDR_WIPEMASK, "novelties", "Novelties", device_));
	wheels_.emplace_back(Wheel::Create<Marker>(IDR_BG, IDR_WIPEMASK, "markers", "Markers", device_));
	wheels_.emplace_back(Wheel::Create<ObjectMarker>(IDR_BG, IDR_WIPEMASK, "object_markers", "Object Markers", device_));

	ImGui_ImplDX11_Init(device_, context_);

	customWheels_ = std::make_unique<CustomWheelsManager>(wheels_, fontDraw_);
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


void Core::Draw()
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
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		
		for (auto& wheel : wheels_)
			wheel->Draw(context_);
		
		customWheels_->Draw(context_);

		SettingsMenu::i().Draw();
		Log::i().Draw();

		if(!firstMessageShown_->value())
			ImGuiPopup("Welcome to GW2Radial!").Position({0.5f, 0.45f}).Size({0.35f, 0.2f}).Display([&](const ImVec2& windowSize)
			{
				ImGui::TextWrapped("Welcome to GW2Radial! This small addon shows a convenient, customizable radial menu overlay to select a mount or novelty on the fly for Guild Wars 2: Path of Fire. "
				"To begin, use the shortcut Shift+Alt+M to open the settings menu and take a moment to bind your keys. If you ever need further assistance, please visit "
				"this project's website at");

				ImGui::Spacing();
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

				ImGui::Spacing();
				ImGui::SetCursorPosX(windowSize.x * 0.1f);

				if(ImGui::Button("https://github.com/Friendly0Fire/GW2Radial/releases/latest", ImVec2(windowSize.x * 0.8f, ImGui::GetFontSize() * 1.3f)))
					ShellExecute(0, 0, L"https://github.com/Friendly0Fire/GW2Radial/releases/latest", 0, 0 , SW_SHOW );
			}, []() { UpdateCheck::i().updateDismissed(true); });

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        customWheels_->DrawOffscreen(device_, context_);
	}
	
	if(forceReloadWheels_)
	{
	    forceReloadWheels_ = false;
	    customWheels_->MarkReload();
	}
}

}

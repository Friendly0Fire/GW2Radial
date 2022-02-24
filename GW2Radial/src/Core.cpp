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
		i.postCreateSwapChainCallback = nullptr;
	});

	if(user32_)
		FreeLibrary(user32_);
}

void Core::OnInjectorCreated()
{
	auto& inject = Direct3D11Inject::i();

	inject.postCreateSwapChainCallback = [this](HWND hwnd, ID3D11Device* dev, IDXGISwapChain* swc) { PostCreateSwapChain(hwnd, dev, swc); };
	inject.prePresentSwapChainCallback = [this]() { Draw(); };
	inject.preResizeSwapChainCallback = [this]() { PreResizeSwapChain(); };
	inject.postResizeSwapChainCallback = [this](uint w, uint h) { PostResizeSwapChain(w, h); };
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

void Core::PreResizeSwapChain()
{
	backBufferRTV_.Reset();
}

void Core::PostResizeSwapChain(uint w, uint h)
{
	screenWidth_ = w;
	screenHeight_ = h;

	ComPtr<ID3D11Texture2D> backbuffer;
	swc_->GetBuffer(0, IID_PPV_ARGS(backbuffer.GetAddressOf()));
	device_->CreateRenderTargetView(backbuffer.Get(), nullptr, backBufferRTV_.ReleaseAndGetAddressOf());
}

void Core::PostCreateSwapChain(HWND hwnd, ID3D11Device* device, IDXGISwapChain* swc)
{
	gameWindow_ = hwnd;

	// Hook WndProc
	if (!baseWndProc_)
	{
		baseWndProc_ = WNDPROC(GetWindowLongPtr(hwnd, GWLP_WNDPROC));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, LONG_PTR(&WndProc));
	}

	device_ = device;
	device_->GetImmediateContext(&context_);
	swc_ = swc;

	context_->QueryInterface(annotations_.ReleaseAndGetAddressOf());

	ComPtr<ID3D11Texture2D> backbuffer;
	swc_->GetBuffer(0, IID_PPV_ARGS(backbuffer.GetAddressOf()));
	device_->CreateRenderTargetView(backbuffer.Get(), nullptr, backBufferRTV_.GetAddressOf());

	DXGI_SWAP_CHAIN_DESC desc;
	swc_->GetDesc(&desc);

	screenWidth_ = desc.BufferDesc.Width;
	screenHeight_ = desc.BufferDesc.Height;

	firstFrame_ = true;

	ShaderManager::i(std::make_unique<ShaderManager>(device_));

	UpdateCheck::i().CheckForUpdates();
	MiscTab::i();

	wheels_.emplace_back(Wheel::Create<Mount>(IDR_BG, IDR_WIPEMASK, "mounts", "Mounts", device_));
	wheels_.emplace_back(Wheel::Create<Novelty>(IDR_BG, IDR_WIPEMASK, "novelties", "Novelties", device_));
	wheels_.emplace_back(Wheel::Create<Marker>(IDR_BG, IDR_WIPEMASK, "markers", "Markers", device_));
	wheels_.emplace_back(Wheel::Create<ObjectMarker>(IDR_BG, IDR_WIPEMASK, "object_markers", "Object Markers", device_));

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
	ImGui_ImplDX11_Init(device_, context_);

	customWheels_ = std::make_unique<CustomWheelsManager>(wheels_, fontDraw_);

	firstMessageShown_ = std::make_unique<ConfigurationOption<bool>>("", "first_message_shown_v1", "Core", false);

#ifdef _DEBUG
	ID3D11Debug* d3dDebug = nullptr;
	if (SUCCEEDED(device_->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
	{
		ID3D11InfoQueue* d3dInfoQueue = nullptr;
		if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
		{
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
			//d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);

			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
			};

			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = std::size(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
			d3dInfoQueue->Release();
		}
		d3dDebug->Release();
	}
#endif
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
	if (annotations_)
		annotations_->BeginEvent(L"GW2Radial");

	StateBackupD3D11 d3dstate;
	BackupD3D11State(context_, d3dstate);

	context_->OMSetRenderTargets(1, backBufferRTV_.GetAddressOf(), nullptr);

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

		// Setup viewport
		D3D11_VIEWPORT vp;
		memset(&vp, 0, sizeof(D3D11_VIEWPORT));
		vp.Width = screenWidth_;
		vp.Height = screenHeight_;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = vp.TopLeftY = 0;
		context_->RSSetViewports(1, &vp);

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

	RestoreD3D11State(context_, d3dstate);

	if(annotations_)
		annotations_->EndEvent();
}

}

#include <Core.h>
#include <Direct3D9Hooks.h>
#include <imgui.h>
#include <examples/imgui_impl_dx9.h>
#include <examples/imgui_impl_win32.h>
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

namespace GW2Radial
{
DEFINE_SINGLETON(Core);

void Core::Init(HMODULE dll)
{
	auto rtss = GetModuleHandleA("RTSSHooks64.dll");
	if(rtss)
	{
		MessageBox(nullptr, TEXT("ERROR: RivaTuner Statistics Server has been detected. GW2Radial is incompatible with RTSS. Please disable RTSS and try again."), TEXT("RTSS Detected"), MB_OK);
		exit(1);
	}

	i()->dllModule_ = dll;
	i()->InternalInit();
}

void Core::Shutdown()
{
	i_.reset();

	// We'll just leak a bunch of things and let the driver/OS take care of it, since we have no clean exit point
	// and calling FreeLibrary in DllMain causes deadlocks
}

Core::Core()
	: firstMessageShown_("", "first_message_shown_v1", "Core", false)
{
}

Core::~Core()
{
	ImGui::DestroyContext();

	if(auto i = Direct3D9Hooks::iNoInit(); i != nullptr)
	{
		i->preCreateDeviceCallback(nullptr);
		i->postCreateDeviceCallback(nullptr);
		
		i->preResetCallback(nullptr);
		i->postResetCallback(nullptr);
		
		i->drawOverCallback(nullptr);
		i->drawUnderCallback(nullptr);
	}
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
	Direct3D9Hooks::i()->drawUnderCallback([this](IDirect3DDevice9* d, bool frameDrawn, bool sceneEnded){ DrawUnder(d, frameDrawn, sceneEnded); });
	
	imguiContext_ = ImGui::CreateContext();
}

void Core::OnFocusLost()
{
	if(wheelMounts_) wheelMounts_->OnFocusLost();
	if(wheelNovelties_) wheelNovelties_->OnFocusLost();
	Input::i()->OnFocusLost();
}

LRESULT Core::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_KILLFOCUS)
		i()->OnFocusLost();
	else if(Input::i()->OnInput(msg, wParam, lParam))
		return 0;

	// Whatever's left should be sent to the game
	return CallWindowProc(i()->baseWndProc_, hWnd, msg, wParam, lParam);
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
	imio.IniFilename = ConfigurationFile::i()->imguiLocation();
	auto fontCfg = ImFontConfig();
	fontCfg.FontDataOwnedByAtlas = false;

	void *fontPtr, *fontBlackPtr, *fontItalicPtr;
	size_t fontSize, fontBlackSize, fontItalicSize;
	if(LoadFontResource(IDR_FONT, fontPtr, fontSize))
		font_ = imio.Fonts->AddFontFromMemoryTTF(fontPtr, int(fontSize), 25.f, &fontCfg);
	if(LoadFontResource(IDR_FONT_BLACK, fontBlackPtr, fontBlackSize))
		fontBlack_ = imio.Fonts->AddFontFromMemoryTTF(fontBlackPtr, int(fontBlackSize), 35.f, &fontCfg);
	if(LoadFontResource(IDR_FONT_ITALIC, fontItalicPtr, fontItalicSize))
		fontItalic_ = imio.Fonts->AddFontFromMemoryTTF(fontItalicPtr, int(fontItalicSize), 25.f, &fontCfg);

	if(font_)
		imio.FontDefault = font_;

	ImGui_ImplWin32_Init(gameWindow_);

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

	ID3DXBuffer *errorBuffer = nullptr;
	D3DXCreateEffectFromResource(device, dllModule_, MAKEINTRESOURCE(IDR_SHADER), nullptr, nullptr, 0, nullptr,
	                             &mainEffect_, &errorBuffer);
	COM_RELEASE(errorBuffer);

	UpdateCheck::i()->CheckForUpdates();

	wheelMounts_ = std::make_unique<Wheel>(IDR_BG, IDR_INK, "mounts", "Mounts", device);
	Mount::AddAllMounts(wheelMounts_.get(), device);

	wheelNovelties_ = std::make_unique<Wheel>(IDR_BG, IDR_INK, "novelties", "Novelties", device);
	Novelty::AddAllNovelties(wheelNovelties_.get(), device);

	ImGui_ImplDX9_Init(device);
}

void Core::OnDeviceUnset()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	quad_.reset();
	wheelMounts_.reset();
	wheelNovelties_.reset();
	COM_RELEASE(mainEffect_);
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
		
		if(!wheelMounts_->drawOverUI()) wheelMounts_->Draw(device, mainEffect_, quad_.get());
		if(!wheelNovelties_->drawOverUI()) wheelNovelties_->Draw(device, mainEffect_, quad_.get());

		if (sceneEnded)
			device->EndScene();
	}
}

void Core::DrawOver(IDirect3DDevice9* device, bool frameDrawn, bool sceneEnded)
{
	// This is the closest we have to a reliable "update" function, so use it as one
	Input::i()->OnUpdate();

	UpdateCheck::i()->CheckForUpdates();

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
		
		if(wheelMounts_->drawOverUI() || !frameDrawn) wheelMounts_->Draw(device, mainEffect_, quad_.get());
		if(wheelNovelties_->drawOverUI() || !frameDrawn) wheelNovelties_->Draw(device, mainEffect_, quad_.get());

		SettingsMenu::i()->Draw();

		if(!firstMessageShown_.value())
		{
			if(ImGui::Begin("Welcome to GW2Radial!", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
			{
				const auto size = ImVec2(screenWidth_ * 0.35f, screenHeight_ * 0.2f);
				const auto pos = ImVec2(0.5f * screenWidth_ - size.x / 2, 0.45f * screenHeight_ - size.y / 2);
				ImGui::SetWindowPos(pos);
				ImGui::SetWindowSize(size);
				ImGui::TextWrapped("Welcome to GW2Radial! This small addon shows a convenient, customizable radial menu overlay to select a mount or novelty on the fly for Guild Wars 2: Path of Fire. "
				"To begin, use the shortcut Shift+Alt+M to open the settings menu and take a moment to bind your keys. If you ever need further assistance, please visit "
				"this project's website at");
				
				ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.1f);

				if(ImGui::Button("https://github.com/Friendly0Fire/GW2Radial", ImVec2(ImGui::GetWindowSize().x * 0.8f, ImGui::GetFontSize() * 1.2f)))
					ShellExecute(0, 0, L"https://github.com/Friendly0Fire/GW2Radial", 0, 0 , SW_SHOW );
				
				ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.4f);
				ImGui::SetCursorPosY(ImGui::GetWindowSize().y - ImGui::GetFontSize() * 2.5f);
				if (ImGui::Button("OK", ImVec2(ImGui::GetWindowSize().x * 0.2f, ImGui::GetFontSize() * 1.5f)))
					firstMessageShown_.value(true);
			}
			ImGui::End();
		}

		if(UpdateCheck::i()->updateAvailable() && !UpdateCheck::i()->updateDismissed())
		{
			if(ImGui::Begin("Update available!", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
			{
				const auto size = ImVec2(screenWidth_ * 0.35f, screenHeight_ * 0.2f);
				const auto pos = ImVec2(0.5f * screenWidth_ - size.x / 2, 0.45f * screenHeight_ - size.y / 2);
				ImGui::SetWindowPos(pos);
				ImGui::SetWindowSize(size);
				ImGui::TextWrapped("A new version of GW2Radial has been released! Please follow the link below to look at the changes and download the update. "
				"Remember that you can always disable this version check in the settings.");
				
				ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.1f);

				if(ImGui::Button("https://github.com/Friendly0Fire/GW2Radial/releases/latest", ImVec2(ImGui::GetWindowSize().x * 0.8f, ImGui::GetFontSize() * 1.2f)))
					ShellExecute(0, 0, L"https://github.com/Friendly0Fire/GW2Radial/releases/latest", 0, 0 , SW_SHOW );
				
				ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.4f);
				ImGui::SetCursorPosY(ImGui::GetWindowSize().y - ImGui::GetFontSize() * 2.5f);
				if(ImGui::Button("OK", ImVec2(ImGui::GetWindowSize().x * 0.2f, ImGui::GetFontSize() * 1.5f)))
					UpdateCheck::i()->updateDismissed(true);
			}
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());	

		if (sceneEnded)
			device->EndScene();
	}
}

}

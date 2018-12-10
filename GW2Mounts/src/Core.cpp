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
#include "../imgui/imgui_internal.h"

namespace GW2Addons
{
DEFINE_SINGLETON(Core);

void Core::Init(HMODULE dll)
{
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
	Direct3D9Hooks::i()->drawUnderCallback([this](IDirect3DDevice9* d, bool frameDrawn, bool sceneEnded){ DrawOver(d, frameDrawn, sceneEnded); });
	
	ImGui::CreateContext();
}

void Core::OnFocusLost()
{
	wheelMounts_->OnFocusLost();
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
		font_ = imio.Fonts->AddFontFromMemoryTTF(fontPtr, fontSize, 25.f, &fontCfg);
	if(LoadFontResource(IDR_FONT_BLACK, fontBlackPtr, fontBlackSize))
		fontBlack_ = imio.Fonts->AddFontFromMemoryTTF(fontBlackPtr, fontBlackSize, 35.f, &fontCfg);
	if(LoadFontResource(IDR_FONT_ITALIC, fontItalicPtr, fontItalicSize))
		fontItalic_ = imio.Fonts->AddFontFromMemoryTTF(fontItalicPtr, fontItalicSize, 25.f, &fontCfg);

	if(font_)
		imio.FontDefault = font_;

	// Setup ImGui binding
	ImGui_ImplDX9_Init(device);
	ImGui_ImplWin32_Init(gameWindow_);

	OnDeviceSet(device, presentationParameters);
}

void Core::OnDeviceSet(IDirect3DDevice9 *device, D3DPRESENT_PARAMETERS *presentationParameters)
{
	// Initialize graphics
	screenWidth_ = presentationParameters->BackBufferWidth;
	screenHeight_ = presentationParameters->BackBufferHeight;
	try { quad_ = std::make_unique<UnitQuad>(device); }
	catch (...) { quad_ = nullptr; }
	firstFrame_ = true;

	ID3DXBuffer *errorBuffer = nullptr;
	D3DXCreateEffectFromResource(device, dllModule_, MAKEINTRESOURCE(IDR_SHADER), nullptr, nullptr, 0, nullptr,
	                             &mainEffect_, &errorBuffer);
	COM_RELEASE(errorBuffer);

	wheelMounts_ = std::make_unique<Wheel>(IDR_BG, "mounts", "Mounts", device);
	Mount::AddAllMounts(wheelMounts_.get(), device);
}

void Core::OnDeviceUnset()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	quad_.reset();
	wheelMounts_.reset();
	COM_RELEASE(mainEffect_);
}

void Core::PreReset()
{
	OnDeviceUnset();
}

void Core::PostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS *presentationParameters)
{
	ImGui_ImplDX9_CreateDeviceObjects();

	OnDeviceSet(device, presentationParameters);
}

void Core::DrawOver(IDirect3DDevice9* device, bool frameDrawn, bool sceneEnded)
{
	// This is the closest we have to a reliable "update" function, so use it as one
	Input::i()->OnUpdate();

	if (frameDrawn)
		return;

	if (!firstFrame_)
	{

		// We have to use Present rather than hooking EndScene because the game seems to do final UI compositing after EndScene
		// This unfortunately means that we have to call Begin/EndScene before Present so we can render things, but thankfully for modern GPUs that doesn't cause bugs
		if (sceneEnded)
			device->BeginScene();
		
		SettingsMenu::i()->Draw();

		if(!firstMessageShown_.value() && ImGui::Begin("Welcome to GW2Addons!", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
		{
			const auto size = ImVec2(screenWidth_ * 0.35f, screenHeight_ * 0.17f);
			const auto pos = ImVec2(0.5f * screenWidth_ - size.x / 2, 0.45f * screenHeight_ - size.y / 2);
			ImGui::SetWindowPos(pos);
			ImGui::SetWindowSize(size);
			ImGui::TextWrapped("Welcome to GW2Addons! This small addon provides a collection of features, chief among them the radial menu for mounts and novelties. "
			"To begin, use the shortcut Shift+Alt+M to open the settings menu and take a moment to bind your keys. If you ever need further assistance, please visit "
			"this project's website at https://github.com/Friendly0Fire/GW2Addons !");
			
			ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.4f);
			ImGui::SetCursorPosY(ImGui::GetWindowSize().y - ImGui::GetFontSize() * 2.5f);
			if (ImGui::Button("OK", ImVec2(ImGui::GetWindowSize().x * 0.2f, ImGui::GetFontSize() * 1.5f)))
				firstMessageShown_.value(true);

			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		wheelMounts_->Draw(device, mainEffect_, quad_.get());

		if (sceneEnded)
			device->EndScene();
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	firstFrame_ = false;
}

}

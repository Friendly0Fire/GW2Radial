#include <Core.h>
#include <Direct3D9Hooks.h>
#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_dx9.h>
#include <imgui/examples/imgui_impl_win32.h>
#include <Input.h>
#include <ConfigurationFile.h>
#include <UnitQuad.h>
#include <Wheel.h>
#include <Mount.h>
#include <SettingsMenu.h>

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

void Core::PostCreateDevice(IDirect3DDevice9 *device, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	// Init ImGui
	ImGui::CreateContext();
	auto &imio = ImGui::GetIO();
	imio.IniFilename = ConfigurationFile::i()->imguiLocation();

	// Setup ImGui binding
	ImGui_ImplDX9_Init(device);
	ImGui_ImplWin32_Init(gameWindow_);

	OnDeviceSet(device, pPresentationParameters);
}

void Core::OnDeviceSet(IDirect3DDevice9 *device, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	// Initialize graphics
	screenWidth_ = pPresentationParameters->BackBufferWidth;
	screenHeight_ = pPresentationParameters->BackBufferHeight;
	try { quad_ = std::make_unique<UnitQuad>(device); }
	catch (...) { quad_ = nullptr; }
	firstFrame_ = true;

	ID3DXBuffer *errorBuffer = nullptr;
	D3DXCreateEffectFromResource(device, dllModule_, MAKEINTRESOURCE(IDR_SHADER), nullptr, nullptr, 0, nullptr,
	                             &mainEffect_, &errorBuffer);
	COM_RELEASE(errorBuffer);

	wheelMounts_ = std::make_unique<Wheel>(IDR_BG, "mountsWheel", "Mounts", device);
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

void Core::PostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	ImGui_ImplDX9_CreateDeviceObjects();

	OnDeviceSet(device, pPresentationParameters);
}

void Core::DrawOver(IDirect3DDevice9* dev, bool frameDrawn, bool sceneEnded)
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
			dev->BeginScene();
		
		SettingsMenu::i()->Draw();

		wheelMounts_->Draw(dev, mainEffect_, quad_.get());

		if (sceneEnded)
			dev->EndScene();
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	firstFrame_ = false;
}

}
#include <Core.h>
#include <Direct3D9Hooks.h>
#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_dx9.h>
#include <imgui/examples/imgui_impl_win32.h>

namespace GW2Addons
{

void Core::Init(HMODULE dll)
{
	i()->dllModule_ = dll;
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

}

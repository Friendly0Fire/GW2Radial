#include <Core.h>
#include <Direct3D9Hooks.h>
#include <imgui/examples/imgui_impl_dx9.h>
#include <imgui/examples/imgui_impl_win32.h>

namespace GW2Addons
{

Core::~Core()
{
	Direct3D9Hooks::i()->preResetCallback()();
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
}

}
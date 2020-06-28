#include <Main.h>
#include <Core.h>
#include <Direct3D9Hooks.h>
#include "gw2al_api.h"

gw2al_addon_dsc gAddonDeps[] = {
	{
		L"loader_core",
		L"whatever",
		0,
		1,
		1,
		0
	},
	{
		L"d3d9_wrapper",
		L"Wrapper for d3d9 API that includes hooking and custom d3d9 loading",
		1,
		0,
		2,
		0
	},
	{0,0,0,0,0,0}
};

gw2al_addon_dsc gAddonDsc = {
	L"gw2radial",
	L"Radial menu overlay to select mount, novelty and more on fly",
	1,
	2,
	1,
	gAddonDeps
};

gw2al_addon_dsc* gw2addon_get_description()
{
	return &gAddonDsc;
}

gw2al_api_ret gw2addon_load(gw2al_core_vtable* core_api)
{
	GW2Radial::Direct3D9Hooks::i()->InitHooks(core_api);
	return GW2AL_OK;
}

gw2al_api_ret gw2addon_unload(int gameExiting)
{
	return GW2AL_OK;
}

bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
#ifdef _DEBUG
		while (!IsDebuggerPresent());
#endif
		GW2Radial::Core::Init(hModule);
		break;
	case DLL_PROCESS_DETACH:
		GW2Radial::Core::Shutdown();
		break;
	}

	return true;
}
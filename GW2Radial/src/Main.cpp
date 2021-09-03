#include <Main.h>
#include <Core.h>
#include <Direct3D9Loader.h>
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
	GW2Radial::Direct3D9Inject::i(std::make_unique<GW2Radial::Direct3D9Loader>());
    GW2Radial::Core::i().OnInjectorCreated();

	GW2Radial::GetD3D9Loader()->Init(core_api);
	return GW2AL_OK;
}

gw2al_api_ret gw2addon_unload(int gameExiting)
{
	return GW2AL_OK;
}

IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion)
{
	GW2Radial::Direct3D9Inject::i(std::move(std::make_unique<GW2Radial::Direct3D9Hooks>()));
    GW2Radial::Core::i().OnInjectorCreated();

	return GW2Radial::GetD3D9Hooks()->Direct3DCreate9(SDKVersion);
}

HRESULT WINAPI Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** d3d9ex)
{
	GW2Radial::Direct3D9Inject::i(std::move(std::make_unique<GW2Radial::Direct3D9Hooks>()));
    GW2Radial::Core::i().OnInjectorCreated();

	return GW2Radial::GetD3D9Hooks()->Direct3DCreate9Ex(SDKVersion, d3d9ex);
}

std::ofstream g_logStream;

std::ofstream& GetLogStream() { return g_logStream; }

bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	    g_logStream = std::ofstream("gw2radial.log");

		GW2Radial::Core::Init(hModule);
		break;
	case DLL_PROCESS_DETACH:
		GW2Radial::Core::Shutdown();
		break;
	}

	return true;
}
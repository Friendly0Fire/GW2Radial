#include <Main.h>
#include <Core.h>
#include <Direct3D11Loader.h>
#include <gw2al_api.h>
#include <gw2al_d3d9_wrapper.h>
#include <Tag.h>
#include <Version.h>

const char* GetAddonName() { return "GW2Radial"; }
const wchar_t* GetAddonNameW() { return L"GW2Radial"; }
const char* GetAddonVersionString() { return GW2RADIAL_VER; }
const semver::version& GetAddonVersion() { return CurrentVersion; }
BaseCore& GetBaseCore()
{
	return GW2Radial::Core::i();
}

gw2al_addon_dsc gAddonDeps[] = {
	GW2AL_CORE_DEP_ENTRY,
	D3D_WRAPPER_DEP_ENTRY,
	{0,0,0,0,0,0}
};

gw2al_addon_dsc gAddonDsc = {
	L"gw2radial",
	L"Radial menu overlay to select mount, novelty and more on fly",
	2,
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
	Direct3D11Loader::reset();
	Direct3D11Loader::i().Init(core_api);
	return GW2AL_OK;
}

gw2al_api_ret gw2addon_unload(int gameExiting)
{
	GW2Radial::Core::Shutdown();
	return GW2AL_OK;
}

std::ofstream g_logStream;


bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	    g_logStream = std::ofstream("gw2radial.log");

		GW2Radial::Core::Init(hModule);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return true;
}
#include <Main.h>
#include <Core.h>
#include <Direct3D9Loader.h>
#include <Direct3D9Hooks.h>
#include <gw2al_api.h>
#include <gw2al_d3d9_wrapper.h>

gw2al_addon_dsc gAddonDeps[] = {
	GW2AL_CORE_DEP_ENTRY,
	D3D_WRAPPER_DEP_ENTRY,
	{0,0,0,0,0,0}
};

gw2al_addon_dsc gAddonDsc = {
	L"gw2radial",
	L"Radial menu overlay to select mount, novelty and more on fly",
	2,
	1,
	1,
	gAddonDeps
};

gw2al_addon_dsc* gw2addon_get_description()
{
	return &gAddonDsc;
}

gw2al_api_ret gw2addon_load(gw2al_core_vtable* core_api)
{
	GW2Radial::Direct3D9Inject::reset();
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

namespace GW2Radial
{
SingletonManager g_singletonManagerInstance;

inline BaseSingleton* BaseSingleton::Store(std::unique_ptr<BaseSingleton>&& ptr)
{
	g_singletonManagerInstance.singletons_.push(std::move(ptr));
	return g_singletonManagerInstance.singletons_.top().get();
}

inline void BaseSingleton::Clear(BaseSingleton* clearPtr)
{
	std::stack<std::unique_ptr<BaseSingleton>> singletons;

	while (!g_singletonManagerInstance.singletons_.empty()) {
		auto ptr = std::move(g_singletonManagerInstance.singletons_.top());
		g_singletonManagerInstance.singletons_.pop();
		if (ptr.get() != clearPtr)
			singletons.push(std::move(ptr));
	}

	std::swap(singletons, g_singletonManagerInstance.singletons_);
}
}


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
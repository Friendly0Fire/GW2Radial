#include <Main.h>
#include <Core.h>
#include <Direct3D11Loader.h>
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
	GW2Radial::Direct3D11Inject::reset();
	GW2Radial::Direct3D11Inject::i(std::make_unique<GW2Radial::Direct3D11Loader>());
    GW2Radial::Core::i().OnInjectorCreated();

	GW2Radial::GetD3D11Loader()->Init(core_api);
	return GW2AL_OK;
}

gw2al_api_ret gw2addon_unload(int gameExiting)
{
	GW2Radial::Core::Shutdown();
	return GW2AL_OK;
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
		break;
	}

	return true;
}
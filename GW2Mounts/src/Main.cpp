#include <Main.h>
#include <Core.h>
#include <Direct3D9Hooks.h>

IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion)
{
	return GW2Addons::Direct3D9Hooks::i()->Direct3DCreate9(SDKVersion);
}

HRESULT WINAPI Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** d3d9ex)
{
	return GW2Addons::Direct3D9Hooks::i()->Direct3DCreate9Ex(SDKVersion, d3d9ex);
}

bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
#ifdef _DEBUG
		while (!IsDebuggerPresent());
#endif
		GW2Addons::Core::Init(hModule);
		break;
	case DLL_PROCESS_DETACH:
		GW2Addons::Core::Shutdown();
		break;
	}

	return true;
}
#include <Main.h>
#include <Core.h>
#include <Direct3D9Hooks.h>

IDirect3D9* Direct3DCreate9(UINT SDKVersion)
{
	return GW2Addons::Direct3D9Hooks::i()->Direct3DCreate9(SDKVersion);
}

IDirect3D9* Direct3DCreate9Ex(UINT SDKVersion)
{
	return GW2Addons::Direct3D9Hooks::i()->Direct3DCreate9Ex(SDKVersion);
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
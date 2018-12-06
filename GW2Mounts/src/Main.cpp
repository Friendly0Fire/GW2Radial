#include <Main.h>
#include <Core.h>

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
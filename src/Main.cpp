#include <Core.h>
#include <Direct3D11Loader.h>
#include <Main.h>
#include <Version.h>
#include <gw2al_api.h>
#include <gw2al_d3d9_wrapper.h>

const char* GetAddonName()
{
    return ADDON_NAME;
}

const wchar_t* GetAddonNameW()
{
    return TEXT(ADDON_NAME);
}

const char* GetAddonVersionString()
{
    return GIT_VER_STR;
}

u64 GetAddonVersion()
{
    static std::array<u64, 4> ver = { GIT_VER };
    return (ver[0] << 16 * 3) + (ver[1] << 16 * 2) + (ver[2] << 16 * 1) + (ver[3] << 16 * 0);
}

BaseCore& GetBaseCore()
{
    return GW2Radial::Core::i();
}

gw2al_addon_dsc gAddonDeps[] = {
    GW2AL_CORE_DEP_ENTRY, D3D_WRAPPER_DEP_ENTRY, {0, 0, 0, 0, 0, 0}
};


gw2al_addon_dsc* gw2addon_get_description()
{
    static gw2al_addon_dsc addonDesc = []
    {
        std::array<u64, 4> ver = { GIT_VER };
        gw2al_addon_dsc    d{ L"gw2radial", L"Radial menu overlay to select mount, novelty and more on fly", 0, 0, 0, gAddonDeps };
        d.majorVer = (unsigned char)ver[0];
        d.minorVer = (unsigned char)ver[1];
        d.revision = (unsigned int)ver[2];
        return d;
    }();
    return &addonDesc;
}

gw2al_api_ret gw2addon_load(gw2al_core_vtable* core_api)
{
    Direct3D11Loader::reset();
    Direct3D11Loader::i().Init(core_api);
    return GW2AL_OK;
}

gw2al_api_ret gw2addon_unload(int gameExiting)
{
    return GW2AL_OK;
}

std::ofstream g_logStream;


bool WINAPI   DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
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
#include <Core.h>
#include <Main.h>
#include <Tag.h>
#include <Version.h>

const char* GetAddonName()
{
    return "GW2Radial";
}
const wchar_t* GetAddonNameW()
{
    return L"GW2Radial";
}
const char* GetAddonVersionString()
{
    return GW2RADIAL_VER;
}
const semver::version& GetAddonVersion()
{
    return CurrentVersion;
}
BaseCore& GetBaseCore()
{
    return GW2Radial::Core::i();
}

#include "common/../src/Main.inl"
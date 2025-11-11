#include <Core.h>
#include <Main.h>
#include "Version.h"

BaseCore& GetBaseCore()
{
    return GW2Radial::Core::i();
}

#include <Main.inl>

const cpp_util::cstring_view  AddonName          = "GW2Radial";
const cpp_util::wcstring_view AddonNameW         = L"GW2Radial";
const cpp_util::cstring_view  AddonVersionString = GIT_VER_STR;
const u64                     AddonVersion       = GetAddonVersion();

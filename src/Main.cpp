#include <Core.h>
#include <Main.h>
#include "Version.h"

BaseCore& GetBaseCore()
{
    return GW2Radial::Core::i();
}

#include <Main.inl>
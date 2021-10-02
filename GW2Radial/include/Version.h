#pragma once

#ifdef USE_GITHUB_VER
#include "github.h"
#else
#include "git.h"

#ifndef STRINGIFY
#define STRINGIFY(v) #v
#endif

#ifndef XSTRINGIFY
#define XSTRINGIFY(v) STRINGIFY(v)
#endif

#define GW2RADIAL_VER "nightly (" XSTRINGIFY(GIT_HASH) ")"
#endif
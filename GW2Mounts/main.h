#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>
#include <string>
#include <memory>

#define COM_RELEASE(x) { if((x)) { (x)->Release(); (x) = nullptr; } }

#include "resource.h"

typedef unsigned char uchar;
typedef unsigned int uint;
typedef std::basic_string<TCHAR> tstring;
typedef unsigned __int64 mstime;
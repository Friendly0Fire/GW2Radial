#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <string>
#include <memory>

#include <resource.h>
#include <ConfigurationFile.h>

#define COM_RELEASE(x) { if((x)) { (x)->Release(); (x) = nullptr; } }
#define NULL_COALESCE(a, b) ((a) != nullptr ? (a) : (b))
#define SQUARE(x) ((x) * (x))

typedef unsigned char uchar;
typedef unsigned int uint;
typedef std::basic_string<TCHAR> tstring;
typedef unsigned __int64 mstime;

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

namespace GW2Addons
{
extern HWND GameWindow;
extern HMODULE DllModule;
}
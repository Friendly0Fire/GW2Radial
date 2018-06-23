#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>
#include <string>
#include <memory>

#define COM_RELEASE(x) { if((x)) { (x)->Release(); (x) = nullptr; } }

#define NULL_COALESCE(a, b) ((a) != nullptr ? (a) : (b))

#include "resource.h"

typedef unsigned char uchar;
typedef unsigned int uint;
typedef std::basic_string<TCHAR> tstring;
typedef unsigned __int64 mstime;

enum class MountType
{
	NONE = -1,
	RAPTOR = 0,
	SPRINGER = 1,
	SKIMMER = 2,
	JACKAL = 3,
	GRIFFON = 4
	//BEETLE = 4,
	//GRIFFON = 5
};
const unsigned int MountTypeCount = 5;

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

extern HWND GameWindow;
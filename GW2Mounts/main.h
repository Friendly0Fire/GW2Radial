#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>
#include <string>
#include <memory>
#include <array>

#define COM_RELEASE(x) { if((x)) { (x)->Release(); (x) = nullptr; } }

#define NULL_COALESCE(a, b) ((a) != nullptr ? (a) : (b))

#define SQUARE(x) ((x) * (x))

#include "resource.h"

typedef unsigned char uchar;
typedef unsigned int uint;
typedef std::basic_string<TCHAR> tstring;
typedef unsigned __int64 mstime;

enum class MountType : uint
{
	NONE = (uint)-1,
	RAPTOR = 0,
	SPRINGER = 1,
	SKIMMER = 2,
	JACKAL = 3,
	BEETLE = 4,
	GRIFFON = 5
};
const unsigned int MountTypeCount = 6;

const std::array<float, 4> MountColors[MountTypeCount] = {
	{ 213 / 255.f, 100 / 255.f, 89 / 255.f, 1 },
	{ 212 / 255.f, 198 / 255.f, 94 / 255.f, 1 },
	{ 108 / 255.f, 128 / 255.f, 213 / 255.f, 1 },
	{ 120 / 255.f, 183 / 255.f, 197 / 255.f, 1 },
	{ 0, 0, 0, 1 },
	{ 136 / 255.f, 123 / 255.f, 195 / 255.f, 1 }
};

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

extern HWND GameWindow;
#pragma once
#include <Win.h>

#include <vector>
#include <string>
#include <memory>
#include <span>
#include <wrl.h>
#include <d3d9.h>
#include <fstream>
#include <filesystem>

#include <Resource.h>
#include <Log.h>

#define NULL_COALESCE(a, b) ((a) != nullptr ? (a) : (b))
#define OPT_COALESCE(a, b) ((a) ? (a) : (b))
#define SQUARE(x) ((x) * (x))

template <typename... Args>
void FormattedMessageBox(const wchar_t* contents, const wchar_t* title, Args&&...args) {
    wchar_t buf[2048];
    swprintf_s(buf, contents, std::forward<Args>(args)...);

    MessageBoxW(nullptr, buf, title, MB_ICONERROR | MB_OK);
}

template <typename... Args>
void CriticalMessageBox(const wchar_t* contents, Args&&...args) {
    FormattedMessageBox(contents, L"GW2Radial Fatal Error", std::forward<Args>(args)...);
    exit(1);
}

#define GW2_ASSERT(test) GW2Assert(test, L#test)

__forceinline void GW2Assert(bool test, const wchar_t* testText) {
    if (test)
        return;

    if (IsDebuggerPresent())
        __debugbreak();
    else
        CriticalMessageBox(L"Assertion failure: \"%s\"!", testText);
}

using Microsoft::WRL::ComPtr;

typedef unsigned char            uchar;
typedef unsigned int             uint;
typedef unsigned short           ushort;
typedef std::basic_string<TCHAR> tstring;
typedef unsigned __int64         mstime;

#define cref const auto&
using std::tie;

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

#ifndef M_PI
#define M_PI 3.14159265359
#endif

#ifdef _DEBUG
#define HOT_RELOAD_SHADERS
#endif

typedef struct fVector4 {
    float x;
    float y;
    float z;
    float w;
}         fVector4;

typedef struct fVector3 {
    float x;
    float y;
    float z;
}         fVector3;

typedef struct fVector2 {
    float x;
    float y;
}         fVector2;

typedef struct iVector4 {
    int x;
    int y;
    int z;
    int w;
}       iVector4;

typedef struct iVector3 {
    int x;
    int y;
    int z;
}       iVector3;

typedef struct iVector2 {
    int x;
    int y;
}       iVector2;

bool ExceptionHandlerMiniDump(
    struct _EXCEPTION_POINTERS* pExceptionInfo, const char* function, const char* file, int line);

#include <Enums.h>
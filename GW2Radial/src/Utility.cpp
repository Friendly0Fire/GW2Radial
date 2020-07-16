#include <Main.h>
#include <Utility.h>
#include <d3d9types.h>
#include <Core.h>
#include <winuser.h>
#include "DDSTextureLoader.h"
#include <iostream>
#include <sstream>

namespace GW2Radial
{

std::string utf8_encode(const std::wstring &wstr)
{
    if( wstr.empty() ) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring utf8_decode(const std::string &str)
{
    if( str.empty() ) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar                  (CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::wstring GetScanCodeName(uint scanCode) {
	if (scanCode >= 2 && scanCode <= 10) {
		wchar_t c = scanCode - 1 + 0x30;
		return std::wstring(1, c);
	}
	if (scanCode == 11)
		return L"0";

	if (IsUniversalModifier(ScanCode(scanCode))) {
		switch (scanCode) {
		case ScanCode_t(ScanCode::SHIFT):
			return L"SHIFT";
		case ScanCode_t(ScanCode::CONTROL):
			return L"CONTROL";
		case ScanCode_t(ScanCode::ALT):
			return L"ALT";
		case ScanCode_t(ScanCode::META):
			return L"META";
		}
	}

	wchar_t keyName[50];
	if (GetKeyNameTextW(scanCode << 16, keyName, sizeof(keyName)) != 0)
		return keyName;

	return L"[Error]";
}

std::wstring GetKeyName(uint virtualKey)
{
	uint scanCode = MapVirtualKeyW(virtualKey, MAPVK_VK_TO_VSC);

	switch (virtualKey)
	{
	case VK_LBUTTON:
		return L"M1";
	case VK_RBUTTON:
		return L"M2";
	case VK_MBUTTON:
		return L"M3";
	case VK_XBUTTON1:
		return L"M4";
	case VK_XBUTTON2:
		return L"M5";
	case VK_F13:
		return L"F13";
	case VK_F14:
		return L"F14";
	case VK_F15:
		return L"F15";
	case VK_F16:
		return L"F16";
	case VK_F17:
		return L"F17";
	case VK_F18:
		return L"F18";
	case VK_F19:
		return L"F19";
	case VK_F20:
		return L"F20";
	case VK_F21:
		return L"F21";
	case VK_F22:
		return L"F22";
	case VK_F23:
		return L"F23";
	case VK_F24:
		return L"F24";
	case VK_LCONTROL:
		return L"LCTRL";
	case VK_RCONTROL:
		return L"RCTRL";
	case VK_LSHIFT:
		return L"LSHIFT";
	case VK_RSHIFT:
		return L"RSHIFT";
	case VK_LMENU:
		return L"LALT";
	case VK_RMENU:
		return L"RALT";
	// because MapVirtualKey strips the extended bit for some keys
	case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
	case VK_PRIOR: case VK_NEXT: // page up and page down
	case VK_END: case VK_HOME:
	case VK_INSERT: case VK_DELETE:
	case VK_DIVIDE: // numpad slash
	case VK_NUMLOCK:
		scanCode |= 0x100; // set extended bit
		break;
	default:
		break;
	}

	return GetScanCodeName(scanCode);
}

void SplitFilename(const tstring& str, tstring* folder, tstring* file)
{
	const auto found = str.find_last_of(TEXT("/\\"));
	if (folder) *folder = str.substr(0, found);
	if (file) *file = str.substr(found + 1);
}

mstime TimeInMilliseconds()
{
	mstime iCount;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&iCount));
	mstime iFreq;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&iFreq));
	return 1000 * iCount / iFreq;
}

bool FileExists(const TCHAR* path)
{
	const auto dwAttrib = GetFileAttributes(path);

	return dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

bool ShaderIsEnd(DWORD token)
{
	return (token & D3DSI_OPCODE_MASK) == D3DSIO_END;
}

int GetShaderFuncLength(const DWORD *pFunction)
{
	int op = 0, l = 1;
	while (!ShaderIsEnd(pFunction[op++]))  l++;
	return l;
}

std::span<byte> LoadResource(UINT resId)
{
    const auto res = FindResource(Core::i()->dllModule(), MAKEINTRESOURCE(resId), RT_RCDATA);
	if(res)
	{
        const auto handle = LoadResource(Core::i()->dllModule(), res);
		if(handle)
		{
			size_t sz = SizeofResource(Core::i()->dllModule(), res);
			void* ptr = LockResource(handle);

			return std::span<byte>((byte*)ptr, sz);
		}
	}

	return std::span<byte>();
}

IDirect3DTexture9 *
CreateTextureFromResource(
	IDirect3DDevice9 * pDev,
	HMODULE hModule,
	unsigned uResource)
{
    const auto resourceSpan = LoadResource(uResource);
	if(resourceSpan.data() == nullptr)
		return nullptr;

	IDirect3DBaseTexture9* ret = nullptr;

	CreateDDSTextureFromMemory(pDev, resourceSpan.data(), resourceSpan.size_bytes(), &ret);

	return static_cast<IDirect3DTexture9*>(ret);
}

std::string ReadFile(std::istream& is)
{
	std::stringstream ss;
	for(std::string line; std::getline(is, line); )
	{
		if(line.ends_with('\r'))
			line.resize(line.size() - 1);
		ss << line << '\n';
	}

    return ss.str();
}

uint RoundUpToMultipleOf(uint numToRound, uint multiple)
{
    if (multiple == 0)
        return numToRound;

    uint remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;

    return numToRound + multiple - remainder;
}
}

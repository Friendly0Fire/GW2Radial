#pragma once
#include <Main.h>
#include <inttypes.h>

namespace GW2Addons
{

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr);
// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str);

std::wstring GetKeyName(unsigned int virtualKey);

void SplitFilename(const tstring & str, tstring * folder, tstring * file);

mstime TimeInMilliseconds();

bool FileExists(const TCHAR* path);

int GetShaderFuncLength(const DWORD *pFunction);

// Reverse iteration wrappers for use in range-based for-loops
// ReSharper disable CppInconsistentNaming

// ReSharper disable once CppImplicitDefaultConstructorNotAvailable
template <typename T>
struct reversion_wrapper { T& iterable; };

template <typename T>
auto begin(reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

template <typename T>
auto end(reversion_wrapper<T> w) { return std::rend(w.iterable); }

template <typename T>
reversion_wrapper<T> reverse(T&& iterable) { return { iterable }; }

bool LoadFontResource(UINT resId, void*& dataPtr, size_t& dataSize);

// ReSharper restore CppInconsistentNaming

inline float Lerp(float a, float b, float s)
{
	if (s < 0)
		return a;
	else if (s > 1)
		return b;
	else
		return (1 - s) * a + s * b;
}

inline float SmoothStep(float x)
{
	return 3 * x * x - 2 * x * x * x;
}

inline void FormattedOutputDebugString(const char* fmt, ...)
{
	char buffer[4096];
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	OutputDebugStringA(buffer);
	va_end(args);
}

}
#pragma once
#include <algorithm>
#include <Main.h>
#include <inttypes.h>
#include <imgui.h>
#include <istream>
#include <cwctype>

namespace GW2Radial
{

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr);
// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str);

void SplitFilename(const tstring & str, tstring * folder, tstring * file);

mstime TimeInMilliseconds();

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

std::span<byte> LoadResource(UINT resId);

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

inline float frand()
{
	return float(rand()) / RAND_MAX;
}

inline void FormattedOutputDebugString(const char* fmt, ...) {
	char buffer[4096];
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	if(IsDebuggerPresent())
	    OutputDebugStringA(buffer);
	else {
		GetLogStream() << buffer;
		GetLogStream().flush();
	}
	va_end(args);
}

inline void FormattedOutputDebugString(const wchar_t* fmt, ...) {
	wchar_t buffer[4096];
	va_list args;
	va_start(args, fmt);
	vswprintf_s(buffer, fmt, args);
	if(IsDebuggerPresent())
	    OutputDebugStringW(buffer);
	else {
		GetLogStream() << utf8_encode(buffer);
		GetLogStream().flush();
	}
	va_end(args);
}

IDirect3DTexture9* CreateTextureFromResource(IDirect3DDevice9 * pDev, HMODULE hModule, unsigned uResource);

template<typename T>
auto ConvertToVector4(const T& val) {
	const float IntOffset = 0.f;
	// Fundamental types
	if constexpr (std::is_floating_point_v<T>) {
		return fVector4{ float(val), float(val), float(val), float(val) };
	} else if constexpr (std::is_integral_v<T> || std::is_enum_v<T>) {
		return fVector4{ float(val) + IntOffset, float(val) + IntOffset, float(val) + IntOffset, float(val) + IntOffset };
	// Float vectors
	} else if constexpr (std::is_same_v<T, fVector2>) {
		return fVector4{ val.x, val.y, val.x, val.y };
	} else if constexpr (std::is_same_v<T, fVector3>) {
		return fVector4{ val.x, val.y, val.z, val.x };
	} else if constexpr (std::is_same_v<T, fVector4>) {
		return val;
	// Int vectors
	} else if constexpr (std::is_same_v<T, iVector2>) {
		return fVector4{ float(val.x) + IntOffset, float(val.y) + IntOffset, float(val.x) + IntOffset, float(val.y) + IntOffset };
	} else if constexpr (std::is_same_v<T, iVector3>) {
		return fVector4{ float(val.x) + IntOffset, float(val.y) + IntOffset, float(val.z) + IntOffset, float(val.x) + IntOffset };
	} else if constexpr (std::is_same_v<T, iVector4>) {
		return fVector4{ float(val.x) + IntOffset, float(val.y) + IntOffset, float(val.z) + IntOffset, float(val.w) + IntOffset };
	} else {
		return fVector4{ };
	}
}

inline ImVec4 ConvertVector(const fVector4& val) {
	return { val.x, val.y, val.z, val.w };
}

inline ImVec2 ConvertVector(const fVector2& val) {
	return { val.x, val.y };
}

uint RoundUpToMultipleOf(uint numToRound, uint multiple);

template<typename Char, typename It>
It SplitString(const Char* str, const Char* delim, It out)
{
	std::basic_string<Char> s(str);
	if(s.empty())
		return out;

	size_t start = 0;
    size_t end = 0;
	while ((end = s.find(delim, start)) != std::string::npos) {
        *out = s.substr(start, end - start);
		++out;
		start = end + 1;
    }

	*out = s.substr(start);
	++out;
	return out;
}

inline uint RGBAto32(const fVector4& rgb, bool scale)
{
	float s = scale ? 255.f : 1.f;
    return D3DCOLOR_RGBA(byte(rgb.x * s), byte(rgb.y * s), byte(rgb.z * s), byte(rgb.w * s));
}

inline std::string ToLower(std::string in) {
	std::transform(in.begin(), in.end(), in.begin(), std::tolower);
	return in;
}
inline std::wstring ToLower(std::wstring in) {
	std::transform(in.begin(), in.end(), in.begin(), std::towlower);
	return in;
}

inline std::string ToUpper(std::string in) {
	std::transform(in.begin(), in.end(), in.begin(), std::toupper);
	return in;
}
inline std::wstring ToUpper(std::wstring in) {
	std::transform(in.begin(), in.end(), in.begin(), std::towupper);
	return in;
}

template<typename Vec>
float Luma(const Vec& v)
{
    return v.x * 0.2126 + v.y * 0.7152 + v.z * 0.0722;
}

void DumpSurfaceToDiskTGA(IDirect3DDevice9* dev, IDirect3DSurface9* surf, uint bpp, const std::wstring& filename);

}

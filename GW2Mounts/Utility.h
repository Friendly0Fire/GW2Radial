#pragma once
#include "main.h"

std::wstring s2ws(const std::string & str);

std::string ws2s(const std::wstring & wstr);

std::string GetKeyName(unsigned int virtualKey);

void SplitFilename(const tstring & str, tstring * folder, tstring * file);

mstime timeInMS();

bool FileExists(const TCHAR* path);

int GetShaderFuncLength(const DWORD *pFunction);

// Reverse iteration wrappers for use in range-based for-loops

template <typename T>
struct reversion_wrapper { T& iterable; };

template <typename T>
auto begin(reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

template <typename T>
auto end(reversion_wrapper<T> w) { return std::rend(w.iterable); }

template <typename T>
reversion_wrapper<T> reverse(T&& iterable) { return { iterable }; }

inline float lerp(float a, float b, float s)
{
	if (s < 0)
		return a;
	else if (s > 1)
		return b;
	else
		return (1 - s) * a + s * b;
}

inline float smoothstep(float x)
{
	return 3 * x * x - 2 * x * x * x;
}
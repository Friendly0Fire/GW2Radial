#pragma once
#include "main.h"

std::wstring s2ws(const std::string & str);

std::string ws2s(const std::wstring & wstr);

std::string GetKeyName(unsigned int virtualKey);

void SplitFilename(const tstring & str, tstring * folder, tstring * file);

mstime timeInMS();

bool FileExists(const TCHAR* path);

// Reverse iteration wrappers for use in range-based for-loops

template <typename T>
struct reversion_wrapper { T& iterable; };

template <typename T>
auto begin(reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

template <typename T>
auto end(reversion_wrapper<T> w) { return std::rend(w.iterable); }

template <typename T>
reversion_wrapper<T> reverse(T&& iterable) { return { iterable }; }

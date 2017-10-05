#pragma once
#include "main.h"

std::wstring s2ws(const std::string & str);

std::string ws2s(const std::wstring & wstr);

std::string GetKeyName(unsigned int virtualKey);

void SplitFilename(const tstring & str, tstring * folder, tstring * file);

mstime timeInMS();

bool FileExists(const TCHAR* path);
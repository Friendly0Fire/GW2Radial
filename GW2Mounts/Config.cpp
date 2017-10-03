#include "Config.h"
#include <Shlobj.h>
#include "Utility.h"
#include <tchar.h>
#include <sstream>

Config::Config()
{
}


Config::~Config()
{
}

void Config::Load()
{
	// Create folders
	TCHAR exeFullPath[MAX_PATH];
	GetModuleFileName(0, exeFullPath, MAX_PATH);
	tstring exeFolder;
	SplitFilename(exeFullPath, &exeFolder, nullptr);
	_ConfigFolder = exeFolder + TEXT("\\addons\\mounts\\");
	_tcscpy_s(_ConfigLocation, (_ConfigFolder + ConfigName).c_str());
#if _UNICODE
	strcpy_s(_ImGuiConfigLocation, ws2s(_ConfigFolder + ImGuiConfigName).c_str());
#else
	strcpy_s(_ImGuiConfigLocation, (_ConfigFolder + ImGuiConfigName).c_str());
#endif
	SHCreateDirectoryEx(nullptr, _ConfigFolder.c_str(), nullptr);

	// Load INI settings
	_INI.SetUnicode();
	_INI.LoadFile(_ConfigLocation);
	_ShowGriffon = _stricmp(_INI.GetValue("General", "show_fifth_mount", "false"), "true") == 0;
	const char* keys = _INI.GetValue("Keybinds", "mount_wheel", nullptr);
	if (keys)
	{
		std::stringstream ss(keys);
		std::vector<std::string> result;

		while (ss.good())
		{
			std::string substr;
			std::getline(ss, substr, ',');
			int val = std::stoi(substr);
			_MountOverlayKeybind.insert((uint)val);
		}
	}
}

void Config::MountOverlayKeybind(std::set<uint>& val)
{
	_MountOverlayKeybind = val;
	std::string setting_value = "";
	for (const auto& k : _MountOverlayKeybind)
		setting_value += std::to_string(k) + ", ";

	_INI.SetValue("Keybinds", "mount_wheel", (setting_value.size() > 0 ? setting_value.substr(0, setting_value.size() - 2) : setting_value).c_str());
	_INI.SaveFile(_ConfigLocation);
}

void Config::MountKeybind(uint i, std::set<uint>& val)
{
	_MountKeybinds[i] = val;
	std::string setting_value = "";
	for (const auto& k : _MountKeybinds[i])
		setting_value += std::to_string(k) + ", ";

	_INI.SetValue("Keybinds", ("mount_" + std::to_string(i)).c_str(), (setting_value.size() > 0 ? setting_value.substr(0, setting_value.size() - 2) : setting_value).c_str());
	_INI.SaveFile(_ConfigLocation);
}

void Config::ShowGriffonSave()
{
	_INI.SetValue("General", "show_fifth_mount", _ShowGriffon ? "true" : "false");
	_INI.SaveFile(_ConfigLocation);
}

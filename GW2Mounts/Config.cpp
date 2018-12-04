#include "Config.h"
#include <Shlobj.h>
#include "Utility.h"
#include <tchar.h>
#include <sstream>

Config::Config()
{
	_SettingsKeybind.insert(VK_SHIFT);
	_SettingsKeybind.insert(VK_MENU);
	_SettingsKeybind.insert('M');
}


Config::~Config()
{
}

void Config::Load()
{
	_ResetCursorOnLockedKeybind = _INI.GetBoolValue("General", "reset_cursor_on_locked_keybind", _ResetCursorOnLockedKeybind);
	_LockCameraWhenOverlayed = _INI.GetBoolValue("General", "lock_camera_when_overlayed", _LockCameraWhenOverlayed);
	_OverlayDelayMilliseconds = _INI.GetLongValue("General", "overlay_delay_ms", _OverlayDelayMilliseconds);
	_OverlayScale = (float)_INI.GetDoubleValue("General", "overlay_scale", _OverlayScale);
	_OverlayDeadZoneScale = (float)_INI.GetDoubleValue("General", "overlay_dead_zone_scale", _OverlayDeadZoneScale);
	_OverlayDeadZoneBehavior = _INI.GetLongValue("General", "overlay_dead_zone_behavior", _OverlayDeadZoneBehavior);
	_FavoriteMount = (MountType)_INI.GetLongValue("General", "favorite_mount", (int)_FavoriteMount);

	const char* keys = _INI.GetValue("Keybinds", "mount_wheel", nullptr);
	LoadKeybindString(keys, _MountOverlayKeybind);
	const char* keys_locked = _INI.GetValue("Keybinds", "mount_wheel_locked", nullptr);
	LoadKeybindString(keys_locked, _MountOverlayLockedKeybind);

	for (uint i = 0; i < MountTypeCount; i++)
	{
		const char* keys_mount = _INI.GetValue("Keybinds", ("mount_" + std::to_string(i)).c_str(), nullptr);
		LoadKeybindString(keys_mount, _MountKeybinds[i]);
	}
}

void Config::MountOverlayKeybind(const std::set<uint>& val)
{
	_MountOverlayKeybind = val;
	std::string setting_value = "";
	for (const auto& k : _MountOverlayKeybind)
		setting_value += std::to_string(k) + ", ";

	_INI.SetValue("Keybinds", "mount_wheel", (setting_value.size() > 0 ? setting_value.substr(0, setting_value.size() - 2) : setting_value).c_str());
	Save();
}

void Config::MountOverlayLockedKeybind(const std::set<uint>& val)
{
	_MountOverlayLockedKeybind = val;
	std::string setting_value = "";
	for (const auto& k : _MountOverlayLockedKeybind)
		setting_value += std::to_string(k) + ", ";

	_INI.SetValue("Keybinds", "mount_wheel_locked", (setting_value.size() > 0 ? setting_value.substr(0, setting_value.size() - 2) : setting_value).c_str());
	Save();
}

void Config::MountKeybind(uint i, const std::set<uint>& val)
{
	_MountKeybinds[i] = val;
	std::string setting_value = "";
	for (const auto& k : _MountKeybinds[i])
		setting_value += std::to_string(k) + ", ";

	_INI.SetValue("Keybinds", ("mount_" + std::to_string(i)).c_str(), (setting_value.size() > 0 ? setting_value.substr(0, setting_value.size() - 2) : setting_value).c_str());
	Save();
}

void Config::ResetCursorOnLockedKeybindSave()
{
	_INI.SetBoolValue("General", "reset_cursor_on_locked_keybind", _ResetCursorOnLockedKeybind);
	Save();
}

void Config::LockCameraWhenOverlayedSave()
{
	_INI.SetBoolValue("General", "lock_camera_when_overlayed", _LockCameraWhenOverlayed);
	Save();
}

void Config::OverlayDelayMillisecondsSave()
{
	_INI.SetLongValue("General", "overlay_delay_ms", _OverlayDelayMilliseconds);
	Save();
}

void Config::OverlayScaleSave()
{
	_INI.SetDoubleValue("General", "overlay_scale", _OverlayScale);
	Save();
}

void Config::OverlayDeadZoneScaleSave()
{
	_INI.SetDoubleValue("General", "overlay_dead_zone_scale", _OverlayDeadZoneScale);
	Save();
}

void Config::OverlayDeadZoneBehaviorSave()
{
	_INI.SetLongValue("General", "overlay_dead_zone_behavior", _OverlayDeadZoneBehavior);
	Save();
}

void Config::FavoriteMountSave()
{
	_INI.SetLongValue("General", "favorite_mount", (int)_FavoriteMount);
	Save();
}
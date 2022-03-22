#include <ConfigurationFile.h>
#include <filesystem>
#include <Shlobj.h>
#include <Utility.h>
#include <tchar.h>
#include <sstream>
#include "../include/ImGuiPopup.h"

namespace GW2Radial
{

const wchar_t* g_configName = L"config.ini";
const wchar_t* g_imguiConfigName = L"imgui_config.ini";

ConfigurationFile::ConfigurationFile()
{
	Reload();
}

void ConfigurationFile::Reload()
{
	LogDebug("Reloading configuration files");
	readOnlyWarned_ = false;

	auto folder = GetAddonFolder();
	if (!folder)
	{
		LogWarn("Could not find addon folder");
		folder_ = std::nullopt;
		readOnly_ = false;
		return;
	}

	auto cfgFile = *folder / g_configName;
	FILE* fp = nullptr;
	if (_wfopen_s(&fp, cfgFile.c_str(), L"ab") != 0)
	{
		LogWarn(L"Could not write to config file '{}'", cfgFile.wstring());
		if (_wfopen_s(&fp, cfgFile.c_str(), L"rb") != 0)
		{
			LogError(L"Could read config file '{}'", cfgFile.wstring());
			folder_ = std::nullopt;
			readOnly_ = false;
			return;
		}
		else if(fp)
			fclose(fp);
		readOnly_ = true;
	}
	else if (fp)
		fclose(fp);
	
	ini_.SetUnicode();
	ini_.LoadFile(cfgFile.c_str());
	LoadImGuiSettings(*folder / g_imguiConfigName);
	folder_ = folder;

	LogInfo(L"Config folder is now '{}'", folder_->wstring());
}

void ConfigurationFile::Save()
{
	if (!folder_)
	{
		const auto prevSaveError = lastSaveError_;
		lastSaveError_ = "No configuration folder could be located.";
		lastSaveErrorChanged_ |= prevSaveError != lastSaveError_;
		return;
	}

	if (readOnly_)
	{
		if (!readOnlyWarned_)
		{
			LogWarn("Configuration files in read-only mode, changes will not be saved!");
			readOnlyWarned_ = true;
		}
		return;
	}

	const auto r = ini_.SaveFile((*folder_ / g_configName).c_str());

	if (r < 0)
	{
		const auto prevSaveError = lastSaveError_;
		switch (r)
		{
		case SI_FAIL:
			lastSaveError_ = "Unknown error";
			break;
		case SI_NOMEM:
			lastSaveError_ = "Out of memory";
			break;
		case SI_FILE:
			char buf[1024];
			if (strerror_s(buf, errno) == 0) {
				buf[1023] = '\0';
				lastSaveError_.assign(buf);
			} else
				lastSaveError_ = "Unknown error";
			break;
		default:
			lastSaveError_ = "Unknown error";
		}

		lastSaveErrorChanged_ |= prevSaveError != lastSaveError_;
	}
	else if(!lastSaveError_.empty())
	{
		lastSaveError_.clear();
		lastSaveErrorChanged_ = true;
	}
}

void ConfigurationFile::OnUpdate() const
{
	if(folder_)
		SaveImGuiSettings(*folder_ / g_imguiConfigName);
}

void ConfigurationFile::LoadImGuiSettings(const std::wstring & location)
{
	FILE *fp = nullptr;
	if(_wfopen_s(&fp, location.c_str(), L"rt, ccs=UTF-8") != 0 || fp == nullptr)
		return;

	fseek(fp, 0, SEEK_END);
	const auto num = ftell(fp);

	std::wstring contents;
	contents.resize(size_t(num) + 1);

	fseek(fp, 0, SEEK_SET);
	fread_s(contents.data(), contents.size() * sizeof(wchar_t), sizeof(wchar_t), num, fp);
	fclose(fp);

	auto utf8 = utf8_encode(contents);
	ImGui::LoadIniSettingsFromMemory(utf8.c_str(), utf8.size());
}

void ConfigurationFile::SaveImGuiSettings(const std::wstring & location)
{
	auto& imio = ImGui::GetIO();
	if(!imio.WantSaveIniSettings)
		return;

	FILE *fp = nullptr;
	if(_wfopen_s(&fp, location.c_str(), L"wt, ccs=UTF-8") != 0 || fp == nullptr)
		return;

	size_t num;
	const auto contentChar = ImGui::SaveIniSettingsToMemory(&num);
	const std::string contents(contentChar, num);
	const auto unicode = utf8_decode(contents);

	fwrite(unicode.data(), sizeof(wchar_t), num, fp);
	fclose(fp);
	
	imio.WantSaveIniSettings = false;
}

}

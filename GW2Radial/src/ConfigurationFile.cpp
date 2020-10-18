#include <ConfigurationFile.h>
#include <filesystem>
#include <Shlobj.h>
#include <Utility.h>
#include <tchar.h>
#include <sstream>
#include "../include/ImGuiPopup.h"

namespace GW2Radial
{
DEFINE_SINGLETON(ConfigurationFile);
	
const wchar_t* g_configName = TEXT("config.ini");
const wchar_t* g_imguiConfigName = TEXT("imgui_config.ini");

ConfigurationFile::ConfigurationFile()
{
	Reload();
}

void ConfigurationFile::Reload()
{
	auto folders = GetAddonFolders();
	auto [pfExists, pfWritable] = CheckFolder(folders.programFiles);
	auto [mdExists, mdWritable] = CheckFolder(folders.myDocuments);
	
	ini_.SetUnicode();
	if(pfExists)
	{
		ini_.LoadFile((folders.programFiles / g_configName).c_str());
		LoadImGuiSettings(folders.programFiles / g_imguiConfigName);
	}
	else if(mdExists)
	{
		ini_.LoadFile((folders.myDocuments / g_configName).c_str());
		LoadImGuiSettings(folders.myDocuments / g_imguiConfigName);
	}

	if(pfWritable)
		folder_ = folders.programFiles;
	else
		folder_ = folders.myDocuments;
	
	location_ = folder_ + g_configName;
	imguiLocation_ = folder_ + g_imguiConfigName;
}

void ConfigurationFile::Save()
{
	const auto r = ini_.SaveFile(location_.c_str());

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
	SaveImGuiSettings(imguiLocation_);
}

std::tuple<bool /*exists*/, bool /*writable*/> ConfigurationFile::CheckFolder(const std::filesystem::path& folder)
{
	const auto filepath = folder / g_configName;

	bool exists = true, writable = true;

	if(SHCreateDirectoryExW(nullptr, folder.c_str(), nullptr) == ERROR_ACCESS_DENIED)
		writable = false;

	if(!std::filesystem::exists(filepath))
		exists = false;

	if(writable)
	{
		FILE *fp = nullptr;
		if(_wfopen_s(&fp, filepath.c_str(), L"ab") != 0)
			writable = false;
		else if(fp)
			fclose(fp);
	}

	return { exists, writable };
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

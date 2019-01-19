#pragma once
#include <Main.h>
#include <simpleini/SimpleIni.h>
#include <Singleton.h>

namespace GW2Radial
{

class ConfigurationFile : public Singleton<ConfigurationFile>
{
public:
	ConfigurationFile();

	void Reload();
	void Save();
	void OnUpdate() const;
	
	const std::string& lastSaveError() const { return lastSaveError_; }
	bool lastSaveErrorChanged() const { return lastSaveErrorChanged_; }
	void lastSaveErrorChanged(bool v) { lastSaveErrorChanged_ = v; lastSaveError_.clear(); }

	CSimpleIniA& ini() { return ini_; }

protected:
	static std::tuple<bool /*exists*/, bool /*writable*/> CheckFolder(const std::wstring& folder);
	static void LoadImGuiSettings(const std::wstring& location);
	static void SaveImGuiSettings(const std::wstring& location);

	CSimpleIniA ini_;
	std::wstring folder_;
	std::wstring location_, imguiLocation_;

	bool lastSaveErrorChanged_ = false;
	std::string lastSaveError_;
};

}

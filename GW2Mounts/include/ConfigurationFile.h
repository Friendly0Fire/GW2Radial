#pragma once
#include <Main.h>
#include <simpleini/SimpleIni.h>

namespace GW2Addons
{

class ConfigurationFile
{
public:
	static ConfigurationFile* i()
	{
		if(!i_)
			i_ = std::make_unique<ConfigurationFile>();
		return i_.get();
	}

	void Reload();
	void Save();

	CSimpleIniA& ini() { return ini_; }

protected:
	ConfigurationFile();

	static std::unique_ptr<ConfigurationFile> i_;

	CSimpleIniA ini_;
	tstring folder_;
	TCHAR location_[MAX_PATH] { };
	char imguiLocation_[MAX_PATH] { };

	bool lastSaveErrorChanged_ = false;
	std::string lastSaveError_;
};

}
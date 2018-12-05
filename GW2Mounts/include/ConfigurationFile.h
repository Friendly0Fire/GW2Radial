#pragma once
#include <Main.h>
#include <simpleini/SimpleIni.h>
#include <Singleton.h>

namespace GW2Addons
{

class ConfigurationFile : public Singleton<ConfigurationFile>
{
public:

	void Reload();
	void Save();

	CSimpleIniA& ini() { return ini_; }

protected:
	ConfigurationFile();

	CSimpleIniA ini_;
	tstring folder_;
	TCHAR location_[MAX_PATH] { };
	char imguiLocation_[MAX_PATH] { };

	bool lastSaveErrorChanged_ = false;
	std::string lastSaveError_;
};

}

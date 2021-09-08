#include <FileSystem.h>
#include <GFXSettings.h>
#include <tinyxml2/tinyxml2.h>
#include <Utility.h>

namespace GW2Radial {

GFXSettings::GFXSettings() {
    filePath_ = FileSystem::GetSystemPath(FOLDERID_RoamingAppData) / "Guild Wars 2" / "GFXSettings.Gw2-64.exe.xml";
    Reload();
}

void GFXSettings::Reload() {
    settings_.clear();

    if(!FileSystem::Exists(filePath_))
        return;

    auto fileTime = std::filesystem::last_write_time(filePath_);
    if(fileTime <= lastFileTime_)
        return;

    lastFileTime_ = fileTime;

    tinyxml2::XMLDocument xml;
    if(xml.LoadFile(filePath_.string().c_str()) != tinyxml2::XML_SUCCESS)
        return;

    const auto* const root = xml.FirstChildElement("GSA_SDK");
    if(root) {
        const auto* const gameSettings = root->FirstChildElement("GAMESETTINGS");
        if(gameSettings) {
            const auto* setting = gameSettings->FirstChildElement();
            do {
                const auto* type = setting->Name();
                if(ToLower(std::string(type)) != "option")
                    continue;

                const auto* name = setting->Attribute("Name");
                if(!name)
                    continue;
                const auto* value = setting->Attribute("Value");
                if(!value)
                    continue;

                settings_[ToLower(std::string(name))] = ToLower(std::string(value));
            } while((setting = setting->NextSiblingElement()) != nullptr);
        }
    }

    if(settings_.count("dpiscaling") != 0)
        dpiScaling_ = settings_["dpiscaling"] == "true";
    else
        dpiScaling_ = false;
}

void GFXSettings::OnUpdate() {
    Reload();
}



}
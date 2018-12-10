#include <Keybind.h>
#include <Utility.h>
#include <sstream>
#include <ConfigurationFile.h>

namespace GW2Addons
{

Keybind::Keybind(std::string nickname, std::string displayName, const std::set<uint>& keys, bool saveToConfig) :
	nickname_(std::move(nickname)), displayName_(std::move(displayName)), saveToConfig_(saveToConfig)
{
	this->keys(keys);
	isBeingModified_ = false;
}

Keybind::Keybind(std::string nickname, std::string displayName) :
	nickname_(std::move(nickname)), displayName_(std::move(displayName))
{
	const auto keys = ConfigurationFile::i()->ini().GetValue("Keybinds", nickname_.c_str());
	if(keys) this->keys(keys);
	isBeingModified_ = false;
}

void Keybind::keys(const std::set<uint>& keys)
{
	if(!isBeingModified_)
		return;

	keys_ = keys;

	ApplyKeys();
}

void Keybind::keys(const char * keys)
{
	keys_.clear();

	if (strnlen_s(keys, 256) > 0)
	{
		std::stringstream ss(keys);
		std::vector<std::string> result;

		while (ss.good())
		{
			std::string substr;
			std::getline(ss, substr, ',');
			const auto val = std::stoi(substr);
			keys_.insert(static_cast<uint>(val));
		}
	}

	ApplyKeys();
}

void Keybind::ApplyKeys()
{
	UpdateDisplayString();
	
	if(saveToConfig_)
	{
		std::string settingValue;
		for (const auto& k : keys_)
			settingValue += std::to_string(k) + ", ";

		if(!keys_.empty())
			settingValue = settingValue.substr(0, settingValue.size() - 2);

		auto cfg = ConfigurationFile::i();
		cfg->ini().SetValue("keybinds", nickname_.c_str(), settingValue.c_str());
		cfg->Save();
	}
}

void Keybind::UpdateDisplayString()
{
	std::string keybind;
	for (const auto& k : keys_)
		keybind += GetKeyName(k) + std::string(" + ");

	strcpy_s(keysDisplayString_.data(), keysDisplayString_.size(), (keybind.empty() ? keybind : keybind.substr(0, keybind.size() - 3)).c_str());
}

}

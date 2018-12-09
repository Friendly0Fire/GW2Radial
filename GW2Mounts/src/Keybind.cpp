#include <Keybind.h>
#include <Utility.h>
#include <sstream>
#include <ConfigurationFile.h>

namespace GW2Addons
{

Keybind::Keybind(std::string displayName, std::string nickname, const std::set<uint>& keys) :
	displayName_(std::move(displayName)), nickname_(std::move(nickname))
{
	this->keys(keys);
}

Keybind::Keybind(std::string displayName, std::string nickname) :
	displayName_(std::move(displayName)), nickname_(std::move(nickname))
{
	const auto keys = ConfigurationFile::i()->ini().GetValue("Keybinds", nickname.c_str());
	if(keys) this->keys(keys);
}

void Keybind::keys(const std::set<uint>& keys)
{
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

	std::string settingValue;
	for (const auto& k : keys_)
		settingValue += std::to_string(k) + ", ";

	auto cfg = ConfigurationFile::i();
	cfg->ini().SetValue("Keybinds", nickname_.c_str(), settingValue.c_str());
	cfg->Save();
}

void Keybind::UpdateDisplayString()
{
	std::string keybind;
	for (const auto& k : keys_)
		keybind += GetKeyName(k) + std::string(" + ");

	strcpy_s(keysDisplayString_.data(), keysDisplayString_.size(), (keybind.empty() ? keybind : keybind.substr(0, keybind.size() - 3)).c_str());
}

}

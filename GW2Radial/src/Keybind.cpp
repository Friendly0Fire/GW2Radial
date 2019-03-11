#include <Keybind.h>
#include <Utility.h>
#include <sstream>
#include <ConfigurationFile.h>
#include <numeric>

namespace GW2Radial
{
std::unordered_map<Keybind*, std::set<uint>> Keybind::keyMaps_;

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

Keybind::~Keybind()
{
	keyMaps_.erase(this);
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
	if(!isBeingModified_)
		return;

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

	CheckForConflict();
	
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

void Keybind::CheckForConflict(bool recurse)
{
	keyMaps_[this] = keys_;
	
	isConflicted_ = false;
	for(auto& elem : keyMaps_)
	{
		if(elem.first != this && !keys_.empty() && elem.second == keys_)
			isConflicted_ = true;
		
		if(recurse) elem.first->CheckForConflict(false);
	}
}

bool Keybind::conflicts(const std::set<uint>& pressedKeys) const
{
	for(auto& elem : keyMaps_)
	{
		if(elem.first != this && keys_.size() < elem.second.size()
			&& std::includes(pressedKeys.begin(), pressedKeys.end(), elem.second.begin(), elem.second.end()))
			return true;
	}

	return false;
}

void Keybind::UpdateDisplayString()
{
	if(keys_.empty())
	{
		keysDisplayString_[0] = '\0';
		return;
	}

	std::wstring keybind = std::accumulate(std::next(keys_.begin()), keys_.end(), GetKeyName(*keys_.begin()), [](std::wstring a, uint b) { return std::move(a) + L" + " + GetKeyName(b); });

	strcpy_s(keysDisplayString_.data(), keysDisplayString_.size(), utf8_encode(keybind).c_str());
}

}

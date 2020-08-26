#include <Keybind.h>
#include <Utility.h>
#include <sstream>
#include <ConfigurationFile.h>
#include <numeric>

namespace GW2Radial
{
std::set<Keybind*> Keybind::keybinds_;

Keybind::Keybind(std::string nickname, std::string displayName, std::string category, const std::set<ScanCode>& scs, bool saveToConfig) :
	nickname_(std::move(nickname)), displayName_(std::move(displayName)), category_(std::move(category)), saveToConfig_(saveToConfig)
{
	keybinds_.insert(this);
	this->scanCodes(scs);
	isBeingModified_ = false;
}

Keybind::Keybind(std::string nickname, std::string displayName, std::string category) :
	nickname_(std::move(nickname)), displayName_(std::move(displayName)), category_(std::move(category))
{
	keybinds_.insert(this);
	auto keys = ConfigurationFile::i()->ini().GetValue("Keybinds.2", nickname_.c_str());
	if(keys) this->scanCodes(keys);
	else {
		keys = ConfigurationFile::i()->ini().GetValue("Keybinds", nickname_.c_str());
		if(keys) this->scanCodes(keys, true);
	}
	isBeingModified_ = false;
}

Keybind::~Keybind()
{
	keybinds_.erase(this);
}

void Keybind::scanCodes(const std::set<ScanCode>& scs)
{
	if(!isBeingModified_)
		return;

	scanCodes_ = scs;

	ApplyKeys();
}

void Keybind::scanCodes(const char* keys, bool vKey)
{
	if(!isBeingModified_)
		return;

	scanCodes_.clear();

	if (strnlen_s(keys, 256) > 0)
	{
		std::stringstream ss(keys);
		std::vector<std::string> result;

		while (ss.good())
		{
			std::string substr;
			std::getline(ss, substr, ',');
			auto val = std::stoi(substr);
			if (vKey)
				val = MapVirtualKeyA(val, MAPVK_VK_TO_VSC);
			scanCodes_.insert(static_cast<ScanCode>(val));
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
		for (cref k : scanCodes_)
			settingValue += std::to_string(uint(k)) + ", ";

		if(!scanCodes_.empty())
			settingValue = settingValue.substr(0, settingValue.size() - 2);

		auto cfg = ConfigurationFile::i();
		cfg->ini().SetValue("Keybinds.2", nickname_.c_str(), settingValue.c_str());
		cfg->Save();
	}
}

bool Keybind::matchesNoLeftRight(const std::set<ScanCode>& scanCodes) const
{
	const auto universalize = [](const std::set<ScanCode>& src)
	{
		std::set<ScanCode> out;
		for(cref k : src)
		{
			if (IsModifier(k))
				out.insert(MakeUniversal(k));
			else
				out.insert(k);
		}

		return out;
	};

	return universalize(scanCodes) == universalize(scanCodes_);
}

void Keybind::UpdateDisplayString()
{
	if(scanCodes_.empty())
	{
		keysDisplayString_[0] = '\0';
		return;
	}

	std::vector<ScanCode> displayScanCodes(scanCodes_.begin(), scanCodes_.end());
	std::sort(displayScanCodes.begin(), displayScanCodes.end(), ScanCodeCompare());

	std::wstring keybind = std::accumulate(std::next(displayScanCodes.begin()), displayScanCodes.end(), GetScanCodeName(displayScanCodes.front()), [](std::wstring a, ScanCode b) { return std::move(a) + L" + " + GetScanCodeName(b); });

	strcpy_s(keysDisplayString_.data(), keysDisplayString_.size(), utf8_encode(keybind).c_str());
}

void Keybind::ForceRefreshDisplayStrings() {
	for (auto& kb : keybinds_)
		kb->UpdateDisplayString();
}

}

#pragma once
#include <Main.h>
#include <array>
#include <set>
#include <functional>
#include <unordered_map>
#include <Input.h>

namespace GW2Radial
{

class Keybind
{
public:
	Keybind(std::string nickname, std::string displayName, const std::set<ScanCode>& scs, bool saveToConfig);
	Keybind(std::string nickname, std::string displayName);
	~Keybind();

	const std::set<ScanCode>& scanCodes() const { return scanCodes_; }
	void scanCodes(const std::set<ScanCode>& scs);
	void scanCodes(const char* keys);

	const std::string& displayName() const { return displayName_; }
	void displayName(const std::string& n) { displayName_ = n; }

	const std::string& nickname() const { return nickname_; }
	void nickname(const std::string& n) { nickname_ = n; }

	bool isSet() const { return !scanCodes_.empty(); }
	bool isConflicted() const { return isConflicted_; }

	bool isBeingModified() const { return isBeingModified_; }
	void isBeingModified(bool ibm) { isBeingModified_ = ibm; }

	const char* keysDisplayString() const { return keysDisplayString_.data(); }
	std::array<char, 256>& keysDisplayStringArray() { return keysDisplayString_; }

	bool conflicts(const std::set<ScanCode>& scanCodes) const;
	
	bool matches(const std::set<ScanCode>& scanCodes) const { return scanCodes == scanCodes_; }
	bool matchesPartial(const std::set<ScanCode>& scanCodes) const
	{
		return isSet() && std::includes(scanCodes.begin(), scanCodes.end(), scanCodes_.begin(), scanCodes_.end());
	}
	bool matchesNoLeftRight(const std::set<ScanCode>& scanCodes) const;

	static void ForceRefreshDisplayStrings();

protected:
	void UpdateDisplayString();
	void ApplyKeys();
	void CheckForConflict(bool recurse = true);

	std::string displayName_, nickname_;
	std::array<char, 256> keysDisplayString_ { };
	bool isBeingModified_ = true;
	std::set<ScanCode> scanCodes_;
	bool saveToConfig_ = true;
	bool isConflicted_ = false;

	static std::vector<Keybind*> keybinds_;
	static std::unordered_map<Keybind*, std::set<ScanCode>> scMaps_;
};

}
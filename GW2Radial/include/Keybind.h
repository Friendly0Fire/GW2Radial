#pragma once
#include <Main.h>
#include <array>
#include <set>
#include <Input.h>

namespace GW2Radial
{

class Keybind
{
public:
	Keybind(std::string nickname, std::string displayName, std::string category, const std::set<ScanCode>& scs, bool saveToConfig);
	Keybind(std::string nickname, std::string displayName, std::string category);
	~Keybind();

	[[nodiscard]] const std::set<ScanCode>& scanCodes() const { return scanCodes_; }
	void scanCodes(const std::set<ScanCode>& scs);
	void scanCodes(const char* keys, bool vKey = false);

	[[nodiscard]] const std::string& displayName() const { return displayName_; }
	void displayName(const std::string& n) { displayName_ = n; }

	[[nodiscard]] const std::string& nickname() const { return nickname_; }
	void nickname(const std::string& n) { nickname_ = n; }

	[[nodiscard]] bool isSet() const { return !scanCodes_.empty(); }

	[[nodiscard]] bool isBeingModified() const { return isBeingModified_; }
	void isBeingModified(bool ibm) { isBeingModified_ = ibm; }

	[[nodiscard]] const char* keysDisplayString() const { return keysDisplayString_.data(); }
	std::array<char, 256>& keysDisplayStringArray() { return keysDisplayString_; }
	
	[[nodiscard]] bool matches(const std::set<ScanCode>& scanCodes) const { return isSet() && (scanCodes == scanCodes_); }
	[[nodiscard]] bool matchesPartial(const std::set<ScanCode>& scanCodes) const
	{
		return isSet() && std::includes(scanCodes.begin(), scanCodes.end(), scanCodes_.begin(), scanCodes_.end());
	}
	[[nodiscard]] bool matchesNoLeftRight(const std::set<ScanCode>& scanCodes) const;

	static void ForceRefreshDisplayStrings();

protected:
	void UpdateDisplayString();
	virtual void ApplyKeys();

	std::string displayName_, nickname_, category_;
	std::array<char, 256> keysDisplayString_ { };
	bool isBeingModified_ = true;
	std::set<ScanCode> scanCodes_;
	bool saveToConfig_ = true;

	static std::set<Keybind*> keybinds_;
};

}
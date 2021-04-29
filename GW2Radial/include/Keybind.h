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
	Keybind(std::string nickname, std::string displayName, std::string category, const ScanCodeSet& scs, bool saveToConfig);
	Keybind(std::string nickname, std::string displayName, std::string category);
	~Keybind();

	[[nodiscard]] const ScanCodeSet& scanCodes() const { return scanCodes_; }
	void scanCodes(const ScanCodeSet& scs);
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
	
	[[nodiscard]] bool matches(const ScanCodeSet& scanCodes) const;
	[[nodiscard]] bool matchesPartial(const ScanCodeSet& scanCodes) const;
	[[nodiscard]] bool matchesNoLeftRight(const ScanCodeSet& scanCodes) const;

	[[nodiscard]] bool contains(ScanCode sc) const { return scanCodes_.count(sc) > 0; }

	static void ForceRefreshDisplayStrings();

protected:
	void UpdateDisplayString();
	virtual void ApplyKeys();

	std::string displayName_, nickname_, category_;
	std::array<char, 256> keysDisplayString_ { };
	bool isBeingModified_ = true;
	ScanCodeSet scanCodes_;
	bool saveToConfig_ = true;

	static std::set<Keybind*> keybinds_;
};

}
#pragma once
#include <Main.h>
#include <array>
#include <set>
#include <Input.h>

namespace GW2Radial
{

using Keyset = std::pair<ScanCode, Modifier>;

class Keybind
{
public:
	Keybind(std::string nickname, std::string displayName, std::string category, Keyset ks, bool saveToConfig)
		: Keybind(nickname, displayName, category, ks.first, ks.second, saveToConfig) {}

	Keybind(std::string nickname, std::string displayName, std::string category, ScanCode key, Modifier mod, bool saveToConfig);
	Keybind(std::string nickname, std::string displayName, std::string category);
	~Keybind();

	const Keyset& keySet() const { return { key_, mod_ }; }
	ScanCode key() const { return key_; }
	Modifier modifier() const { return mod_; }

	void keySet(const Keyset& ks) {
		if (!isBeingModified_)
			return; 
		key_ = ks.first;
		mod_ = ks.second;

		ApplyKeys();
	}
	void key(ScanCode key) {
		if (!isBeingModified_)
			return;
		key_ = key;

		ApplyKeys();
	}
	void modifier(Modifier mod) {
		if (!isBeingModified_)
			return; 
		mod_ = mod;

		ApplyKeys();
	}

	void ParseKeys(const char* keys);
	void ParseConfig(const char* keys);

	[[nodiscard]] const std::string& displayName() const { return displayName_; }
	void displayName(const std::string& n) { displayName_ = n; }

	[[nodiscard]] const std::string& nickname() const { return nickname_; }
	void nickname(const std::string& n) { nickname_ = n; }

	[[nodiscard]] bool isSet() const { return key_ != ScanCode::NONE; }

	[[nodiscard]] bool isBeingModified() const { return isBeingModified_; }
	void isBeingModified(bool ibm) { isBeingModified_ = ibm; }

	[[nodiscard]] const char* keysDisplayString() const { return keysDisplayString_.data(); }
	std::array<char, 256>& keysDisplayStringArray() { return keysDisplayString_; }
	
	[[nodiscard]] bool matches(const Keyset& ks) const;
	[[nodiscard]] bool matchesPartial(const Keyset& ks) const;
	[[nodiscard]] bool matchesNoLeftRight(const Keyset& ks) const;

	static void ForceRefreshDisplayStrings();

protected:
	void UpdateDisplayString();
	virtual void ApplyKeys();

	std::string displayName_, nickname_, category_;
	std::array<char, 256> keysDisplayString_ { };
	bool isBeingModified_ = true;
	ScanCode key_;
	Modifier mod_;
	bool saveToConfig_ = true;

	static std::set<Keybind*> keybinds_;
};

}
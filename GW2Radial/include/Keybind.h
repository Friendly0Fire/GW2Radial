#pragma once
#include <Main.h>
#include <array>
#include <set>
#include <Input.h>

namespace GW2Radial
{

using KeyCombo = std::pair<ScanCode, Modifier>;

class Keybind
{
public:
	Keybind(std::string nickname, std::string displayName, std::string category, KeyCombo ks, bool saveToConfig)
		: Keybind(nickname, displayName, category, ks.first, ks.second, saveToConfig) {}

	Keybind(std::string nickname, std::string displayName, std::string category, ScanCode key, Modifier mod, bool saveToConfig);
	Keybind(std::string nickname, std::string displayName, std::string category);
	~Keybind();

	const KeyCombo& keyCombo() const { return { key_, mod_ }; }
	ScanCode key() const { return key_; }
	Modifier modifier() const { return mod_; }

	void keyCombo(const KeyCombo& ks) {
		tie(key_, mod_) = ks;

		ApplyKeys();
	}
	void key(ScanCode key) {
		key_ = key;

		ApplyKeys();
	}
	void modifier(Modifier mod) {
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

	[[nodiscard]] const char* keysDisplayString() const { return keysDisplayString_.data(); }
	
	[[nodiscard]] bool matches(const KeyCombo& ks) const;
	[[nodiscard]] bool matchesPartial(const KeyCombo& ks) const;
	[[nodiscard]] bool matchesNoLeftRight(const KeyCombo& ks) const;

	static void ForceRefreshDisplayStrings();

protected:
	void UpdateDisplayString() const;
	void ApplyKeys() const;

	std::string displayName_, nickname_, category_;
	ScanCode key_;
	Modifier mod_;
	bool saveToConfig_ = true;

	mutable std::array<char, 256> keysDisplayString_ { };

	static std::set<Keybind*> keybinds_;
};

}
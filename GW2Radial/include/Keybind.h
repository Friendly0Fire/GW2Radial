#pragma once
#include <Main.h>
#include <array>
#include <set>
#include <Input.h>

namespace GW2Radial
{

struct KeyCombo
{
public:
	ScanCode& key = key_;
	Modifier& mod = mod_;

	KeyCombo() {
		key = ScanCode::NONE;
		mod = Modifier::NONE;
	}
	KeyCombo(ScanCode k, Modifier m) : key(k), mod(m) { }

	operator std::tuple<ScanCode, Modifier>() const
	{
		return std::make_tuple(key, mod);
	}

private:
	union {
		uint64_t storage_;
		struct {
			ScanCode key_;
			Modifier mod_;
		};
	};

	friend std::partial_ordering operator<=>(const KeyCombo& a, const KeyCombo& b);
};

std::partial_ordering operator<=>(const KeyCombo& a, const KeyCombo& b)
{
	return a.storage_ <=> b.storage_;
}

class Keybind
{
public:
	Keybind(std::string nickname, std::string displayName, std::string category, KeyCombo ks, bool saveToConfig)
		: Keybind(nickname, displayName, category, ks.key, ks.mod, saveToConfig) {}

	Keybind(std::string nickname, std::string displayName, std::string category, ScanCode key, Modifier mod, bool saveToConfig);
	Keybind(std::string nickname, std::string displayName, std::string category);

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
	[[nodiscard]] char* keysDisplayString() { return keysDisplayString_.data(); }
	[[nodiscard]] const size_t keysDisplayStringSize() const { return keysDisplayString_.size(); }

	[[nodiscard]] bool matches(const KeyCombo& ks) const;
	[[nodiscard]] bool matches(const std::set<ScanCode>& ks) const;
	[[nodiscard]] float matchesScored(const std::set<ScanCode>& ks) const;
	//[[nodiscard]] bool matchesPartial(const std::set<ScanCode>& ks) const;
	//[[nodiscard]] bool matchesNoLeftRight(const KeyCombo& ks) const;

	int keyCount() const {
		return 1 + (notNone(mod_ & Modifier::CTRL) ? 1 : 0)
			     + (notNone(mod_ & Modifier::SHIFT) ? 1 : 0)
			     + (notNone(mod_ & Modifier::ALT) ? 1 : 0);
	}

protected:
	void UpdateDisplayString() const;
	void ApplyKeys() const;

	std::string displayName_, nickname_, category_;
	ScanCode key_;
	Modifier mod_;
	bool saveToConfig_ = true;

	mutable std::array<char, 256> keysDisplayString_ { };
};

}
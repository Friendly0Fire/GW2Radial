#pragma once
#include <Main.h>
#include <array>
#include <set>
#include <functional>

namespace GW2Addons
{

class Keybind
{
public:
	Keybind(std::string displayName, std::string nickname, const std::set<uint>& keys);
	Keybind(std::string displayName, std::string nickname, const char* keys);
	Keybind(std::string displayName, std::string nickname);

	const std::set<uint>& keys() const { return keys_; }
	void keys(const std::set<uint>& keys);
	void keys(const char* keys);

	const std::string& displayName() const { return displayName_; }
	void displayName(const std::string& n) { displayName_ = n; }

	const std::string& nickname() const { return nickname_; }
	void nickname(const std::string& n) { nickname_ = n; }

	bool isSet() const { return !keys_.empty(); }

	bool isBeingModified() const { return isBeingModified_; }
	void isBeingModified(bool ibm) { isBeingModified_ = ibm; }

	const char* keysDisplayString() const { return keysDisplayString_.data(); }
	std::array<char, 256>& keysDisplayStringArray() { return keysDisplayString_; }

protected:
	void UpdateDisplayString();
	void ApplyKeys();

	std::string displayName_, nickname_;
	std::array<char, 256> keysDisplayString_ { };
	bool isBeingModified_ = false;
	std::set<uint> keys_;
};

}
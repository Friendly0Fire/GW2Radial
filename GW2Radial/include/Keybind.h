#pragma once
#include <Main.h>
#include <array>
#include <set>
#include <functional>
#include <unordered_map>

namespace GW2Radial
{

class Keybind
{
public:
	Keybind(std::string nickname, std::string displayName, const std::set<uint>& keys, bool saveToConfig);
	Keybind(std::string nickname, std::string displayName);
	~Keybind();

	const std::set<uint>& keys() const { return keys_; }
	void keys(const std::set<uint>& keys);
	void keys(const char* keys);

	const std::string& displayName() const { return displayName_; }
	void displayName(const std::string& n) { displayName_ = n; }

	const std::string& nickname() const { return nickname_; }
	void nickname(const std::string& n) { nickname_ = n; }

	bool isSet() const { return !keys_.empty(); }
	bool isConflicted() const { return isConflicted_; }

	bool isBeingModified() const { return isBeingModified_; }
	void isBeingModified(bool ibm) { isBeingModified_ = ibm; }

	const char* keysDisplayString() const { return keysDisplayString_.data(); }
	std::array<char, 256>& keysDisplayStringArray() { return keysDisplayString_; }

	bool conflicts(const std::set<uint>& pressedKeys) const;

protected:
	void UpdateDisplayString();
	void ApplyKeys();
	void CheckForConflict(bool recurse = true);

	std::string displayName_, nickname_;
	std::array<char, 256> keysDisplayString_ { };
	bool isBeingModified_ = true;
	std::set<uint> keys_;
	bool saveToConfig_ = true;
	bool isConflicted_ = false;

	static std::unordered_map<Keybind*, std::set<uint>> keyMaps_;
};

}
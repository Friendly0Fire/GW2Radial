#pragma once
#include <Main.h>
#include <set>
#include <Keybind.h>
#include <Condition.h>

namespace GW2Radial
{

class ActivationKeybind : public Keybind
{
public:
	using PreventPassToGame = bool;
	using Activated = bool;
	using Callback = std::function<PreventPassToGame(Activated)>;
	ActivationKeybind(std::string nickname, std::string displayName, std::string category, KeyCombo ks, bool saveToConfig)
		: Keybind(nickname, displayName, category, ks.key, ks.mod, saveToConfig) {
		Bind();
	}

	ActivationKeybind(std::string nickname, std::string displayName, std::string category, ScanCode key, Modifier mod, bool saveToConfig)
		: Keybind(nickname, displayName, category, key, mod, saveToConfig) {
		Bind();
	}
	ActivationKeybind(std::string nickname, std::string displayName, std::string category)
		: Keybind(nickname, displayName, category) {
		Bind();
	}
	~ActivationKeybind();

	void callback(Callback&& cb) { callback_ = std::move(cb); }
	Callback callback() const { return callback_; }
	void conditions(ConditionSetPtr ptr) { conditions_ = ptr; }

	[[nodiscard]] bool conditionsFulfilled() const { return conditions_->passes(); }

protected:
	void Bind();
	ConditionSetPtr conditions_;
	Callback callback_;
};

}
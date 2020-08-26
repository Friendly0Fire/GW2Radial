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
	using Keybind::Keybind;

	[[nodiscard]] bool isConflicted() const { return isConflicted_; }

	[[nodiscard]] bool conflicts(const std::set<ScanCode>& scanCodes) const;

	[[nodiscard]] bool conditionsFulfilled() const { return conditions_(); }

protected:
	void ApplyKeys() override;
	void CheckForConflict(bool recurse = true);
	
	bool isConflicted_ = false;
	ConditionSet conditions_;
};

}
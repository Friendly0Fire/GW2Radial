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

	void conditions(ConditionSetPtr ptr) { conditions_ = ptr; }

	[[nodiscard]] bool conditionsFulfilled() const { return conditions_->passes(); }

protected:
	ConditionSetPtr conditions_;
};

}
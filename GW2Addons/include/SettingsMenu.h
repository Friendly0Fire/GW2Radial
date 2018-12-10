#pragma once

#include <Main.h>
#include <Singleton.h>
#include <list>
#include <Keybind.h>
#include <Input.h>

namespace GW2Addons
{

class SettingsMenu : public Singleton<SettingsMenu>
{
public:
	class Implementer
	{
	public:
		virtual void DrawMenu() = 0;
	};

	SettingsMenu();

	void Draw();

	const Keybind& showKeybind() const { return showKeybind_; }
	
	void AddImplementer(Implementer* impl) { implementers_.push_back(impl); }
	void RemoveImplementer(Implementer* impl) { implementers_.remove(impl); }

protected:
	InputResponse OnInputChange(bool changed, const std::set<uint>& keys, const std::list<EventKey>& changedKeys);

	std::list<Implementer*> implementers_;

	bool isVisible_ = false;
	Keybind showKeybind_;
	
	Input::InputChangeCallback inputChangeCallback_;
};

}
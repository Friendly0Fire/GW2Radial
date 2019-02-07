#pragma once

#include <Main.h>
#include <Singleton.h>
#include <list>
#include <Keybind.h>
#include <Input.h>

namespace GW2Radial
{

class SettingsMenu : public Singleton<SettingsMenu>
{
public:
	class Implementer
	{
	public:
		virtual const char* GetTabName() const = 0;
		virtual void DrawMenu() = 0;
	};

	SettingsMenu();

	void Draw();

	const Keybind& showKeybind() const { return showKeybind_; }
	
	void AddImplementer(Implementer* impl) { implementers_.push_back(impl); if (!currentTab_) currentTab_ = impl; }
	void RemoveImplementer(Implementer* impl) { implementers_.remove(impl); }

protected:
	InputResponse OnInputChange(bool changed, const std::set<uint>& keys, const std::list<EventKey>& changedKeys);

	std::list<Implementer*> implementers_;
	Implementer* currentTab_ = nullptr;

	bool isVisible_ = false;
	Keybind showKeybind_;
	
	Input::InputChangeCallback inputChangeCallback_;
};

}
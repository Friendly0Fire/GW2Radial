#pragma once

#include <Main.h>
#include <Singleton.h>
#include <list>
#include <ActivationKeybind.h>
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
		virtual void DrawMenu(Keybind* currentEditedKeybind) = 0;
		virtual bool visible() { return true; }
	};

	SettingsMenu();

	void Draw();

	const Keybind& showKeybind() const { return showKeybind_; }
	
	void AddImplementer(Implementer* impl) { implementers_.push_back(impl); if (!currentTab_) currentTab_ = impl; }
	void RemoveImplementer(Implementer* impl) { implementers_.remove(impl); if(currentTab_ == impl) currentTab_ = nullptr; }

	bool isVisible() const { return isVisible_; }

protected:
	std::list<Implementer*> implementers_;
	Implementer* currentTab_ = nullptr;

	bool isVisible_ = false;
	bool isFocused_ = false;
	ActivationKeybind showKeybind_;
	Keybind* currentEditedKeybind_ = nullptr;
	ScanCode allowThroughAlt_ = ScanCode::NONE;
	ScanCode allowThroughShift_ = ScanCode::NONE;
};

}
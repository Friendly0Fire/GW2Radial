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
		Implementer() = default;
		virtual void DrawMenu() = 0;
		virtual ~Implementer() = default;
	};

	SettingsMenu();

	void Draw();

	const Keybind& showKeybind() const { return showKeybind_; }

protected:
	InputResponse OnInputChange(bool changed, const std::set<uint>& keys);

	std::list<Implementer*> implementers_;

	bool isVisible_ = false;
	Keybind showKeybind_;
};

}
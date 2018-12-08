#pragma once

#include <Main.h>
#include <Singleton.h>
#include <list>

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

	void Draw();

protected:
	std::list<Implementer*> implementers_;

	bool isVisible_ = false;
};

}
#pragma once
#include <SettingsMenu.h>
#include <Singleton.h>

namespace GW2Radial
{

class MiscTab : public SettingsMenu::Implementer, public Singleton<MiscTab>
{
	bool reloadOnFocus_ = false;
public:
	MiscTab();
	~MiscTab();

	const char * GetTabName() const override { return "Misc"; }
	void DrawMenu(Keybind*) override;

	bool reloadOnFocus() const { return reloadOnFocus_; }
};

}
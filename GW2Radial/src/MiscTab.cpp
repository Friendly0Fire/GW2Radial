#include <MiscTab.h>
#include <UpdateCheck.h>
#include <ImGuiExtensions.h>
#include <imgui/imgui.h>
#include <Core.h>
#include <GFXSettings.h>
#include <MumbleLink.h>

namespace GW2Radial
{

MiscTab::MiscTab()
{
	SettingsMenu::i().AddImplementer(this);
}

MiscTab::~MiscTab()
{
	SettingsMenu::i([&](auto& i) { i.RemoveImplementer(this); });
}
void MiscTab::DrawMenu(Keybind*)
{
	ImGuiTitle("General");

	ImGuiConfigurationWrapper(ImGui::Checkbox, UpdateCheck::i().checkEnabled_);
	
	ImGuiTitle("Toggle Menu Visibility");

	for(auto& wheel : Core::i().wheels()) {
	    ImGuiConfigurationWrapper(ImGui::Checkbox, wheel->visibleInMenuOption());
	}

	ImGuiTitle("Custom Wheel Tools");

	ImGui::Checkbox("Reload custom wheels on focus", &reloadOnFocus_);

	if(ImGui::Button("Reload custom wheels"))
		Core::i().ForceReloadWheels();

#ifdef _DEBUG
	cref pos = MumbleLink::i().position();
	ImGui::Text("position = %f, %f, %f", pos.x, pos.y, pos.z);

	bool dpiScaling = GFXSettings::i().dpiScaling();
	ImGui::Text(dpiScaling ? "DPI scaling enabled" : "DPI scaling disabled");
#endif
}

}

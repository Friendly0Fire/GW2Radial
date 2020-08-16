#include <MiscTab.h>
#include <UpdateCheck.h>
#include <ImGuiExtensions.h>
#include <imgui/imgui.h>
#include <Core.h>
#include <GFXSettings.h>
#include <MumbleLink.h>

namespace GW2Radial
{
DEFINE_SINGLETON(MiscTab);

MiscTab::MiscTab()
{
	SettingsMenu::i()->AddImplementer(this);
}

MiscTab::~MiscTab()
{
	if(auto i = SettingsMenu::iNoInit(); i)
		i->RemoveImplementer(this);
}
void MiscTab::DrawMenu()
{
	if(auto uc = UpdateCheck::iNoInit(); uc)
		ImGuiConfigurationWrapper(ImGui::Checkbox, uc->checkEnabled_);

	ImGui::Checkbox("Reload custom wheels on focus", &reloadOnFocus_);

#ifdef _DEBUG
	cref pos = MumbleLink::i()->position();
	ImGui::Text("position = %f, %f, %f", pos.x, pos.y, pos.z);

	bool dpiScaling = GFXSettings::i()->dpiScaling();
	ImGui::Text(dpiScaling ? "DPI scaling enabled" : "DPI scaling disabled");
#endif

	if(ImGui::Button("Reload custom wheels"))
		Core::i()->ForceReloadWheels();
}

}

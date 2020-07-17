#include <MiscTab.h>
#include <UpdateCheck.h>
#include <ImGuiExtensions.h>
#include <imgui/imgui.h>
#include <Core.h>

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

	if(ImGui::Button("Reload custom wheels"))
		Core::i()->ForceReloadWheels();
}

}

#include <MiscTab.h>
#include <UpdateCheck.h>
#include <ImGuiExtensions.h>
#include <imgui/imgui.h>
#include <Utility.h>
#include <Input.h>

namespace GW2Radial
{
DEFINE_SINGLETON(MiscTab);

MiscTab::MiscTab()
    : reloadOnFocus_("Reload custom wheels on focus", "reload_focus", "General", false)
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

	ImGuiConfigurationWrapper(ImGui::Checkbox, reloadOnFocus_);
}

}

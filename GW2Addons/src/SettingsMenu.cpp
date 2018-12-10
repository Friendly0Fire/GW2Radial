#include <SettingsMenu.h>
#include <imgui.h>
#include <ImGuiExtensions.h>
#include <examples/imgui_impl_dx9.h>

namespace GW2Addons
{
DEFINE_SINGLETON(SettingsMenu);

SettingsMenu::SettingsMenu()
	: showKeybind_("show_settings", "Show settings", { VK_SHIFT, VK_MENU, 'M' }, false)
{
	inputChangeCallback_ = [this](bool changed, const std::set<uint>& keys, const std::list<EventKey>& changedKeys) { return OnInputChange(changed, keys, changedKeys); };
	Input::i()->AddInputChangeCallback(&inputChangeCallback_);
}
void SettingsMenu::Draw()
{
	if (isVisible_)
	{
		if(!ImGui::Begin("GW2Addons Options Menu", &isVisible_))
			return;

		ImGui::PushItemWidth(-1);
	
		for(const auto& i : implementers_)
			i->DrawMenu();
		
		ImGui::PopItemWidth();

		ImGui::End();

		if (!ConfigurationFile::i()->lastSaveError().empty() && !ImGui::IsPopupOpen("Configuration Update Error") && ConfigurationFile::i()->lastSaveErrorChanged())
			ImGui::OpenPopup("Configuration Update Error");

		if (ImGui::BeginPopup("Configuration Update Error"))
		{
			ImGui::Text("Could not save addon configuration. Reason given was:");
			ImGui::TextWrapped(ConfigurationFile::i()->lastSaveError().c_str());
			if (ImGui::Button("OK"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

	}
}

InputResponse SettingsMenu::OnInputChange(bool changed, const std::set<uint>& keys, const std::list<EventKey>& changedKeys)
{
	const bool isMenuKeybind = keys == showKeybind_.keys();
	if(isMenuKeybind)
		isVisible_ = true;

	return isMenuKeybind ? InputResponse::PREVENT_ALL : InputResponse::PASS_TO_GAME;
}

}
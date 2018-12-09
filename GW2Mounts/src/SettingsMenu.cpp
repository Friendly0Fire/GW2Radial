#include <SettingsMenu.h>
#include <imgui.h>
#include <ImGuiExtensions.h>
#include <examples/imgui_impl_dx9.h>

namespace GW2Addons
{
	SettingsMenu::SettingsMenu()
		: showKeybind_("Show settings", "show_settings", { VK_SHIFT, VK_MENU, 'M' })
	{
		Input::i()->AddInputChangeCallback([this](bool changed, const std::set<uint>& keys, const std::list<EventKey>& changedKeys) { return OnInputChange(changed, keys, changedKeys); });
	}
	void SettingsMenu::Draw()
	{
		if (isVisible_)
		{
			if(!ImGui::Begin("Mounts Options Menu", &isVisible_))
				return;
		
			for(const auto& i : implementers_)
				i->DrawMenu();

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

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	InputResponse SettingsMenu::OnInputChange(bool changed, const std::set<uint>& keys, const std::list<EventKey>& changedKeys)
	{
		const bool isMenuKeybind = keys == showKeybind_.keys();
		return isMenuKeybind ? InputResponse::PREVENT_ALL : InputResponse::PASS_TO_GAME;
	}
}
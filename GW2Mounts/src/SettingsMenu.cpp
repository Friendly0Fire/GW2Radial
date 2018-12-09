#include <SettingsMenu.h>
#include <imgui.h>
#include <ImGuiExtensions.h>
#include <examples/imgui_impl_dx9.h>

namespace GW2Addons
{
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
}
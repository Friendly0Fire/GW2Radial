#include <SettingsMenu.h>

namespace GW2Addons
{
	void SettingsMenu::Draw()
	{
		if (DisplayOptionsWindow)
		{
			ImGui::Begin("Mounts Options Menu", &DisplayOptionsWindow);

			

			ImGui::Separator();

			ImGuiKeybindInput("Overlay Keybind", MainKeybind);
			ImGuiKeybindInput("Overlay Keybind (Center Locked)", MainLockedKeybind);

			ImGui::Separator();
			ImGui::Text("Mount Keybinds");
			ImGui::Text("(set to relevant game keybinds)");

			for (uint i = 0; i < MountTypeCount; i++)
				ImGuiKeybindInput(GetMountName((MountType)i), MountKeybinds[i]);

			ImGui::End();

			if (!Cfg.LastSaveError().empty() && !ImGui::IsPopupOpen("Configuration Update Error") && Cfg.LastSaveErrorChanged())
				ImGui::OpenPopup("Configuration Update Error");

			if (ImGui::BeginPopup("Configuration Update Error"))
			{
				ImGui::Text("Could not save addon configuration. Reason given was:");
				ImGui::TextWrapped(Cfg.LastSaveError().c_str());
				if (ImGui::Button("OK"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
		}

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		for(const auto& i : implementers_)
			i->DrawMenu();
	}
}
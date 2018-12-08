#include <SettingsMenu.h>

namespace GW2Addons
{
	void SettingsMenu::Draw()
	{
		if (DisplayOptionsWindow)
		{
			ImGui::Begin("Mounts Options Menu", &DisplayOptionsWindow);

			if (ImGui::SliderInt("Pop-up Delay", &Cfg.OverlayDelayMilliseconds(), 0, 1000, "%d ms"))
				Cfg.OverlayDelayMillisecondsSave();

			if (ImGui::SliderFloat("Overlay Scale", &Cfg.OverlayScale(), 0.f, 4.f))
				Cfg.OverlayScaleSave();

			if (ImGui::SliderFloat("Overlay Dead Zone Scale", &Cfg.OverlayDeadZoneScale(), 0.f, 0.25f))
				Cfg.OverlayDeadZoneScaleSave();

			ImGui::Text("Overlay Dead Zone Behavior:");
			int oldBehavior = Cfg.OverlayDeadZoneBehavior();
			ImGui::RadioButton("Nothing", &Cfg.OverlayDeadZoneBehavior(), 0);
			ImGui::RadioButton("Last Mount", &Cfg.OverlayDeadZoneBehavior(), 1);
			ImGui::RadioButton("Favorite Mount", &Cfg.OverlayDeadZoneBehavior(), 2);
			if (oldBehavior != Cfg.OverlayDeadZoneBehavior())
				Cfg.OverlayDeadZoneBehaviorSave();

			if (Cfg.OverlayDeadZoneBehavior() == 2)
			{
				if (FavoriteMountNames[0] == nullptr)
				{
					auto mounts = GetAllMounts();
					for (uint i = 0; i < mounts.size(); i++)
						FavoriteMountNames[i] = GetMountName(mounts[i]);
				}

				if (ImGui::Combo("Favorite Mount", (int*)&Cfg.FavoriteMount(), FavoriteMountNames.data(), (int)FavoriteMountNames.size()))
					Cfg.FavoriteMountSave();
			}

			if (ImGui::Checkbox("Reset cursor to center with Center Locked keybind", &Cfg.ResetCursorOnLockedKeybind()))
				Cfg.ResetCursorOnLockedKeybindSave();
			if (ImGui::Checkbox("Lock camera when overlay is displayed", &Cfg.LockCameraWhenOverlayed()))
				Cfg.LockCameraWhenOverlayedSave();

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
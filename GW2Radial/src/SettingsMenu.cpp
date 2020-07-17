#include <SettingsMenu.h>
#include <imgui.h>
#include <ImGuiExtensions.h>
#include <UpdateCheck.h>

namespace GW2Radial
{
DEFINE_SINGLETON(SettingsMenu);

SettingsMenu::SettingsMenu()
	: showKeybind_("show_settings", "Show settings", "__core__", true, { ScanCode::SHIFT, ScanCode::ALT, ScanCode::M }, false)
{
	inputChangeCallback_ = [this](bool changed, const std::set<ScanCode>& scs, const std::list<EventKey>& changedKeys) { return OnInputChange(changed, scs, changedKeys); };
	Input::i()->AddInputChangeCallback(&inputChangeCallback_);
}
void SettingsMenu::Draw()
{
	if (isVisible_)
	{
		ImGui::SetNextWindowSize({ 750, 600 }, ImGuiCond_::ImGuiCond_FirstUseEver);
		if(!ImGui::Begin("GW2Radial Options Menu", &isVisible_))
		{
			ImGui::End();
			return;
		}

		ImGui::PushItemWidth(-1);
	
		if (!implementers_.empty())
		{
			// Uncomment to make tabs fill the entire line
			// float tabWidth = (ImGui::GetContentRegionAvailWidth() - ImGui::GetStyle().ItemSpacing.x * (implementers_.size() + 1)) / implementers_.size();
			float tabWidth = 0;
			auto currentTabColor = ImGui::GetStyleColorVec4(ImGuiCol_::ImGuiCol_Button);
			auto activeTabColor = ImGui::GetStyleColorVec4(ImGuiCol_::ImGuiCol_ButtonActive);
			auto hoveredTabColor = ImGui::GetStyleColorVec4(ImGuiCol_::ImGuiCol_ButtonHovered);
			auto inactiveTabColor = currentTabColor;
			inactiveTabColor.w /= 4;

			// Draw buttons simulating tabs
			for (cref i : implementers_)
			{
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, currentTab_ == i ? currentTabColor : inactiveTabColor);
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, currentTab_ == i ? currentTabColor : activeTabColor);
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, currentTab_ == i ? currentTabColor : hoveredTabColor);
				ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_FramePadding, { 10, ImGui::GetStyle().FramePadding.y });
				if (ImGui::Button(i->GetTabName(), { tabWidth, 0 }))
					currentTab_ = i;
				ImGui::PopStyleVar();
				ImGui::PopStyleColor(3);
			}

			// Draw a fake line below the tabs to make the buttons look more like actual tabs
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, currentTabColor);
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, currentTabColor);
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, currentTabColor);
			ImGui::Button("", { ImGui::GetContentRegionAvailWidth(), 2 });
			ImGui::PopStyleColor(3);

			// Draw selected tab's contents inside its own scrolling frame
			if (currentTab_)
			{
				ImGui::BeginChild((std::string("CurrentTab") + currentTab_->GetTabName()).c_str());
				currentTab_->DrawMenu();
				ImGui::EndChild();
			}
		}
		
		ImGui::PopItemWidth();

		ImGui::End();
	}
}

InputResponse SettingsMenu::OnInputChange(bool /*changed*/, const std::set<ScanCode>& scs, const std::list<EventKey>& /*changedKeys*/)
{
	const bool isMenuKeybind = showKeybind_.matchesNoLeftRight(scs);
	if (isMenuKeybind) {
		isVisible_ = true;
		Keybind::ForceRefreshDisplayStrings();
	}

	return isMenuKeybind ? InputResponse::PREVENT_ALL : InputResponse::PASS_TO_GAME;
}

}
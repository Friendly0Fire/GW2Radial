#include <SettingsMenu.h>
#include <imgui.h>
#include <ImGuiExtensions.h>
#include <imgui_internal.h>
#include <UpdateCheck.h>

namespace GW2Radial
{
DEFINE_SINGLETON(SettingsMenu);

SettingsMenu::SettingsMenu()
	: showKeybind_("show_settings", "Show settings", "__core__", { ScanCode::SHIFT, ScanCode::ALT, GetScanCodeFromVirtualKey('M') }, false)
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
	
		if (!implementers_.empty())
		{
			if(currentTab_ == nullptr)
				currentTab_ = implementers_.front();

			if(ImGui::BeginTabBar("GW2RadialMainTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyScroll)) {
			    for (cref i : implementers_)
			    {
					if(!i->visible())
						continue;

				    if(ImGui::BeginTabItem(i->GetTabName(), nullptr, 0)) {
					    currentTab_ = i;
					    i->DrawMenu();
				        ImGui::EndTabItem();
				    }
			    }
				
			    ImGui::EndTabBar();
			}
		}

		ImGui::End();
	}
}

InputResponse SettingsMenu::OnInputChange(bool /*changed*/, const std::set<ScanCode>& scs, const std::list<EventKey>& /*changedKeys*/)
{
	const bool isMenuKeybind = showKeybind_.matchesNoLeftRight(scs);
	if (isMenuKeybind) {
		if(isVisible_)
			isVisible_ = false;
		else {
		    isVisible_ = true;
		    Keybind::ForceRefreshDisplayStrings();
		}
	}

	return isMenuKeybind ? InputResponse::PREVENT_ALL : InputResponse::PASS_TO_GAME;
}

}
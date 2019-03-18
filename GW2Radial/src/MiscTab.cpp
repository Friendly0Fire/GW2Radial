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
		ImGuiConfigurationWrapper(ImGui::Checkbox, "Automatically check for updates", uc->checkEnabled_);
	if(auto i = Input::iNoInit(); i)
		ImGuiConfigurationWrapper(ImGui::Checkbox, "Distinguish between left and right SHIFT/CTRL/ALT", i->distinguishLeftRight_);

#if 0
	ImGui::Separator();

	ImGui::Text("Use these buttons to bind the respective key in your game's settings menu:");

	ImGui::InputInt("Virtual Key", reinterpret_cast<int*>(&vk_), 1, 100, ImGuiInputTextFlags_CharsHexadecimal);

	ImGui::Text(utf8_encode(GetKeyName(vk_)).c_str());

	if(ImGui::Button("Send"))
	{
		Input::i()->SendKeybind({ vk_ });
	}
#elif 0
	const auto btnWidth = ImGui::GetWindowWidth() * 0.25f - ImGui::GetStyle().FramePadding.x * 2;

	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			const uint vk = VK_F13 + i * 4 + j;
			if(ImGui::Button(utf8_encode(GetKeyName(vk)).c_str(), ImVec2(btnWidth, 0)))
			{
				Input::i()->SendKeybind({ vk });
			}

			if(j < 3)
				ImGui::SameLine();
		}
	}
#endif
}

}

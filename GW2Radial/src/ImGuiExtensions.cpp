#include <ImGuiExtensions.h>
#include <Core.h>

ImVec4 operator/(const ImVec4& v, const float f)
{
	return { v.x / f, v.y / f, v.z / f, v.w / f };
}

void ImGuiKeybindInput(GW2Radial::Keybind& setting)
{
	std::string suffix = "##" + setting.nickname();

	const auto windowWidth = ImGui::GetWindowWidth();

	ImGui::PushItemWidth(windowWidth * 0.4f);

	int popcount = 1;
	if (setting.isBeingModified())
	{
		popcount = 3;
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(201, 215, 255, 200) / 255.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0, 0, 0, 1));
	}
	else
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1));

	ImGui::InputText(suffix.c_str(), setting.keysDisplayStringArray().data(), setting.keysDisplayStringArray().size(), ImGuiInputTextFlags_ReadOnly);

	ImGui::PopItemWidth();

	ImGui::PopStyleColor(popcount);

	ImGui::SameLine();

	if (!setting.isBeingModified() && ImGui::Button(("Set" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)))
	{
		setting.isBeingModified(true);
		setting.keysDisplayStringArray().at(0) = '\0';
	}
	else if (setting.isBeingModified() && ImGui::Button(("Clear" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)))
	{
		setting.isBeingModified(false);
		setting.keysDisplayStringArray().at(0) = '\0';
		setting.keys(std::set<uint>());
	}

	ImGui::SameLine();

	ImGui::PushItemWidth(windowWidth * 0.5f);

	if(setting.isConflicted())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, 0xFF0000FF);
		ImGui::Text((setting.displayName() + "[!]").c_str());
		ImGui::PopStyleColor();
	}
	else
		ImGui::Text(setting.displayName().c_str());

	ImGui::PopItemWidth();
}

void ImGuiTitle(const char * text)
{
	ImGui::PushFont(GW2Radial::Core::i()->fontBlack());
	ImGui::TextUnformatted(text);
	ImGui::Separator();
	ImGuiSpacing();
	ImGui::PopFont();
}

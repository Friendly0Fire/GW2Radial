#include "imgui_ext.h"
#include "Utility.h"

void ImGuiKeybind::UpdateDisplayString(const std::set<uint>& keys)
{
	std::string keybind = "";
	for (const auto& k : keys)
	{
		keybind += GetKeyName(k) + std::string(" + ");
	}

	strcpy_s(DisplayString, (keybind.size() > 0 ? keybind.substr(0, keybind.size() - 3) : keybind).c_str());
}

void ImGuiKeybind::UpdateKeybind(const std::set<uint>& keys, bool apply)
{
	if (IsBeingModified)
	{
		UpdateDisplayString(keys);
		if (apply)
		{
			IsBeingModified = false;
			SetCallback(keys);
		}
	}
}

ImVec4 operator/(const ImVec4& v, float f)
{
	return ImVec4(v.x / f, v.y / f, v.z / f, v.w / f);
}

void ImGuiKeybindInput(const std::string& name, ImGuiKeybind& setting)
{
	std::string suffix = "##" + name;

	float windowWidth = ImGui::GetWindowWidth();

	ImGui::PushItemWidth(windowWidth * 0.3f);

	int popcount = 1;
	if (setting.IsBeingModified)
	{
		popcount = 3;
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(201, 215, 255, 200) / 255.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0, 0, 0, 1));
	}
	else
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1));

	ImGui::InputText(suffix.c_str(), setting.DisplayString, 256, ImGuiInputTextFlags_ReadOnly);

	ImGui::PopItemWidth();

	ImGui::PopStyleColor(popcount);

	ImGui::SameLine();

	if (!setting.IsBeingModified && ImGui::Button(("Set" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)))
	{
		setting.IsBeingModified = true;
		setting.DisplayString[0] = '\0';
	}
	else if (setting.IsBeingModified && ImGui::Button(("Clear" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)))
	{
		setting.IsBeingModified = false;
		setting.DisplayString[0] = '\0';
		setting.SetCallback(std::set<uint>());
	}

	ImGui::SameLine();

	ImGui::PushItemWidth(windowWidth * 0.5f);

	ImGui::Text(name.c_str());

	ImGui::PopItemWidth();
}
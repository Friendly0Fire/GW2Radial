#include <ImGuiExtensions.h>
#include <Core.h>
#include "../imgui/imgui_internal.h"
#include <Input.h>

ImVec2 operator*(const ImVec2& a, const ImVec2& b)
{
	return ImVec2(a.x * b.x, a.y * b.y);
}
ImVec2 operator*(const ImVec2& a, float b)
{
	return ImVec2(a.x * b, a.y * b);
}
ImVec2 operator-(const ImVec2& a, const ImVec2& b)
{
	return ImVec2(a.x - b.x, a.y - b.y);
}

ImVec2 operator*=(ImVec2 & a, const ImVec2 & b)
{
	a = a * b;
	return a;
}

ImVec4 operator/(const ImVec4& v, const float f)
{
	return { v.x / f, v.y / f, v.z / f, v.w / f };
}

std::map<void*, int> specialIds;

void ImGuiKeybindInput(GW2Radial::Keybind& setting)
{
	std::string suffix = "##" + setting.nickname();

	const auto windowWidth = ImGui::GetWindowWidth();

#if 0
	ImGui::PushItemWidth(windowWidth * (setting.isBeingModified() ? 0.3f : 0.45f));
#endif
	ImGui::PushItemWidth(windowWidth * 0.45f);

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

#if 0
	if(setting.isBeingModified())
	{
		ImGui::PushItemWidth(windowWidth * 0.15f - ImGui::GetStyle().FramePadding.x * 2);

		int& specialId = specialIds[&setting];
		if(ImGui::Combo(("##specialkey" + suffix).c_str(), &specialId, "F13\0F14\0F15\0F16\0F17\0F18\0F19\0F20\0F21\0F22\0F23\0F24\0"))
		{
			const uint vk = VK_F13 + specialId;
			setting.keys({ vk });
			setting.isBeingModified(false);
			specialId = 0;
		}

		ImGui::PopItemWidth();

		ImGui::SameLine();
	}
#endif

	if (!setting.isBeingModified() && ImGui::Button(("Set" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)))
	{
		setting.isBeingModified(true);
		setting.keysDisplayStringArray().at(0) = '\0';
	}
	else if (setting.isBeingModified() && ImGui::Button(("Clear" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)))
	{
		setting.isBeingModified(false);
		setting.scanCodes(std::set<GW2Radial::ScanCode>());
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

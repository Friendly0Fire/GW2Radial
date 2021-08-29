#include <ActivationKeybind.h>
#include <ImGuiExtensions.h>
#include <Core.h>
#include "../imgui/imgui_internal.h"
#include <Input.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

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

bool ImGuiKeybindInput(GW2Radial::Keybind& setting, bool beingModified, const char* tooltip)
{
	std::string suffix = "##" + setting.nickname();

	float windowWidth = ImGui::GetWindowWidth() - ImGuiHelpTooltipSize();

	ImGui::PushItemWidth(windowWidth * 0.45f);

	int popcount = 1;
	if (beingModified)
	{
		popcount = 3;
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(201, 215, 255, 200) / 255.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0, 0, 0, 1));
	}
	else
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1));

	ImGui::InputText(suffix.c_str(), setting.keysDisplayString(), setting.keysDisplayStringSize(), ImGuiInputTextFlags_ReadOnly);

	ImGui::PopItemWidth();

	ImGui::PopStyleColor(popcount);

	ImGui::SameLine();

	if (!beingModified && ImGui::Button(("Set" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)))
	{
		beingModified = true;
		setting.keysDisplayString()[0] = '\0';
		GW2Radial::Input::i().BeginRecordInputs([&setting](GW2Radial::KeyCombo kc) {
			setting.keyCombo(kc);
		});
	}
	else if (beingModified && ImGui::Button(("Clear" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)))
	{
		beingModified = false;
		setting.keyCombo({});
		GW2Radial::Input::i().CancelRecordInputs();
	}

	ImGui::SameLine();

	ImGui::PushItemWidth(windowWidth * 0.5f);

	ImGui::Text(setting.displayName().c_str());

	ImGui::PopItemWidth();

	if(tooltip)
	    ImGuiHelpTooltip(tooltip);

	return beingModified;
}

void ImGuiTitle(const char * text)
{
	ImGui::Dummy({0, ImGui::GetStyle().ItemSpacing.y * 2 });
	ImGui::PushFont(GW2Radial::Core::i().fontBlack());
	ImGui::TextUnformatted(text);
	ImGui::Separator();
	ImGui::PopFont();
	ImGui::Spacing();
}

float ImGuiHelpTooltipSize() {
	ImGui::PushFont(GW2Radial::Core::i().fontIcon());
    auto r = ImGui::CalcTextSize(reinterpret_cast<const char*>(ICON_FA_QUESTION_CIRCLE)).x + ImGui::GetStyle().ItemSpacing.x + 1.f;
	ImGui::PopFont();

	return r;
}

void ImGuiHelpTooltip(const char* desc)
{
	ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGuiHelpTooltipSize() - ImGui::GetScrollX() - ImGui::GetStyle().ScrollbarSize);
	ImGui::PushFont(GW2Radial::Core::i().fontIcon());
    ImGui::TextDisabled(reinterpret_cast<const char*>(ICON_FA_QUESTION_CIRCLE));
	ImGui::PopFont();
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ImGuiDisable(float alpha) {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * alpha);
	auto disabledColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, disabledColor);
	ImGui::PushStyleColor(ImGuiCol_CheckMark, disabledColor);
	ImGui::PushStyleColor(ImGuiCol_Text, disabledColor);
	ImGui::PushStyleColor(ImGuiCol_Button, disabledColor);
}
void ImGuiDisableEnd() {
	ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
    ImGui::PopItemFlag();
}

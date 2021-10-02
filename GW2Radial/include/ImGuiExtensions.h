#pragma once
#include <Main.h>
#include <imgui.h>
#include <Keybind.h>
#include <ConfigurationOption.h>

ImVec2 operator*(const ImVec2& a, const ImVec2& b);
ImVec2 operator*(const ImVec2& a, float b);
ImVec2 operator-(const ImVec2& a, const ImVec2& b);
ImVec2 operator*=(ImVec2& a, const ImVec2& b);

ImVec4 operator/(const ImVec4& v, float f);
void ImGuiKeybindInput(GW2Radial::Keybind& keybind, GW2Radial::Keybind** keybindBeingModified, const char* tooltip);

template<typename F, typename T, typename... Args>
bool ImGuiConfigurationWrapper(F fct, const char* name, GW2Radial::ConfigurationOption<T>& value, Args&&... args)
{
	if(fct(name, &value.value(), std::forward<Args>(args)...)) {
		value.ForceSave();
		return true;
	}

	return false;
}

template<typename F, typename T, typename... Args>
bool ImGuiConfigurationWrapper(F fct, GW2Radial::ConfigurationOption<T>& value, Args&&... args)
{
	if(fct(value.displayName().c_str(), &value.value(), std::forward<Args>(args)...)) {
		value.ForceSave();
		return true;
	}

	return false;
}

inline bool ImGuiInputIntFormat(const char* label, int* v, const char* format, int step = 0, int step_fast = 0, ImGuiInputTextFlags flags = 0)
{
	return ImGui::InputScalar(label, ImGuiDataType_S32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

void ImGuiTitle(const char * text);
float ImGuiHelpTooltipSize();
void ImGuiHelpTooltip(const char* desc);

class ImGuiDisabler
{
	static bool disabled_;
public:
	ImGuiDisabler(bool disable, float alpha = 0.6f);
	~ImGuiDisabler();
};
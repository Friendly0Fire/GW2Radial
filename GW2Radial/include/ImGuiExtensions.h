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
void ImGuiKeybindInput(GW2Radial::Keybind& setting, const char* tooltip = nullptr);

template<typename F, typename T, typename... Args>
void ImGuiConfigurationWrapper(F fct, const char* name, GW2Radial::ConfigurationOption<T>& value, Args&&... args)
{
	if(fct(name, &value.value(), std::forward<Args>(args)...))
		value.ForceSave();
}

template<typename F, typename T, typename... Args>
void ImGuiConfigurationWrapper(F fct, GW2Radial::ConfigurationOption<T>& value, Args&&... args)
{
	if(fct(value.displayName().c_str(), &value.value(), std::forward<Args>(args)...))
		value.ForceSave();
}

void ImGuiTitle(const char * text);
float ImGuiHelpTooltipSize();
void ImGuiHelpTooltip(const char* desc);
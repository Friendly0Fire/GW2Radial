#pragma once
#include <Main.h>
#include <imgui.h>
#include <Keybind.h>
#include <ConfigurationOption.h>

ImVec4 operator/(const ImVec4& v, float f);
void ImGuiKeybindInput(GW2Addons::Keybind& setting);

template<typename F, typename T, typename... Args>
void ImGuiConfigurationWrapper(F* fct, const char* name, GW2Addons::ConfigurationOption<T>& value, Args... args)
{
	if(fct(name, value.value(), args...))
		value.SaveValue();
}

template<typename F, typename T, typename... Args>
void ImGuiConfigurationWrapper(F* fct, GW2Addons::ConfigurationOption<T>& value, Args... args)
{
	if(fct(value.displayName(), value.value(), args...))
		value.SaveValue();
}
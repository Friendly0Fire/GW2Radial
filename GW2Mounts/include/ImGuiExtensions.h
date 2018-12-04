#pragma once
#include <Main.h>
#include <imgui/imgui.h>
#include <Keybind.h>

ImVec4 operator/(const ImVec4& v, float f);
void ImGuiKeybindInput(GW2Addons::Keybind& setting);
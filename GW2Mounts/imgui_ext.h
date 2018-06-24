#pragma once
#include "main.h"
#include <functional>
#include <set>
#include <imgui.h>

struct ImGuiKeybind
{
	char DisplayString[256];
	bool IsBeingModified = false;
	std::function<void(const std::set<uint>&)> SetCallback;

	void UpdateDisplayString(const std::set<uint>& keys);
	void UpdateKeybind(const std::set<uint>& keys, bool apply);
};

ImVec4 operator/(const ImVec4& v, float f);
void ImGuiKeybindInput(const std::string& name, ImGuiKeybind& setting);
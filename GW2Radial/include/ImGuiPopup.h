#pragma once
#include <Main.h>
#include <imgui.h>
#include <functional>

namespace GW2Radial
{

class ImGuiPopup
{
public:
	static constexpr int DefaultFlags() { return ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove; }

	ImGuiPopup(const std::string& name, ImGuiWindowFlags flags = DefaultFlags());
	ImGuiPopup& Position(ImVec2 centerPos, bool relative = true);
	ImGuiPopup& Size(ImVec2 size, bool relative = true);
	ImGuiPopup& Display(std::function<void(const ImVec2& windowSize)> content, std::function<void()> closeCallback);
protected:
	bool opened_ = false;
	ImVec2 pos_, size_;
};

}

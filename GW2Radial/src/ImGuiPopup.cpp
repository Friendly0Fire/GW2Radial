#include <ImGuiPopup.h>
#include <ImGuiExtensions.h>
#include <Core.h>

namespace GW2Radial
{
	ImGuiPopup::ImGuiPopup(const std::string & name, ImGuiWindowFlags flags)
	{
		opened_ = ImGui::Begin(name.c_str(), nullptr, flags);
		if(!opened_)
			ImGui::End();
	}

	ImGuiPopup& ImGuiPopup::Position(ImVec2 centerPos, bool relative)
	{
		if(opened_)
		{
			if(relative)
				centerPos *= ImVec2(float(Core::i()->screenWidth()), float(Core::i()->screenHeight()));

			pos_ = centerPos;
		}

		return *this;
	}

	ImGuiPopup& ImGuiPopup::Size(ImVec2 size, bool relative)
	{
		if(opened_)
		{
			if(relative)
				size *= ImVec2(float(Core::i()->screenWidth()), float(Core::i()->screenHeight()));

			size_ = size;
		}

		return *this;
	}

	ImGuiPopup& ImGuiPopup::Display(std::function<void(const ImVec2&windowSize)> content, std::function<void()> closeCallback)
	{
		if(!opened_)
			return *this;
		
		ImGui::SetWindowPos(pos_ - size_ * 0.5f);
		ImGui::SetWindowSize(size_);

		content(ImGui::GetWindowSize());
		
		ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.4f);
		ImGui::SetCursorPosY(ImGui::GetWindowSize().y - ImGui::GetFontSize() * 2.5f);
		if (ImGui::Button("OK", ImVec2(ImGui::GetWindowSize().x * 0.2f, ImGui::GetFontSize() * 1.5f)))
			closeCallback();

		ImGui::End();

		return *this;
	}

}
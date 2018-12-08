#pragma once
#include <Main.h>
#include <ImGuiExtensions.h>
#include <SettingsMenu.h>

#include <d3dx9.h>

namespace GW2Addons
{

class WheelElement : public SettingsMenu::Implementer
{
public:
	WheelElement(uint id, std::string nickname, std::string displayName, IDirect3DDevice9* dev);
	virtual ~WheelElement();

	void Draw(int n, D3DXVECTOR4 spriteDimensions, size_t activeElementsCount, const mstime& currentTime, const WheelElement* elementHovered, const class Wheel* parent);

	uint elementId() const { return elementId_; }
	
	const std::string& nickname() const { return nickname_; }
	const std::string& displayName() const { return displayName_; }

	mstime currentHoverTime() const { return currentHoverTime_; }
	void currentHoverTime(mstime cht) { currentHoverTime_ = cht; }

	const Keybind& keybind() const { return keybind_; }
	bool isActive() const { return keybind_.isSet(); }

protected:
	virtual const std::array<float, 4> color() = 0;

	std::string nickname_, displayName_;
	uint elementId_;
	Keybind keybind_;
	IDirect3DTexture9* appearance_ = nullptr;
	mstime currentHoverTime_ = 0;

	// Inherited via Implementer
	virtual void DrawMenu() override;
};

}

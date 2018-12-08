#pragma once
#include <Main.h>
#include <ImGuiExtensions.h>
#include <simpleini/SimpleIni.h>

#include <array>
#include <d3dx9core.h>

namespace GW2Addons
{

class WheelElement
{
public:
	WheelElement(uint id, std::string nickname, std::string displayName, IDirect3DDevice9* dev);
	virtual ~WheelElement();

	virtual const char* name() const = 0;

	uint elementId() const { return elementId_; }

	mstime currentHoverTime() const { return currentHoverTime_; }
	void currentHoverTime(mstime cht) { currentHoverTime_ = cht; }

protected:
	uint elementId_;
	Keybind keybind_;
	IDirect3DTexture9* appearance_ = nullptr;
	mstime currentHoverTime_ = 0;
};

}

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
	WheelElement(uint id, uint resourceBaseId, std::string nickname, std::string displayName, IDirect3DDevice9* dev);
	virtual ~WheelElement();

	virtual const char* name() const = 0;

protected:

	uint elementId_;
	std::set<uint> keys_;
	Keybind keybind_;
	IDirect3DTexture9* appearance_ { };
};

}

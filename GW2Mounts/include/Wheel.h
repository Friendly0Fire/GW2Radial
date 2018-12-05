#pragma once

#include <Main.h>
#include <WheelElement.h>

namespace GW2Addons
{

class Wheel
{
public:
	Wheel(uint resourceId, std::string nickname, std::string displayName, IDirect3DDevice9* dev);
	virtual ~Wheel();

protected:
	const float CircleRadiusBase = 256.f / 1664.f * 0.25f;

	std::vector<std::unique_ptr<WheelElement>> wheelElements_;
	bool isVisible_ = false;
	Keybind keybind_, centralKeybind_;

	D3DXVECTOR2 currentPosition_;
	mstime currentTriggerTime_ = 0;

	WheelElement* currentHovered_ = nullptr;
	WheelElement* previousHovered_ = nullptr;
	
	IDirect3DTexture9* appearance_ = nullptr;
};

}
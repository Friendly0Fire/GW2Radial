#pragma once

#include <Main.h>
#include <WheelElement.h>
#include <ConfigurationOption.h>

namespace GW2Addons
{

class Wheel
{
public:
	enum class CenterBehavior : int
	{
		PREVIOUS,
		FAVORITE,
		NOTHING
	};

	Wheel(uint resourceId, std::string nickname, std::string displayName, IDirect3DDevice9* dev);
	virtual ~Wheel();

	void UpdateHover();
	void AddElement(std::unique_ptr<WheelElement>&& we) { wheelElements_.push_back(std::move(we)); }
	void Draw(IDirect3DDevice9* dev, ID3DXEffect* fx, class UnitQuad* quad);
	void OnFocusLost();

protected:
	WheelElement* ModifyCenterHoveredElement(WheelElement* elem);

	const float CircleRadiusBase = 256.f / 1664.f * 0.25f;

	std::vector<std::unique_ptr<WheelElement>> wheelElements_;
	bool isVisible_ = false;
	Keybind keybind_, centralKeybind_;

	ConfigurationOption<CenterBehavior> centerBehaviorOption_;
	ConfigurationOption<int> centerFavoriteOption_;
	
	ConfigurationOption<float> scaleOption_;
	ConfigurationOption<float> centerScaleOption_;
	
	ConfigurationOption<int> displayDelayOption_;

	D3DXVECTOR2 currentPosition_;
	mstime currentTriggerTime_ = 0;

	WheelElement* currentHovered_ = nullptr;
	WheelElement* previousHovered_ = nullptr;
	
	IDirect3DTexture9* appearance_ = nullptr;
};

}

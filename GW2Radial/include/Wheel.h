#pragma once

#include <Main.h>
#include <WheelElement.h>
#include <ConfigurationOption.h>
#include <SettingsMenu.h>
#include <d3dx9.h>
#include <Input.h>

namespace GW2Radial
{

class Wheel : public SettingsMenu::Implementer
{
public:
	enum class CenterBehavior : int
	{
		NOTHING = 0,
		PREVIOUS = 1,
		FAVORITE = 2
	};

	Wheel(uint bgResourceId, uint inkResourceId, std::string nickname, std::string displayName, IDirect3DDevice9* dev);
	virtual ~Wheel();

	void UpdateHover();
	void AddElement(std::unique_ptr<WheelElement>&& we) { wheelElements_.push_back(std::move(we)); Sort(); }
	void Draw(IDirect3DDevice9* dev, ID3DXEffect* fx, class UnitQuad* quad);
	void OnFocusLost();

	bool drawOverUI() const { return showOverGameUIOption_.value(); }

protected:
	void Sort();
	WheelElement* GetCenterHoveredElement();
	std::vector<WheelElement*> GetActiveElements();
	bool OnMouseMove();
	InputResponse OnInputChange(bool changed, const std::set<uint>& keys, const std::list<EventKey>& changedKeys);
	void ActivateWheel(bool isMountOverlayLocked);
	void DeactivateWheel();

	std::string nickname_, displayName_;

	std::vector<std::unique_ptr<WheelElement>> wheelElements_;
	bool isVisible_ = false;
	uint minElementSortingPriority_ = 0;
	Keybind keybind_, centralKeybind_;

	ConfigurationOption<int> centerBehaviorOption_;
	ConfigurationOption<int> centerFavoriteOption_;
	
	ConfigurationOption<float> scaleOption_;
	ConfigurationOption<float> centerScaleOption_;
	
	ConfigurationOption<int> displayDelayOption_;
	ConfigurationOption<int> animationTimeOption_;
	
	ConfigurationOption<bool> resetCursorOnLockedKeybindOption_;
	ConfigurationOption<bool> lockCameraWhenOverlayedOption_;
	ConfigurationOption<bool> showOverGameUIOption_;
	ConfigurationOption<bool> noHoldOption_;

	D3DXVECTOR2 currentPosition_;
	mstime currentTriggerTime_ = 0;

	WheelElement* currentHovered_ = nullptr;
	WheelElement* previousUsed_ = nullptr;
	
	IDirect3DTexture9* backgroundTexture_ = nullptr;
	IDirect3DTexture9* inkTexture_ = nullptr;
	
	Input::MouseMoveCallback mouseMoveCallback_;
	Input::InputChangeCallback inputChangeCallback_;

	D3DXVECTOR3 inkSpot_;

	void DrawMenu() override;

	friend class WheelElement;
};

}

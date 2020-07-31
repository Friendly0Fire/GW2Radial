#pragma once

#include <Main.h>
#include <WheelElement.h>
#include <ConfigurationOption.h>
#include <SettingsMenu.h>

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
	enum class BehaviorBeforeDelay : int
	{
		NOTHING = 0,
		PREVIOUS = 1,
		FAVORITE = 2,
		DIRECTION = 3
	};

	Wheel(uint bgResourceId, uint wipeMaskResourceId, std::string nickname, std::string displayName, IDirect3DDevice9* dev);
	virtual ~Wheel();

	template<typename T>
	static std::unique_ptr<Wheel> Create(uint bgResourceId, uint inkResourceId, std::string nickname, std::string displayName, IDirect3DDevice9* dev)
	{
		// TODO: Would be nice to somehow let wheel element .cpps determine these parameters as well
		auto wheel = std::make_unique<Wheel>(bgResourceId, inkResourceId, std::move(nickname), std::move(displayName), dev);
		wheel->Setup<T>(dev);
		return std::move(wheel);
	}

	template<typename T>
	void Setup(IDirect3DDevice9* dev); // Requires implementation for each wheel element type

	void UpdateHover();
	void AddElement(std::unique_ptr<WheelElement>&& we) { wheelElements_.push_back(std::move(we)); Sort(); }
	void Draw(IDirect3DDevice9* dev, Effect* fx, class UnitQuad* quad);
	void OnFocusLost();
	void OnUpdate();
	void OnMapChange(uint prevId, uint newId);
	void OnCharacterChange(const wchar_t* prevCharacterName, const wchar_t* newCharacterName);

	bool drawOverUI() const { return showOverGameUIOption_.value(); }

	void SetResetCursorPositionBeforeKeyPress(bool enabled) { resetCursorPositionBeforeKeyPress_ = enabled; }
	
	const std::string& nickname() const { return nickname_; }
	const std::string& displayName() const { return displayName_; }

protected:
	void Sort();
	WheelElement* GetCenterHoveredElement();
	WheelElement* GetFavorite(int favoriteId);
	std::vector<WheelElement*> GetActiveElements(bool sorted = true);
	bool OnMouseMove();
	InputResponse OnInputChange(bool changed, const std::set<ScanCode>& scs, const std::list<EventKey>& changedKeys);
	void ActivateWheel(bool isMountOverlayLocked);
	void DeactivateWheel();
	void SendKeybindOrDelay(WheelElement* we, std::optional<Point> mousePos);
	std::function<bool(WheelElement*&)> doBypassWheel_ = [](auto) { return false; };

	std::string nickname_, displayName_;
	bool resetCursorPositionBeforeKeyPress_ = false;
	bool resetCursorPositionToCenter_ = false;

	std::vector<std::unique_ptr<WheelElement>> wheelElements_;
	std::vector<WheelElement*> sortedWheelElements_;
	bool isVisible_ = false;
	uint minElementSortingPriority_ = 0;
	Keybind keybind_, centralKeybind_;
	bool waitingForBypassComplete_ = false;

	bool worksOnlyAboveWater_ = false;
	bool worksOnlyOutOfCombat_ = false;
	WheelElement* conditionallyDelayed_ = nullptr;
	mstime conditionallyDelayedTime_ = 0;
	uint conditionallyDelayedTestCount_ = 0;

	ConfigurationOption<int> centerBehaviorOption_;
	ConfigurationOption<int> centerFavoriteOption_;
	ConfigurationOption<int> delayFavoriteOption_;
	
	ConfigurationOption<float> scaleOption_;
	ConfigurationOption<float> centerScaleOption_;
	
	ConfigurationOption<int> displayDelayOption_;
	ConfigurationOption<int> animationTimeOption_;
	
	ConfigurationOption<bool> resetCursorOnLockedKeybindOption_;
	ConfigurationOption<bool> lockCameraWhenOverlayedOption_;
	ConfigurationOption<bool> showOverGameUIOption_;
	ConfigurationOption<bool> noHoldOption_;
	ConfigurationOption<bool> clickSelectOption_;
	ConfigurationOption<int> behaviorOnReleaseBeforeDelay_;
	ConfigurationOption<bool> resetCursorAfterKeybindOption_;

	ConfigurationOption<bool> disableKeybindsInCombatOption_;
	ConfigurationOption<int> maximumConditionalWaitTimeOption_;
	ConfigurationOption<bool> showDelayTimerOption_;

	std::optional<Point> cursorResetPosition_;
	fVector2 currentPosition_;
	mstime currentTriggerTime_ = 0;

	WheelElement* currentHovered_ = nullptr;
	WheelElement* previousUsed_ = nullptr;
	
	IDirect3DTexture9* backgroundTexture_ = nullptr;
	IDirect3DTexture9* wipeMaskTexture_ = nullptr;
	
	Input::MouseMoveCallback mouseMoveCallback_;
	Input::InputChangeCallback inputChangeCallback_;

	fVector3 wipeMaskData_;

	const char* GetTabName() const override { return displayName_.c_str(); }
	void DrawMenu() override;

	friend class WheelElement;
};

}

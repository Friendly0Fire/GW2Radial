#pragma once

#include <Main.h>
#include <WheelElement.h>
#include <ConfigurationOption.h>
#include <SettingsMenu.h>

#include <Input.h>

namespace GW2Radial
{

class Effect;

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
	void OnCharacterChange(const std::wstring& prevCharacterName, const std::wstring& newCharacterName);

    [[nodiscard]] bool drawOverUI() const { return showOverGameUIOption_.value(); }

	void SetResetCursorPositionBeforeKeyPress(bool enabled) { resetCursorPositionBeforeKeyPress_ = enabled; }
	
	[[nodiscard]] const std::string& nickname() const { return nickname_; }
	[[nodiscard]] const std::string& displayName() const { return displayName_; }
	
	auto& visibleInMenuOption() { return visibleInMenuOption_; }

	bool visible() override { return visibleInMenuOption_.value(); }

protected:
	void Sort();

	WheelElement* GetCenterHoveredElement();
	WheelElement* GetFavorite(int favoriteId);
	std::vector<WheelElement*> GetActiveElements(bool sorted = true);
	PreventPassToGame KeybindEvent(bool center, bool activated);
	void OnMouseMove(bool& rv);
	void OnMouseButton(ScanCode sc, bool down, bool& rv);
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
	ConditionSetPtr conditions_;
	ActivationKeybind keybind_, centralKeybind_;
	bool waitingForBypassComplete_ = false;

	struct DelayTest {
	    bool enabled = false;
		bool canToggleOff = false;
		std::function<bool()> test, toggleOffTest;

		[[nodiscard]] bool delayed() const {
			if(!enabled)
				return false;
		    return test() && (!canToggleOff || !toggleOffTest());
		}

		[[nodiscard]] bool passes() const {
			if(!enabled)
				return true;
		    return !test() ||
				canToggleOff && toggleOffTest();
		}
	};

	DelayTest aboveWater_, outOfCombat_;

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

	ConfigurationOption<int> maximumConditionalWaitTimeOption_;
	ConfigurationOption<bool> showDelayTimerOption_;
	ConfigurationOption<bool> centerCancelDelayedInputOption_;
	ConfigurationOption<bool> enableConditionsOption_;
	
	ConfigurationOption<bool> visibleInMenuOption_;

	std::optional<Point> cursorResetPosition_;
	fVector2 currentPosition_;
	mstime currentTriggerTime_ = 0;

	WheelElement* currentHovered_ = nullptr;
	WheelElement* previousUsed_ = nullptr;
	
	ComPtr<IDirect3DTexture9> backgroundTexture_;
	ComPtr<IDirect3DTexture9> wipeMaskTexture_;

	std::unique_ptr<Input::MouseMoveCallback> mouseMoveCallback_;
	std::unique_ptr<Input::MouseButtonCallback> mouseButtonCallback_;

	struct ExtraUI {
	    std::function<void()> display, interaction, queuing, misc;
	};

	struct ExtraData {};

	std::optional<ExtraUI> extraUI_;
	std::shared_ptr<ExtraData> extraData_;

	fVector3 wipeMaskData_;

    [[nodiscard]] const char* GetTabName() const override { return displayName_.c_str(); }
	void DrawMenu(Keybind** currentEditedKeybind) override;

	friend class WheelElement;
	friend class CustomWheelsManager;
};

}

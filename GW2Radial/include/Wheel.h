#pragma once

#include <Main.h>
#include <WheelElement.h>
#include <ConfigurationOption.h>
#include <SettingsMenu.h>
#include <Utility.h>
#include <ShaderManager.h>

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

	Wheel(uint bgResourceId, uint wipeMaskResourceId, std::string nickname, std::string displayName, ID3D11Device* dev);
	virtual ~Wheel();

	template<typename T>
	static std::unique_ptr<Wheel> Create(uint bgResourceId, uint inkResourceId, std::string nickname, std::string displayName, ID3D11Device* dev)
	{
		// TODO: Would be nice to somehow let wheel element .cpps determine these parameters as well
		auto wheel = std::make_unique<Wheel>(bgResourceId, inkResourceId, std::move(nickname), std::move(displayName), dev);
		wheel->Setup<T>(dev);
		return std::move(wheel);
	}

	template<typename T>
	void Setup(ID3D11Device* dev); // Requires implementation for each wheel element type

	void UpdateHover();
	void AddElement(std::unique_ptr<WheelElement>&& we) { wheelElements_.push_back(std::move(we)); Sort(); }
	void Draw(ComPtr<ID3D11DeviceContext> ctx);
	void OnFocusLost();
	void OnUpdate();
	void OnMapChange(uint prevId, uint newId);
	void OnCharacterChange(const std::wstring& prevCharacterName, const std::wstring& newCharacterName);

    [[nodiscard]] bool drawOverUI() const { return showOverGameUIOption_.value(); }

	void SetAlwaysResetCursorPositionBeforeKeyPress(bool enabled) { alwaysResetCursorPositionBeforeKeyPress_ = enabled; }

	[[nodiscard]] const std::string& nickname() const { return nickname_; }
	[[nodiscard]] const std::string& displayName() const { return displayName_; }

	auto& visibleInMenuOption() { return visibleInMenuOption_; }

	bool visible() override { return visibleInMenuOption_.value(); }

	ID3D11Buffer* GetConstantBuffer() const { return cb_s.buffer().Get(); }

protected:
	void Sort();
	void UpdateConstantBuffer(ID3D11DeviceContext* ctx, const fVector4& spriteDimensions, float fadeIn, float animationTimer,
		const std::vector<WheelElement*>& activeElements, const std::vector<float>& hoveredFadeIns, float timeLeft, bool showIcon);
	void UpdateConstantBuffer(ID3D11DeviceContext* ctx, const fVector4& baseSpriteDimensions);

	static const mstime conditionallyDelayedFadeOutTime = 500;

	WheelElement* GetCenterHoveredElement();
	WheelElement* GetFavorite(int favoriteId);
	std::vector<WheelElement*> GetActiveElements(bool sorted = true);
	PreventPassToGame KeybindEvent(bool center, bool activated);
	void OnMouseMove(bool& rv);
	void OnMouseButton(ScanCode sc, bool down, bool& rv);
	void ActivateWheel(bool isMountOverlayLocked);
	void DeactivateWheel();
	void SendKeybindOrDelay(WheelElement* we, std::optional<Point> mousePos);
	void ResetConditionallyDelayed(bool withFadeOut, mstime currentTime = TimeInMilliseconds());
	std::function<bool(WheelElement*&)> doBypassWheel_ = [](auto) { return false; };

	std::string nickname_, displayName_;
	bool alwaysResetCursorPositionBeforeKeyPress_ = false;
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

	DelayTest aboveWater_, outOfCombat_, custom_;

	WheelElement* conditionallyDelayed_ = nullptr;
	mstime conditionallyDelayedTime_ = 0;
	bool conditionallyDelayedTestPasses_ = false;
	bool conditionallyDelayedCustom_ = false;
	mstime conditionallyDelayedPassesTime_ = 0;

	ConfigurationOption<int> centerBehaviorOption_;
	ConfigurationOption<int> centerFavoriteOption_;
	ConfigurationOption<int> delayFavoriteOption_;

	ConfigurationOption<float> scaleOption_;
	ConfigurationOption<float> centerScaleOption_;
	ConfigurationOption<int> opacityMultiplierOption_;

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
	ConfigurationOption<int> conditionalDelayDelayOption_;
	ConfigurationOption<bool> showDelayTimerOption_;
	ConfigurationOption<bool> centerCancelDelayedInputOption_;
	ConfigurationOption<bool> enableConditionsOption_;
	ConfigurationOption<bool> enableQueuingOption_;

	ConfigurationOption<bool> visibleInMenuOption_;

	std::optional<Point> cursorResetPosition_;
	fVector2 currentPosition_;
	mstime currentTriggerTime_ = 0;

	WheelElement* currentHovered_ = nullptr;
	WheelElement* previousUsed_ = nullptr;

	Texture2D backgroundTexture_;
	Texture2D wipeMaskTexture_;
	ShaderId psWheel_, psWheelElement_, psCursor_, psDelayIndicator_, vs_;
	ComPtr<ID3D11BlendState> blendState_;
	ComPtr<ID3D11SamplerState> borderSampler_;
	ComPtr<ID3D11SamplerState> baseSampler_;

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

	struct WheelCB
	{
		fVector3 wipeMaskData;
		float wheelFadeIn;
		float animationTimer;
		float centerScale;
		int elementCount;
		float globalOpacity;
		float hoverFadeIns[12];
		float timeLeft;
		bool showIcon;
	};

	static ConstantBuffer<WheelCB> cb_s;
};

}

#pragma once

#include <ConfigurationOption.h>
#include <Graphics.h>
#include <Input.h>
#include <Main.h>
#include <SettingsMenu.h>
#include <ShaderManager.h>
#include <Utility.h>
#include <WheelElement.h>

namespace GW2Radial
{
class Wheel : public SettingsMenu::Implementer
{
public:
    enum class CenterBehavior : int
    {
        Nothing    = 0,
        Previous   = 1,
        Favorite   = 2,
        PassToGame = 3
    };

    enum class BehaviorBeforeDelay : int
    {
        Nothing    = 0,
        Previous   = 1,
        Favorite   = 2,
        Direction  = 3,
        PassToGame = 4
    };

    Wheel(std::shared_ptr<Texture2D> bgTexture, std::string nickname, std::string displayName);
    virtual ~Wheel();

    void UpdateHover();

    void AddElement(std::unique_ptr<WheelElement>&& we)
    {
        wheelElements_.push_back(std::move(we));
        Sort();
    }

    void               Draw(ID3D11DeviceContext* ctx);
    void               OnFocusLost();
    virtual void       OnUpdate();
    void               OnMapChange(u32 prevId, u32 newId);
    void               OnCharacterChange(const std::wstring& prevCharacterName, const std::wstring& newCharacterName);

    [[nodiscard]] bool drawOverUI() const
    {
        return showOverGameUIOption_.value();
    }

    void SetAlwaysResetCursorPositionBeforeKeyPress(bool enabled)
    {
        alwaysResetCursorPositionBeforeKeyPress_ = enabled;
    }

    [[nodiscard]] const std::string& nickname() const
    {
        return nickname_;
    }

    [[nodiscard]] const std::string& displayName() const
    {
        return displayName_;
    }

    [[nodiscard]] auto& visibleInMenuOption()
    {
        return visibleInMenuOption_;
    }

    [[nodiscard]] bool visible() override
    {
        return visibleInMenuOption_.value();
    }

    [[nodiscard]] const ComPtr<ID3D11Buffer>& GetConstantBuffer() const
    {
        return cb_->buffer();
    }

    // Get current state, ignoring flags we don't want when *skipping* the wheel:
    // e.g. no skip in WvW -> we're never in WvW, ergo all mounts are "available", ergo no skip
    [[nodiscard]] ConditionalState GetSkipState() const
    {
        return ((enableSkipWvWOption_.value() ? ConditionalState::InWvW : ConditionalState::None) |
                (enableSkipUWOption_.value() ? ConditionalState::Underwater : ConditionalState::None) |
                (enableSkipOWOption_.value() ? ConditionalState::OnWater : ConditionalState::None)) &
               MumbleLink::i().currentState();
    }

    // Determine whether displaying the wheel should be skipped (in favor of just immediately triggering an element)
    // by looking for an unambiguous single choice within available usable elements
    [[nodiscard]] bool ShouldSkip(WheelElement*& we)
    {
        if (!enableSkipOWOption_.value() && !enableSkipUWOption_.value() && !enableSkipWvWOption_.value())
            return false;

        if (auto elems = GetUsableElements(GetSkipState()); elems.size() == 1)
        {
            we = elems.front();
            return true;
        }

        return false;
    }

    [[nodiscard]] bool CanActivate(const WheelElement* we) const;

protected:
    using OptKeybindWheelElement = std::variant<std::monostate, Keybind*, WheelElement*>;

    static bool OptHasValue(const OptKeybindWheelElement& o)
    {
        return o.index() != 0;
    }

    virtual Keybind* GetKeybindFromOpt(OptKeybindWheelElement& o)
    {
        if (std::holds_alternative<Keybind*>(o))
            return std::get<Keybind*>(o);
        else if (std::holds_alternative<WheelElement*>(o))
            return &std::get<WheelElement*>(o)->keybind();
        else
            return nullptr;
    }

    virtual void MenuSectionKeybinds(Keybind**)
    {}

    virtual void MenuSectionDisplay()
    {}

    virtual void MenuSectionInteraction()
    {}

    virtual void MenuSectionQueuing()
    {}

    virtual void MenuSectionMisc()
    {}

    virtual bool BypassCheck(WheelElement*&, Keybind*&)
    {
        return false;
    }

    virtual bool CustomDelayCheck(OptKeybindWheelElement&)
    {
        return false;
    }

    virtual bool ResetMouseCheck(WheelElement*)
    {
        return false;
    }

    virtual bool SpecialBehaviorBeforeDelay()
    {
        return false;
    }

    union Favorite
    {
        int value;
        struct Bitmask
        {
            int baseline : 6;

            int inCombat : 6;
            int onWater : 6;
            int underwater : 6;
            int inWvW : 6;
        } bits;
        static_assert(sizeof(Bitmask) == sizeof(int));
    };
    static Favorite MakeDefaultFavorite();

    void            Sort();
    void UpdateConstantBuffer(ID3D11DeviceContext* ctx, const glm::vec4& spriteDimensions, float fadeIn, float animationTimer, const std::vector<WheelElement*>& activeElements,
                              const std::span<float>& hoveredFadeIns, float timeLeft, bool showIcon, bool tilt);
    void UpdateConstantBuffer(ID3D11DeviceContext* ctx, const glm::vec4& baseSpriteDimensions);

    WheelElement*                              GetCenterHoveredElement();
    WheelElement*                              GetFavorite(Favorite fav) const;
    std::vector<WheelElement*>                 GetVisibleElements(ConditionalState cs, bool sorted = true) const;
    bool                                       HasVisibleElements(ConditionalState cs) const;
    std::vector<WheelElement*>                 GetUsableElements(ConditionalState cs, bool sorted = true) const;
    bool                                       HasUsableElements(ConditionalState cs) const;
    bool                                       HasVisibleOrUsableElements(ConditionalState cs) const;
    PassToGame                                 KeybindEvent(bool center, Activated activated);
    void                                       OnMouseMove(bool& rv);
    void                                       OnMouseButton(ScanCode sc, bool down, bool& rv);
    void                                       ActivateWheel(bool isMountOverlayLocked);
    void                                       DeactivateWheel();
    void                                       SendKeybindOrDelay(OptKeybindWheelElement kbwe, std::optional<Point> mousePos);
    void                                       ResetConditionallyDelayed(bool withFadeOut, mstime currentTime = TimeInMilliseconds());
    void                                       PassToGame();

    std::string                                nickname_, displayName_;
    bool                                       alwaysResetCursorPositionBeforeKeyPress_ = false;
    bool                                       resetCursorPositionToCenter_             = false;

    std::vector<std::unique_ptr<WheelElement>> wheelElements_;
    std::vector<WheelElement*>                 sortedWheelElements_;
    bool                                       isVisible_                 = false;
    u32                                        minElementSortingPriority_ = 0;
    ConditionSetPtr                            conditions_;
    ActivationKeybind                          keybind_, centralKeybind_;
    bool                                       waitingForBypassComplete_    = false;
    bool                                       clearConditionalDelayOnSend_ = true;

    struct ConditionalDelay
    {
        static constexpr mstime FadeOutTime    = 500;

        OptKeybindWheelElement  element        = {};
        mstime                  time           = TimeInMilliseconds();
        bool                    hidden         = false;
        bool                    immediate      = false;

        bool                    testPasses     = false;
        mstime                  testPassesTime = 0;
    };

    ConditionalDelay              conditionalDelay_;
    WheelElement*                 conditionalDelayDisplay_ = nullptr;

    ConfigurationOption<int>      centerBehaviorOption_;
    ConfigurationOption<Favorite> centerFavoriteOption_;
    ConfigurationOption<Favorite> delayFavoriteOption_;

    ConfigurationOption<float>    scaleOption_;
    ConfigurationOption<float>    centerScaleOption_;
    ConfigurationOption<int>      opacityMultiplierOption_;

    ConfigurationOption<int>      displayDelayOption_;
    ConfigurationOption<int>      animationTimeOption_;

    ConfigurationOption<bool>     resetCursorOnLockedKeybindOption_;
    ConfigurationOption<bool>     lockCameraWhenOverlayedOption_;
    ConfigurationOption<bool>     showOverGameUIOption_;
    ConfigurationOption<bool>     noHoldOption_;
    ConfigurationOption<bool>     clickSelectOption_;
    ConfigurationOption<int>      behaviorOnReleaseBeforeDelay_;
    ConfigurationOption<bool>     resetCursorAfterKeybindOption_;

    ConfigurationOption<int>      maximumConditionalWaitTimeOption_;
    ConfigurationOption<int>      conditionalDelayDelayOption_;
    ConfigurationOption<bool>     showDelayTimerOption_;
    ConfigurationOption<bool>     centerCancelDelayedInputOption_;
    ConfigurationOption<bool>     enableConditionsOption_;
    ConfigurationOption<bool>     enableQueuingOption_;
    ConfigurationOption<bool>     enableSkipOWOption_;
    ConfigurationOption<bool>     enableSkipUWOption_;
    ConfigurationOption<bool>     enableSkipWvWOption_;

    ConfigurationOption<bool>     visibleInMenuOption_;

    ConfigurationOption<float>    animationScale_;

    Point                         cursorResetPosition_;
    glm::vec2                     currentPosition_;
    mstime                        currentTriggerTime_ = 0;

    WheelElement*                 currentHovered_     = nullptr;
    WheelElement*                 previousUsed_       = nullptr;

    std::shared_ptr<Texture2D>    backgroundTexture_;
    ShaderId                      psWheel_, psWheelElement_, psCursor_, psDelayIndicator_, vs_;
    ComPtr<ID3D11BlendState>      blendState_;
    ComPtr<ID3D11SamplerState>    borderSampler_;
    ComPtr<ID3D11SamplerState>    baseSampler_;

    EventCallbackHandle           mouseMoveCallbackID_;
    EventCallbackHandle           mouseButtonCallbackID_;

    glm::vec3                     wipeMaskData_;
    bool                          showEmptyPopup_ = false;

    [[nodiscard]] const char*     GetTabName() const override
    {
        return displayName_.c_str();
    }

    void DrawMenu(Keybind** currentEditedKeybind) override;

    friend class WheelElement;
    friend class CustomWheelsManager;

    static inline const u32 MaxHoverFadeIns = 20;

    struct WheelCB
    {
        glm::vec3 wipeMaskData;
        float     wheelFadeIn;
        float     animationTimer;
        float     centerScale;
        int       elementCount;
        float     globalOpacity;
        float     hoverFadeIns[MaxHoverFadeIns];
        float     timeLeft;
        bool      showIcon;
    };

    ConstantBufferSPtr<WheelCB>        cb_;
    static ConstantBufferWPtr<WheelCB> cb_s;
};
} // namespace GW2Radial

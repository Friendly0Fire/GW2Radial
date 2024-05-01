#pragma once
#include <Graphics.h>
#include <ImGuiExtensions.h>
#include <Main.h>
#include <SettingsMenu.h>
#include <ShaderManager.h>

namespace GW2Radial
{
enum class ConditionalProperties : u32
{
    None              = 0,

    VisibleDefault    = 1,
    UsableDefault     = 2,

    VisibleUnderwater = 4,
    UsableUnderwater  = 8,

    VisibleOnWater    = 16,
    UsableOnWater     = 32,

    VisibleInCombat   = 64,
    UsableInCombat    = 128,

    VisibleWvW        = 256,
    UsableWvW         = 512,

    VisibleAll        = VisibleDefault | VisibleUnderwater | VisibleOnWater | VisibleInCombat | VisibleWvW,
    UsableAll         = UsableDefault | UsableUnderwater | UsableOnWater | UsableInCombat | UsableWvW,

    IsFlag
};

inline bool IsUsable(ConditionalState cs, ConditionalProperties cp)
{
    if (IsNone(cs) && IsNone(cp & ConditionalProperties::UsableDefault))
        return false;
    if (NotNone(cs & ConditionalState::InWvW) && IsNone(cp & ConditionalProperties::UsableWvW))
        return false;

    if (NotNone(cs & ConditionalState::InCombat) && IsNone(cp & ConditionalProperties::UsableInCombat))
        return false;
    if (NotNone(cs & ConditionalState::Underwater) && IsNone(cp & ConditionalProperties::UsableUnderwater))
        return false;
    if (NotNone(cs & ConditionalState::OnWater) && IsNone(cp & ConditionalProperties::UsableOnWater))
        return false;

    return true;
}

inline bool IsVisible(ConditionalState cs, ConditionalProperties cp)
{
    if (IsNone(cs) && IsNone(cp & ConditionalProperties::VisibleDefault))
        return false;
    if (NotNone(cs & ConditionalState::InWvW) && IsNone(cp & ConditionalProperties::VisibleWvW))
        return false;

    if (NotNone(cs & ConditionalState::InCombat) && IsNone(cp & ConditionalProperties::VisibleInCombat))
        return false;
    if (NotNone(cs & ConditionalState::Underwater) && IsNone(cp & ConditionalProperties::VisibleUnderwater))
        return false;
    if (NotNone(cs & ConditionalState::OnWater) && IsNone(cp & ConditionalProperties::VisibleOnWater))
        return false;

    return true;
}

class WheelElement
{
public:
    WheelElement(u32 id, const std::string& nickname, const std::string& category, const std::string& displayName, const glm::vec4& color, ConditionalProperties defaultProps,
                 Texture2D tex = {});
    virtual ~WheelElement() = default;

    int  DrawPriority(int extremumIndicator);

    void SetShaderState(ID3D11DeviceContext* ctx) const;
    void SetShaderState(ID3D11DeviceContext* ctx, const vec4& spriteDimensions, const ComPtr<ID3D11Buffer>& wheelCb, bool shadow, float hoverRatio) const;
    void Draw(ID3D11DeviceContext* ctx, int n, vec4 spriteDimensions, size_t activeElementsCount, const mstime& currentTime, const WheelElement* elementHovered,
              const class Wheel* parent);

    u32  elementId() const
    {
        return elementId_;
    }

    int sortingPriority() const
    {
        return sortingPriorityOption_.value();
    }

    void sortingPriority(int value)
    {
        return sortingPriorityOption_.value(value);
    }

    const std::string& nickname() const
    {
        return nickname_;
    }

    const std::string& displayName() const
    {
        return displayName_;
    }

    mstime currentHoverTime() const
    {
        return currentHoverTime_;
    }

    void currentHoverTime(mstime cht)
    {
        currentHoverTime_ = cht;
    }

    mstime currentExitTime() const
    {
        return currentExitTime_;
    }

    void currentExitTime(mstime cht)
    {
        currentExitTime_ = cht;
    }

    float hoverFadeIn(const mstime& currentTime, const Wheel* parent) const;

    float shadowStrength() const
    {
        return shadowStrength_;
    }

    void shadowStrength(float ss)
    {
        shadowStrength_ = ss;
    }

    float colorizeAmount() const
    {
        return colorizeAmount_;
    }

    void colorizeAmount(float ca)
    {
        colorizeAmount_ = ca;
    }

    bool premultiplyAlpha() const
    {
        return premultiplyAlpha_;
    }

    void premultiplyAlpha(bool pa)
    {
        premultiplyAlpha_ = pa;
    }

    const glm::vec4& color() const
    {
        return color_;
    }

    void color(const glm::vec4& c)
    {
        color_ = c;
    }

    [[nodiscard]] const Keybind& keybind() const
    {
        return keybind_;
    }

    [[nodiscard]] Keybind& keybind()
    {
        return keybind_;
    }

    [[nodiscard]] bool isBound() const
    {
        return keybind_.isSet();
    }

    [[nodiscard]] bool isUsable(ConditionalState cs) const
    {
        return isBound() && IsUsable(cs, props_.value());
    }

    [[nodiscard]] bool isVisible(ConditionalState cs) const
    {
        return isBound() && IsVisible(cs, props_.value());
    }

    const Texture2D& appearance() const
    {
        return appearance_;
    }

protected:
    ConfigurationOption<int>                   sortingPriorityOption_;
    ConfigurationOption<ConditionalProperties> props_;

    std::string                                nickname_, displayName_;
    u32                                        elementId_;
    Keybind                                    keybind_;
    Texture2D                                  appearance_;
    mstime                                     currentHoverTime_ = 0;
    mstime                                     currentExitTime_  = 0;
    float                                      aspectRatio_      = 1.f;
    float                                      shadowStrength_   = 0.8f;
    float                                      colorizeAmount_   = 1.f;
    float                                      texWidth_         = 0.f;
    bool                                       premultiplyAlpha_ = false;
    glm::vec4                                  color_{};

    struct WheelElementCB
    {
        glm::vec4 adjustedColor;
        float     elementHoverFadeIn;
        bool      premultiplyAlpha;
    };

    ConstantBufferSPtr<WheelElementCB>        cb_;
    static ConstantBufferWPtr<WheelElementCB> cb_s;
};
} // namespace GW2Radial

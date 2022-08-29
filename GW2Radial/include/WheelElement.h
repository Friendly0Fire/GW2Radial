#pragma once
#include <Graphics.h>
#include <ImGuiExtensions.h>
#include <Main.h>
#include <SettingsMenu.h>
#include <ShaderManager.h>

namespace GW2Radial
{
enum class ConditionalProperties : uint
{
    NONE               = 0,

    VISIBLE_DEFAULT    = 1,
    USABLE_DEFAULT     = 2,

    VISIBLE_UNDERWATER = 4,
    USABLE_UNDERWATER  = 8,

    VISIBLE_ON_WATER   = 16,
    USABLE_ON_WATER    = 32,

    VISIBLE_IN_COMBAT  = 64,
    USABLE_IN_COMBAT   = 128,

    VISIBLE_WVW        = 256,
    USABLE_WVW         = 512,

    VISIBLE_ALL        = VISIBLE_DEFAULT | VISIBLE_UNDERWATER | VISIBLE_ON_WATER | VISIBLE_IN_COMBAT | VISIBLE_WVW,
    USABLE_ALL         = USABLE_DEFAULT | USABLE_UNDERWATER | USABLE_ON_WATER | USABLE_IN_COMBAT | USABLE_WVW
};

inline ConditionalProperties operator|(ConditionalProperties a, ConditionalProperties b)
{
    return static_cast<ConditionalProperties>(static_cast<uint>(a) | static_cast<uint>(b));
}

inline ConditionalProperties operator&(ConditionalProperties a, ConditionalProperties b)
{
    return static_cast<ConditionalProperties>(static_cast<uint>(a) & static_cast<uint>(b));
}

inline ConditionalProperties operator~(ConditionalProperties a)
{
    return static_cast<ConditionalProperties>(~static_cast<uint>(a));
}

inline bool IsUsable(ConditionalState cs, ConditionalProperties cp)
{
    if (isNone(cs) && isNone(cp & ConditionalProperties::USABLE_DEFAULT))
        return false;
    if (notNone(cs & ConditionalState::IN_WVW) && isNone(cp & ConditionalProperties::USABLE_WVW))
        return false;

    if (notNone(cs & ConditionalState::IN_COMBAT) && isNone(cp & ConditionalProperties::USABLE_IN_COMBAT))
        return false;
    if (notNone(cs & ConditionalState::UNDERWATER) && isNone(cp & ConditionalProperties::USABLE_UNDERWATER))
        return false;
    if (notNone(cs & ConditionalState::ON_WATER) && isNone(cp & ConditionalProperties::USABLE_ON_WATER))
        return false;

    return true;
}

inline bool IsVisible(ConditionalState cs, ConditionalProperties cp)
{
    if (isNone(cs) && isNone(cp & ConditionalProperties::VISIBLE_DEFAULT))
        return false;
    if (notNone(cs & ConditionalState::IN_WVW) && isNone(cp & ConditionalProperties::VISIBLE_WVW))
        return false;

    if (notNone(cs & ConditionalState::IN_COMBAT) && isNone(cp & ConditionalProperties::VISIBLE_IN_COMBAT))
        return false;
    if (notNone(cs & ConditionalState::UNDERWATER) && isNone(cp & ConditionalProperties::VISIBLE_UNDERWATER))
        return false;
    if (notNone(cs & ConditionalState::ON_WATER) && isNone(cp & ConditionalProperties::VISIBLE_ON_WATER))
        return false;

    return true;
}

class WheelElement
{
public:
    WheelElement(uint id, const std::string& nickname, const std::string& category, const std::string& displayName, const glm::vec4& color, ConditionalProperties defaultProps,
                 Texture2D tex = {});
    virtual ~WheelElement() = default;

    int  DrawPriority(int extremumIndicator);

    void SetShaderState(ID3D11DeviceContext* ctx) const;
    void SetShaderState(ID3D11DeviceContext* ctx, const fVector4& spriteDimensions, const ComPtr<ID3D11Buffer>& wheelCb, bool shadow, float hoverRatio) const;
    void Draw(ID3D11DeviceContext* ctx, int n, fVector4 spriteDimensions, size_t activeElementsCount, const mstime& currentTime, const WheelElement* elementHovered,
              const class Wheel* parent);

    uint elementId() const
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
    uint                                       elementId_;
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

    static ConstantBuffer<WheelElementCB> cb_s;
};
} // namespace GW2Radial

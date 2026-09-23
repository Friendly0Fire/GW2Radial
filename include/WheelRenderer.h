#pragma once

#include <Graphics.h>
#include <ShaderManager.h>

namespace GW2Radial
{

class Wheel;
class WheelElement;

class WheelRenderer
{
    static constexpr u32 MaxHoverFadeIns = 20;

    struct WheelCB
    {
        vec3  wipeMaskData;
        float wheelFadeIn;
        float animationTimer;
        float centerScale;
        int   elementCount;
        float globalOpacity;
        float hoverFadeIns[MaxHoverFadeIns];
        float timeLeft;
        bool  showIcon;
    };

    std::shared_ptr<Texture2D>  backgroundTexture_;
    ShaderId                    psWheel_, psWheelElement_, psCursor_, psDelayIndicator_, vs_;
    ComPtr<ID3D11BlendState>    blendState_;
    ComPtr<ID3D11SamplerState>  borderSampler_;
    ComPtr<ID3D11SamplerState>  baseSampler_;
    ConstantBufferSPtr<WheelCB> wheelCB_;

    struct ElementCB
    {
        glm::vec4 adjustedColor;
        float     elementHoverFadeIn;
        bool      premultiplyAlpha;
    };

    ConstantBufferSPtr<ElementCB> elementCB_;

public:
    WheelRenderer(std::shared_ptr<Texture2D> bgTexture, ID3D11Device* dev);

    static float GetElementHoverFadeIn(mstime wheelDisplayTime, mstime hoverTime, mstime currentTime = TimeInMilliseconds()) const
    {
        return std::min(1.f, (currentTime - std::max(hoverTime, wheelDisplayTime)) / 1000.f * 6);
    }

    static float GetElementHoverFadeOut(mstime wheelDisplayTime, mstime exitTime, mstime currentTime = TimeInMilliseconds()) const
    {
        return 1.f - std::min(1.f, (currentTime - std::max(exitTime, wheelDisplayTime)) / 1000.f * 6);
    }

    struct WheelParams
    {
        vec2                               position;
        float                              scale;
        mstime                             displayTime;
        float                              animationDuration;
        std::span<const WheelElement*>     elements;
        const WheelElement*                hoveredElement;
        const Wheel*                       parent;
        std::array<float, MaxHoverFadeIns> hoveredFadeIns;
        vec3                               wipeMaskData;
        float                              centerScale;
        float                              opacity;
        float                              animationScale;
    };
    void DrawWheel(const WheelParams& wp, ID3D11DeviceContext* ctx);
    void DrawCounter(ID3D11DeviceContext* ctx);

    struct WheelCBParams
    {
        float fadeIn;
        float animationTimer;
        float timeLeft;
        bool  showIcon;
        bool  tilt;
        vec4  spriteDimensions;
    };
    void UpdateWheelConstantBuffer(const WheelParams& wp, const WheelCBParams& wcbp, ID3D11DeviceContext* ctx) const;
    void UpdateWheelConstantBuffer(const vec4& spriteDimensions, ID3D11DeviceContext* ctx) const;

    struct ElementParams
    {
        float                     fadeIn;
        float                     anglePosition;
        float                     aspectRatio;
        float                     shadowStrength;
        vec4                      spriteDimensions;
        vec4                      color;
        float                     colorizeAmount;
        bool                      premultiplyAlpha;
        ID3D11ShaderResourceView* sprite;
    };

    void DrawElement(ElementParams ep, size_t elementCount, ID3D11DeviceContext* ctx) const;
    void UpdateElementConstantBuffer(const ElementParams& ep, ID3D11DeviceContext* ctx) const;
    void UpdateElementConstantBuffer(const ElementParams& ep, bool shadow, ID3D11DeviceContext* ctx) const;
};

} // namespace GW2Radial
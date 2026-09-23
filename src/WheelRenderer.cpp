#include "WheelRenderer.h"

#include <GFXSettings.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Core.h"

namespace GW2Radial
{

WheelRenderer::WheelRenderer(std::shared_ptr<Texture2D> bgTexture, ID3D11Device* dev)
    : backgroundTexture_{ std::move(bgTexture) }
{
    vs_               = ShaderManager::i().GetShader(L"ScreenQuad.hlsl", D3D11_SHVER_VERTEX_SHADER, "ScreenQuad");
    psWheel_          = ShaderManager::i().GetShader(L"Wheel.hlsl", D3D11_SHVER_PIXEL_SHADER, "Wheel");
    psWheelElement_   = ShaderManager::i().GetShader(L"WheelElement.hlsl", D3D11_SHVER_PIXEL_SHADER, "WheelElement");
    psCursor_         = ShaderManager::i().GetShader(L"Cursor.hlsl", D3D11_SHVER_PIXEL_SHADER, "Cursor");
    psDelayIndicator_ = ShaderManager::i().GetShader(L"DelayIndicator.hlsl", D3D11_SHVER_PIXEL_SHADER, "DelayIndicator");

    CD3D11_BLEND_DESC blendDesc(D3D11_DEFAULT);
    blendDesc.RenderTarget[0].BlendEnable    = true;
    blendDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    GW2_CHECKED_HRESULT(dev->CreateBlendState(&blendDesc, blendState_.GetAddressOf()));

    CD3D11_SAMPLER_DESC sampDesc(D3D11_DEFAULT);
    GW2_CHECKED_HRESULT(dev->CreateSamplerState(&sampDesc, baseSampler_.GetAddressOf()));

    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    std::fill_n(sampDesc.BorderColor, std::size(sampDesc.BorderColor), 0.f);
    GW2_CHECKED_HRESULT(dev->CreateSamplerState(&sampDesc, borderSampler_.GetAddressOf()));

    wheelCB_   = ShaderManager::i().MakeConstantBuffer<WheelCB>();
    elementCB_ = ShaderManager::i().MakeConstantBuffer<ElementCB>();
}

void WheelRenderer::UpdateWheelConstantBuffer(const WheelParams& wp, const WheelCBParams& wcbp, ID3D11DeviceContext* ctx) const
{
    auto& cb           = *wheelCB_;
    cb->wipeMaskData   = wp.wipeMaskData;
    cb->wheelFadeIn    = wcbp.fadeIn;
    cb->animationTimer = wcbp.animationTimer;
    cb->centerScale    = wp.centerScale;
    cb->elementCount   = int(wp.elements.size());
    cb->globalOpacity  = wp.opacity * 0.01f;
    cb->timeLeft       = wcbp.timeLeft;
    cb->showIcon       = wcbp.showIcon;
    std::ranges::copy(wp.hoveredFadeIns, cb->hoverFadeIns);

    cb.Update(ctx);
    ctx->PSSetConstantBuffers(0, 1, cb.buffer().GetAddressOf());

    glm::mat4x4 tiltMatrix;
    if (wcbp.tilt)
    {
        auto mousePos = ImGui::GetIO().MousePos;
        vec2 mouseDist{ wp.position.x - mousePos.x / float(Core::i().screenWidth()), wp.position.y - mousePos.y / float(Core::i().screenHeight()) };
        mouseDist = -mouseDist / wcbp.spriteDimensions.zw;
        if (glm::length(mouseDist) > 0.2f)
            mouseDist *= 0.2f / glm::length(mouseDist);
        mouseDist *= 0.4f * wp.animationScale;

        tiltMatrix = glm::eulerAngleXY(-mouseDist.y, mouseDist.x);
    }
    else
        tiltMatrix = glm::identity<glm::mat4x4>();

    auto& vscb             = *Core::i().vertexCB();
    vscb->spriteDimensions = wcbp.spriteDimensions;
    vscb->tiltMatrix       = tiltMatrix;
    vscb->spriteZ          = 0.f;
    vscb.Update(ctx);
    ctx->VSSetConstantBuffers(0, 1, vscb.buffer().GetAddressOf());
}

void WheelRenderer::UpdateWheelConstantBuffer(const vec4& spriteDimensions, ID3D11DeviceContext* ctx) const
{
    auto& vscb             = *Core::i().vertexCB();
    vscb->spriteDimensions = spriteDimensions;
    vscb->spriteZ          = 0.f;
    vscb.Update(ctx);
    ctx->VSSetConstantBuffers(0, 1, vscb.buffer().GetAddressOf());
}

void WheelRenderer::DrawWheel(const WheelParams& wp, ID3D11DeviceContext* ctx)
{
    const u32  screenWidth  = Core::i().screenWidth();
    const u32  screenHeight = Core::i().screenHeight();

    const vec4 screenSize{ static_cast<float>(screenWidth), static_cast<float>(screenHeight), 1.f / static_cast<float>(screenWidth), 1.f / static_cast<float>(screenHeight) };

    const auto currentTime         = TimeInMilliseconds();

    ID3D11SamplerState* samplers[] = { baseSampler_.Get(), baseSampler_.Get() };
    ctx->PSSetSamplers(0, 2, samplers);

    D3D11_VIEWPORT vp;
    vp.TopLeftX = vp.TopLeftY = 0;
    vp.Width                  = static_cast<float>(screenWidth);
    vp.Height                 = static_cast<float>(screenHeight);
    vp.MinDepth               = 0.0f;
    vp.MaxDepth               = 1.0f;
    ctx->RSSetViewports(1, &vp);

    vec4 baseSpriteDimensions;
    baseSpriteDimensions.xy = wp.position;
    baseSpriteDimensions.z  = wp.scale * 0.5f * screenSize.y * screenSize.z;
    baseSpriteDimensions.w  = wp.scale * 0.5f;

    const float fadeTimer   = std::min(1.f, static_cast<float>(currentTime - wp.displayTime) / (wp.animationDuration * 0.5f));

    // std::ranges::copy(w.GetHoveredFadeValues(currentTime), hoveredFadeIns.begin());

    ShaderManager::i().SetShaders(ctx, vs_, psWheel_);
    ctx->OMSetBlendState(blendState_.Get(), nullptr, 0xffffffff);
    UpdateWheelConstantBuffer(wp,
                              { .fadeIn           = fadeTimer,
                                .animationTimer   = fmod(static_cast<float>(currentTime) / 1010.f, 55000.f),
                                .timeLeft         = 0.f,
                                .showIcon         = false,
                                .tilt             = true,
                                .spriteDimensions = baseSpriteDimensions },
                              ctx);

    ctx->PSSetShaderResources(0, 1, backgroundTexture_->srv.GetAddressOf());

    DrawScreenQuad(ctx);

    ShaderManager::i().SetShaders(ctx, vs_, psWheelElement_);
    ctx->OMSetBlendState(blendState_.Get(), nullptr, 0xffffffff);

    for (auto [i, we] : wp.elements | std::views::enumerate)
    {
        DrawElement({ .fadeIn           = GetElementHoverFadeIn(wp.displayTime, we->currentHoverTime(), currentTime),
                      .anglePosition    = wp.elements.size() == 1 ? 0.f : static_cast<float>(i) / static_cast<float>(wp.elements.size()) * float(M_PI) * 2.f,
                      .aspectRatio      = we->aspectRatio(),
                      .shadowStrength   = we->shadowStrength(),
                      .spriteDimensions = baseSpriteDimensions,
                      .color            = we->color(),
                      .colorizeAmount   = we->colorizeAmount(),
                      .premultiplyAlpha = we->premultiplyAlpha(),
                      .sprite           = we->appearance().srv.Get() },
                    wp.elements.size(), ctx);
    }

    // Cursor drawing
    {
        const auto& io = ImGui::GetIO();

        ShaderManager::i().SetShaders(ctx, vs_, psCursor_);
        ctx->OMSetBlendState(blendState_.Get(), nullptr, 0xffffffff);

        const vec4 spriteDimensions = { io.MousePos.x * screenSize.z, io.MousePos.y * screenSize.w, 0.08f * screenSize.y * screenSize.z, 0.08f };
        UpdateWheelConstantBuffer(spriteDimensions, ctx);
        DrawScreenQuad(ctx);
    }
}

void WheelRenderer::DrawCounter(Wheel& w, ID3D11DeviceContext* ctx)
{
    auto delayFadeOutTime = conditionalDelay_.time + maximumConditionalWaitTimeOption_.value() * 1000ull;
    if (showDelayTimerOption_.value() && !conditionalDelay_.hidden && currentTime < delayFadeOutTime + ConditionalDelay::FadeOutTime)
    {
        auto* delayElement = [&]
        {
            if (std::holds_alternative<WheelElement*>(conditionalDelay_.element))
                return std::get<WheelElement*>(conditionalDelay_.element);
            else
                return conditionalDelayDisplay_;
        }();
        if (delayElement)
        {
            bool  inFadeOut = currentTime >= delayFadeOutTime;

            float dt        = float(currentTime - conditionalDelay_.time) / 1000.f;
            float absDt     = dt;
            float timeLeft  = 0.f;
            if (inFadeOut)
                absDt = 1.f - (dt - maximumConditionalWaitTimeOption_.value()) / (ConditionalDelay::FadeOutTime / 1000.f);
            else
                timeLeft = 1.f - (currentTime - conditionalDelay_.time) / (float(maximumConditionalWaitTimeOption_.value()) * 1000.f);
            const auto& io = ImGui::GetIO();

            ShaderManager::i().SetShaders(ctx, vs_, psDelayIndicator_);
            ctx->OMSetBlendState(blendState_.Get(), nullptr, 0xffffffff);

            float dpiScale = 1.f;
            if (GFXSettings::i().dpiScaling())
                dpiScale = float(Core::i().GetDpiForWindow(Core::i().gameWindow())) / 96.f;

            auto      uiScale = float(MumbleLink::i().uiScale());

            glm::vec2 topLeftCorner;
            topLeftCorner.y = 77.f + 10.f * uiScale;
            if (uiScale == 0)
                topLeftCorner.y += 2.f;

            float bottom         = 13.f + 1.f * uiScale;
            topLeftCorner.x      = 450.f / 107.f * topLeftCorner.y;

            float widthMinScale  = std::min(1.f, screenSize.x / 1024.f);
            float heightMinScale = std::min(1.f, screenSize.y / 768.f);

            float minScale       = std::min(widthMinScale, heightMinScale);

            topLeftCorner.x *= dpiScale * minScale;
            topLeftCorner.y *= dpiScale * minScale;
            bottom *= dpiScale * minScale;

            topLeftCorner.x += screenSize.x * 0.5f;
            topLeftCorner.y  = screenSize.y - topLeftCorner.y;
            bottom           = screenSize.y - bottom;

            float spriteSize = bottom - topLeftCorner.y;

            if (dt < 0.3333f && !inFadeOut)
            {
                float sizeInterpolant = SmoothStep(dt * 3);
                spriteSize            = std::lerp(spriteSize * 3, spriteSize, sizeInterpolant);
                topLeftCorner.x       = std::lerp(0.5f * screenSize.x, topLeftCorner.x, 0.25f + sizeInterpolant * 0.75f);
                topLeftCorner.y       = std::lerp(0.5f * screenSize.y, topLeftCorner.y, 0.25f + sizeInterpolant * 0.75f);
            }

            vec4 spriteDimensions{ topLeftCorner.x * screenSize.z, topLeftCorner.y * screenSize.w, spriteSize * screenSize.z, spriteSize * screenSize.w };
            spriteDimensions.x += spriteDimensions.z * 0.5f;
            spriteDimensions.y += spriteDimensions.w * 0.5f;

            std::array<float, MaxHoverFadeIns> hoveredFadeIns;
            std::ranges::fill(hoveredFadeIns, 0.f);
            UpdateWheelConstantBuffer(ctx, spriteDimensions, std::min(absDt * 2, 1.f), fmod(currentTime / 1010.f, 55000.f), {}, hoveredFadeIns, timeLeft, delayElement, false);
            delayElement->SetShaderState(ctx);

            ID3D11ShaderResourceView* srvs[] = { backgroundTexture_->srv.Get(), delayElement->appearance().srv.Get() };
            ctx->PSSetShaderResources(0, 2, srvs);
            ctx->PSSetSamplers(1, 1, borderSampler_.GetAddressOf());

            DrawScreenQuad(ctx);
        }
    }
}

void WheelRenderer::UpdateElementConstantBuffer(const ElementParams& ep, ID3D11DeviceContext* ctx) const
{
    vec4 adjustedColor              = ep.color;
    adjustedColor.x                 = Lerp(1, adjustedColor.x, ep.colorizeAmount);
    adjustedColor.y                 = Lerp(1, adjustedColor.y, ep.colorizeAmount);
    adjustedColor.z                 = Lerp(1, adjustedColor.z, ep.colorizeAmount);

    auto& sm                        = ShaderManager::i();

    (*elementCB_)->adjustedColor    = adjustedColor;
    (*elementCB_)->premultiplyAlpha = ep.premultiplyAlpha;

    elementCB_->Update(ctx);
    ctx->PSSetConstantBuffers(1, 1, elementCB_->buffer().GetAddressOf());
}

void WheelRenderer::UpdateElementConstantBuffer(const ElementParams& ep, bool shadow, ID3D11DeviceContext* ctx) const
{
    vec4 adjustedColor                = ep.color;
    adjustedColor.x                   = Lerp(1, adjustedColor.x, ep.colorizeAmount);
    adjustedColor.y                   = Lerp(1, adjustedColor.y, ep.colorizeAmount);
    adjustedColor.z                   = Lerp(1, adjustedColor.z, ep.colorizeAmount);

    auto& sm                          = ShaderManager::i();

    (*elementCB_)->elementHoverFadeIn = ep.fadeIn;
    (*elementCB_)->adjustedColor      = shadow ? vec4{ 0.f, 0.f, 0.f, ep.shadowStrength } : adjustedColor;
    (*elementCB_)->premultiplyAlpha   = shadow ? false : ep.premultiplyAlpha;

    elementCB_->Update(ctx);
    ID3D11Buffer* cbs[] = { wheelCB_->buffer().Get(), elementCB_->buffer().Get() };
    ctx->PSSetConstantBuffers(0, u32(std::size(cbs)), cbs);

    auto& vscb             = *Core::i().vertexCB();
    vscb->spriteDimensions = ep.spriteDimensions;
    if (shadow)
    {
        vscb->spriteDimensions.z *= 1.05f + ep.fadeIn * 0.04f;
        vscb->spriteDimensions.w *= 1.05f + ep.fadeIn * 0.04f;
    }
    vscb->spriteZ = shadow ? 0.f : 0.02f + ep.fadeIn * 0.04f;
    vscb.Update(ctx);
    ctx->VSSetConstantBuffers(0, 1, vscb.buffer().GetAddressOf());
}

void WheelRenderer::DrawElement(ElementParams ep, size_t elementCount, ID3D11DeviceContext* ctx) const
{
    ep.fadeIn = SmoothStep(ep.fadeIn);

    const vec2 elementLocation{ std::cos(ep.anglePosition - M_PI_2) * 0.2f, std::sin(ep.anglePosition - M_PI_2) * 0.2f };

    ep.spriteDimensions.x += elementLocation.x * ep.spriteDimensions.z;
    ep.spriteDimensions.y += elementLocation.y * ep.spriteDimensions.w;

    float elementDiameter = std::sin(M_PI / static_cast<float>(elementCount)) * 2.f * 0.2f * 0.66f;
    if (elementCount == 1)
        elementDiameter = 2.f * 0.2f;
    else
        elementDiameter *= Lerp(1.f, 1.1f, ep.fadeIn);

    switch (elementCount)
    {
        case 1:
            ep.spriteDimensions.z *= 0.5f;
            ep.spriteDimensions.w *= 0.5f;
            break;
        case 2:
            ep.spriteDimensions.z *= 0.7f;
            ep.spriteDimensions.w *= 0.7f;
            break;
        case 3:
            ep.spriteDimensions.z *= 0.9f;
            ep.spriteDimensions.w *= 0.9f;
            break;
        case 4:
            ep.spriteDimensions.z *= 0.95f;
            ep.spriteDimensions.w *= 0.95f;
            break;
        default:
            break;
    }

    ep.spriteDimensions.z *= elementDiameter;
    ep.spriteDimensions.w *= elementDiameter * ep.aspectRatio;

    ctx->PSSetShaderResources(1, 1, &ep.sprite);

    if (ep.shadowStrength > 0.f)
    {
        UpdateElementConstantBuffer(ep, true, ctx);
        DrawScreenQuad(ctx);
    }

    UpdateElementConstantBuffer(ep, false, ctx);

    DrawScreenQuad(ctx);
}

} // namespace GW2Radial
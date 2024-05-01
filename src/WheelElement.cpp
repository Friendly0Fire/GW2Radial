#include <Core.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <ShaderManager.h>
#include <Utility.h>
#include <Wheel.h>
#include <WheelElement.h>
#include <imgui_internal.h>

namespace GW2Radial
{
ConstantBufferWPtr<WheelElement::WheelElementCB> WheelElement::cb_s;

WheelElement::WheelElement(u32 id, const std::string& nickname, const std::string& category, const std::string& displayName, const glm::vec4& color,
                           ConditionalProperties defaultProps, Texture2D tex)
    : sortingPriorityOption_(displayName + " Priority", nickname + "_priority", category, static_cast<int>(id))
    , props_("", nickname + "_props", category, defaultProps)
    , nickname_(nickname)
    , displayName_(displayName)
    , elementId_(id)
    , keybind_(nickname, displayName, category)
    , appearance_(std::move(tex))
    , color_(color)
{
    auto dev = Core::i().device();
    if (!appearance_.srv)
        appearance_ = CreateTextureFromResource(dev.Get(), Core::i().dllModule(), elementId_);

    GW2_ASSERT(appearance_.srv);

    D3D11_TEXTURE2D_DESC desc;
    appearance_.texture->GetDesc(&desc);

    aspectRatio_ = static_cast<float>(desc.Height) / static_cast<float>(desc.Width);
    texWidth_    = static_cast<float>(desc.Width);

    if (auto cb = cb_s.lock())
        cb_ = cb;
    else
    {
        cb_  = ShaderManager::i().MakeConstantBuffer<WheelElementCB>();
        cb_s = cb_;
    }
}

int WheelElement::DrawPriority(int extremumIndicator)
{
    ImVec4 col = ToImGui(color_);
    ImGui::PushStyleColor(ImGuiCol_Text, col);

    constexpr float fontOffset      = 5.f;
    constexpr float fontMultiplier  = 0.6f;
    const float     realItemSpacing = ImGui::GetStyle().ItemSpacing.x;
    auto            props           = props_.value();

    ImGui::TableNextColumn();
    if (!isBound() || props == ConditionalProperties::None)
        ImGui::PushFont(Core::i().fontItalic());
    auto displayName = displayName_;
    if (!keybind_.isSet())
        displayName += " [No keybind]";
    ImGui::TextUnformatted(displayName.c_str());
    if (!isBound() || props == ConditionalProperties::None)
        ImGui::PopFont();

    ImGui::TableNextColumn();

    static const auto previewMaxDims = []()
    {
        auto character = ImGui::CalcTextSize("X");
        ImGui::SetWindowFontScale(fontMultiplier);
        float iconA = ImGui::CalcTextSize(ICON_FA_CHECK_DOUBLE).x;
        float iconB = ImGui::CalcTextSize(ICON_FA_EYE).x;
        float iconC = ImGui::CalcTextSize(ICON_FA_HAND_POINTER).x;
        ImGui::SetWindowFontScale(1.f);

        return ImVec2((character.x + std::max({ iconA, iconB, iconC })) * 5 + ImGui::GetStyle().ItemSpacing.x * (2 + 5 - 1), character.y);
    }();

    auto previewFmt = [props, realItemSpacing](ConditionalProperties v, ConditionalProperties u, char c) mutable
    {
        if (NotNone(props & (v | u)))
        {
            const float cursorY = ImGui::GetCursorPosY();
            ImGui::TextUnformatted(&c, &c + 1);
            ImGui::SameLine();
            ImGui::SetCursorPosY(cursorY + fontOffset);
            ImGui::SetWindowFontScale(fontMultiplier);
            if (NotNone(props & v) && NotNone(props & u)) // Visible and usable
                ImGui::TextUnformatted(ICON_FA_CHECK_DOUBLE);
            else if (NotNone(props & v) && IsNone(props & u)) // Visible but not usable
                ImGui::TextUnformatted(ICON_FA_EYE);
            else
                ImGui::TextUnformatted(ICON_FA_HAND_POINTER); // Usable but not visible
            ImGui::SetWindowFontScale(1.f);
            ImGui::SameLine(0.f, realItemSpacing);
            ImGui::SetCursorPosY(cursorY);
        }
    };

    ImGui::SetNextItemWidth(ImGui::GetFrameHeight() + previewMaxDims.x);
    if (ImGui::BeginCombo(("##ConditionalProps" + nickname_).c_str(), nullptr, ImGuiComboFlags_CustomPreview))
    {
        auto chk = [&props, suffix = "##" + nickname_](ConditionalProperties p, const char* display)
        {
            bool enabled = NotNone(props & p);
            if (ImGui::Checkbox((display + suffix).c_str(), &enabled))
                props = enabled ? (props | p) : (props & ~p);
        };

        chk(ConditionalProperties::VisibleDefault, "Visible by default");
        chk(ConditionalProperties::UsableDefault, "Usable by default");

        chk(ConditionalProperties::VisibleInCombat, "Visible in combat");
        chk(ConditionalProperties::UsableInCombat, "Usable in combat");

        chk(ConditionalProperties::VisibleUnderwater, "Visible underwater");
        chk(ConditionalProperties::UsableUnderwater, "Usable underwater");

        chk(ConditionalProperties::VisibleOnWater, "Visible on water");
        chk(ConditionalProperties::UsableOnWater, "Usable on water");

        chk(ConditionalProperties::VisibleWvW, "Visible in WvW");
        chk(ConditionalProperties::UsableWvW, "Usable in WvW");

        ImGui::EndCombo();
    }
    if (ImGui::BeginComboPreview())
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
        previewFmt(ConditionalProperties::VisibleDefault, ConditionalProperties::UsableDefault, 'D');
        previewFmt(ConditionalProperties::VisibleInCombat, ConditionalProperties::UsableInCombat, 'C');
        previewFmt(ConditionalProperties::VisibleUnderwater, ConditionalProperties::UsableUnderwater, 'U');
        previewFmt(ConditionalProperties::VisibleOnWater, ConditionalProperties::UsableOnWater, 'O');
        previewFmt(ConditionalProperties::VisibleWvW, ConditionalProperties::UsableWvW, 'W');
        ImGui::PopStyleVar(2);

        ImGui::EndComboPreview();
    }

    if (props != props_.value())
        props_.value(props);

    int rv = 0;
    ImGui::TableNextColumn();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImGui::GetStyle().ItemSpacing * 0.5f);
    if (extremumIndicator != 1)
    {
        if (ImGui::Button((ICON_FA_ARROW_UP + std::string("##PriorityValueUp") + nickname_).c_str()))
            rv = 1;
    }
    if (extremumIndicator != -1)
    {
        static const auto upButtonTextSize = ImGui::CalcTextSize(ICON_FA_ARROW_UP) + ImGui::GetStyle().FramePadding * 2;

        if (extremumIndicator != 0)
            ImGui::Dummy(upButtonTextSize);

        ImGui::SameLine();
        if (ImGui::Button((ICON_FA_ARROW_DOWN + std::string("##PriorityValueDown") + nickname_).c_str()))
            rv = -1;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    return rv;
}

void WheelElement::SetShaderState(ID3D11DeviceContext* ctx) const
{
    glm::vec4 adjustedColor  = color_;
    adjustedColor.x          = Lerp(1, adjustedColor.x, colorizeAmount_);
    adjustedColor.y          = Lerp(1, adjustedColor.y, colorizeAmount_);
    adjustedColor.z          = Lerp(1, adjustedColor.z, colorizeAmount_);

    auto& sm                 = ShaderManager::i();

    (*cb_)->adjustedColor    = adjustedColor;
    (*cb_)->premultiplyAlpha = premultiplyAlpha_;

    cb_->Update(ctx);
    ctx->PSSetConstantBuffers(1, 1, cb_->buffer().GetAddressOf());
}

void WheelElement::SetShaderState(ID3D11DeviceContext* ctx, const glm::vec4& spriteDimensions, const ComPtr<ID3D11Buffer>& wheelCb, bool shadow, float hoverRatio) const
{
    glm::vec4 adjustedColor    = color_;
    adjustedColor.x            = Lerp(1, adjustedColor.x, colorizeAmount_);
    adjustedColor.y            = Lerp(1, adjustedColor.y, colorizeAmount_);
    adjustedColor.z            = Lerp(1, adjustedColor.z, colorizeAmount_);

    auto& sm                   = ShaderManager::i();

    (*cb_)->elementHoverFadeIn = hoverRatio;
    (*cb_)->adjustedColor      = shadow ? glm::vec4{ 0.f, 0.f, 0.f, shadowStrength_ } : adjustedColor;
    (*cb_)->premultiplyAlpha   = shadow ? false : premultiplyAlpha_;

    cb_->Update(ctx);
    ID3D11Buffer* cbs[] = { wheelCb.Get(), cb_->buffer().Get() };
    ctx->PSSetConstantBuffers(0, u32(std::size(cbs)), cbs);

    auto& vscb             = *Core::i().vertexCB();
    vscb->spriteDimensions = spriteDimensions;
    if (shadow)
    {
        vscb->spriteDimensions.z *= 1.05f + hoverRatio * 0.04f;
        vscb->spriteDimensions.w *= 1.05f + hoverRatio * 0.04f;
    }
    vscb->spriteZ = shadow ? 0.f : 0.02f + hoverRatio * 0.04f;
    vscb.Update(ctx);
    ctx->VSSetConstantBuffers(0, 1, vscb.buffer().GetAddressOf());
}

void WheelElement::Draw(ID3D11DeviceContext* ctx, int n, glm::vec4 spriteDimensions, size_t activeElementsCount, const mstime& currentTime, const WheelElement* elementHovered,
                        const Wheel* parent)
{
    const float hoverTimer   = SmoothStep(hoverFadeIn(currentTime, parent));

    float       elementAngle = static_cast<float>(n) / static_cast<float>(activeElementsCount) * 2 * static_cast<float>(M_PI);
    if (activeElementsCount == 1)
        elementAngle = 0;
    const glm::vec2 elementLocation{ cos(elementAngle - static_cast<float>(M_PI) / 2) * 0.2f, sin(elementAngle - static_cast<float>(M_PI) / 2) * 0.2f };

    spriteDimensions.x += elementLocation.x * spriteDimensions.z;
    spriteDimensions.y += elementLocation.y * spriteDimensions.w;

    float elementDiameter = static_cast<float>(sin((2 * M_PI / static_cast<double>(activeElementsCount)) / 2)) * 2.f * 0.2f * 0.66f;
    if (activeElementsCount == 1)
        elementDiameter = 2.f * 0.2f;
    else
        elementDiameter *= Lerp(1.f, 1.1f, hoverTimer);

    switch (activeElementsCount)
    {
        case 1:
            spriteDimensions.z *= 0.5f;
            spriteDimensions.w *= 0.5f;
            break;
        case 2:
            spriteDimensions.z *= 0.7f;
            spriteDimensions.w *= 0.7f;
            break;
        case 3:
            spriteDimensions.z *= 0.9f;
            spriteDimensions.w *= 0.9f;
            break;
        case 4:
            spriteDimensions.z *= 0.95f;
            spriteDimensions.w *= 0.95f;
            break;
        default:
            break;
    }

    spriteDimensions.z *= elementDiameter;
    spriteDimensions.w *= elementDiameter;

    spriteDimensions.w *= aspectRatio_;

    ctx->PSSetShaderResources(1, 1, appearance_.srv.GetAddressOf());

    if (shadowStrength_ > 0.f)
    {
        SetShaderState(ctx, spriteDimensions, parent->GetConstantBuffer(), true, hoverTimer);

        DrawScreenQuad(ctx);
    }

    SetShaderState(ctx, spriteDimensions, parent->GetConstantBuffer(), false, hoverTimer);

    DrawScreenQuad(ctx);
}

float WheelElement::hoverFadeIn(const mstime& currentTime, const Wheel* parent) const
{
    const auto hoverIn  = std::min(1.f, (currentTime - std::max(currentHoverTime_, parent->currentTriggerTime_ + parent->displayDelayOption_.value())) / 1000.f * 6);
    const auto hoverOut = 1.f - std::min(1.f, (currentTime - std::max(currentExitTime_, parent->currentTriggerTime_ + parent->displayDelayOption_.value())) / 1000.f * 6);

    return parent->currentHovered_ == this ? hoverIn : std::min(hoverIn, hoverOut);
}
} // namespace GW2Radial

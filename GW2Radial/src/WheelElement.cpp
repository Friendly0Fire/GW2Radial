#include <Core.h>
#include <ShaderManager.h>
#include <Utility.h>
#include <VSCB.h>
#include <Wheel.h>
#include <WheelElement.h>
#include <common/IconFontCppHeaders/IconsFontAwesome5.h>

namespace GW2Radial
{
ConstantBuffer<WheelElement::WheelElementCB> WheelElement::cb_s;

WheelElement::WheelElement(uint id, const std::string& nickname, const std::string& category, const std::string& displayName, const glm::vec4& color,
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
        appearance_ = CreateTextureFromResource<ID3D11Texture2D>(dev.Get(), Core::i().dllModule(), elementId_);

    GW2_ASSERT(appearance_.srv);

    D3D11_TEXTURE2D_DESC desc;
    appearance_.texture->GetDesc(&desc);

    aspectRatio_ = static_cast<float>(desc.Height) / static_cast<float>(desc.Width);
    texWidth_    = static_cast<float>(desc.Width);

    if (!cb_s.IsValid())
        cb_s = ShaderManager::i().MakeConstantBuffer<WheelElementCB>();
}

int WheelElement::DrawPriority(int extremumIndicator)
{
    ImVec4 col = ToImGui(color_);
    ImGui::PushStyleColor(ImGuiCol_Text, col);

    auto        props           = props_.value();
    const float realItemSpacing = ImGui::GetStyle().ItemSpacing.x;

    auto        previewFmt      = [props, realItemSpacing](ConditionalProperties v, ConditionalProperties u, char c) mutable
    {
        if (notNone(props & (v | u)))
        {
            ImGui::TextUnformatted(&c, &c + 1);
            ImGui::SameLine();
            ImGui::PushFont(Core::i().fontIcon());
            ImGui::SetWindowFontScale(0.9f);
            if (notNone(props & v) && notNone(props & u)) // Visible and usable
                ImGui::TextUnformatted(ICON_FA_CHECK_DOUBLE);
            else if (notNone(props & v) && isNone(props & u)) // Visible but not usable
                ImGui::TextUnformatted(ICON_FA_EYE);
            else
                ImGui::TextUnformatted(ICON_FA_HAND_POINTER); // Usable but not visible
            ImGui::SetWindowFontScale(1.f);
            ImGui::PopFont();
            ImGui::SameLine(0.f, realItemSpacing);
        }
    };

    ImGui::TableNextColumn();
    float cursorX = ImGui::GetCursorPosX();

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
    previewFmt(ConditionalProperties::VISIBLE_DEFAULT, ConditionalProperties::USABLE_DEFAULT, 'D');
    previewFmt(ConditionalProperties::VISIBLE_IN_COMBAT, ConditionalProperties::USABLE_IN_COMBAT, 'C');
    previewFmt(ConditionalProperties::VISIBLE_UNDERWATER, ConditionalProperties::USABLE_UNDERWATER, 'U');
    previewFmt(ConditionalProperties::VISIBLE_ON_WATER, ConditionalProperties::USABLE_ON_WATER, 'O');
    previewFmt(ConditionalProperties::VISIBLE_WVW, ConditionalProperties::USABLE_WVW, 'W');
    ImGui::PopStyleVar(2);

    static const float previewMaxWidth = []()
    {
        float character = ImGui::CalcTextSize("X").x;
        ImGui::PushFont(Core::i().fontIcon());
        ImGui::SetWindowFontScale(0.9f);
        float iconA = ImGui::CalcTextSize(ICON_FA_CHECK_DOUBLE).x;
        float iconB = ImGui::CalcTextSize(ICON_FA_EYE).x;
        float iconC = ImGui::CalcTextSize(ICON_FA_HAND_POINTER).x;
        ImGui::SetWindowFontScale(1.f);
        ImGui::PopFont();

        return (character + std::max({ iconA, iconB, iconC })) * 5 + ImGui::GetStyle().ItemSpacing.x * (2 + 4 - 1);
    }();
    ImGui::SetCursorPosX(cursorX + previewMaxWidth);

    if (ImGui::BeginCombo(("##ConditionalProps" + nickname_).c_str(), nullptr, ImGuiComboFlags_NoPreview))
    {
        auto chk = [&props, suffix = "##" + nickname_](ConditionalProperties p, const char* display)
        {
            bool enabled = notNone(props & p);
            if (ImGui::Checkbox((display + suffix).c_str(), &enabled))
                props = enabled ? (props | p) : (props & ~p);
        };

        chk(ConditionalProperties::VISIBLE_DEFAULT, "Visible by default");
        chk(ConditionalProperties::USABLE_DEFAULT, "Usable by default");

        chk(ConditionalProperties::VISIBLE_IN_COMBAT, "Visible in combat");
        chk(ConditionalProperties::USABLE_IN_COMBAT, "Usable in combat");

        chk(ConditionalProperties::VISIBLE_UNDERWATER, "Visible underwater");
        chk(ConditionalProperties::USABLE_UNDERWATER, "Usable underwater");

        chk(ConditionalProperties::VISIBLE_ON_WATER, "Visible on water");
        chk(ConditionalProperties::USABLE_ON_WATER, "Usable on water");

        chk(ConditionalProperties::VISIBLE_WVW, "Visible in WvW");
        chk(ConditionalProperties::USABLE_WVW, "Usable in WvW");

        ImGui::EndCombo();
    }

    if (props != props_.value())
        props_.value(props);

    ImGui::TableNextColumn();
    if (!isBound() || props == ConditionalProperties::NONE)
        ImGui::PushFont(Core::i().fontItalic());
    auto displayName = displayName_;
    if (!keybind_.isSet())
        displayName += " [No keybind]";
    ImGui::TextUnformatted(displayName.c_str());
    if (!isBound() || props == ConditionalProperties::NONE)
        ImGui::PopFont();

    ImGui::PushFont(Core::i().fontIcon());

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
    ImGui::PopFont();
    ImGui::PopStyleColor();

    return rv;
}

void WheelElement::SetShaderState(ID3D11DeviceContext* ctx) const
{
    glm::vec4 adjustedColor = color_;
    adjustedColor.x         = Lerp(1, adjustedColor.x, colorizeAmount_);
    adjustedColor.y         = Lerp(1, adjustedColor.y, colorizeAmount_);
    adjustedColor.z         = Lerp(1, adjustedColor.z, colorizeAmount_);

    auto& sm                = ShaderManager::i();

    cb_s->adjustedColor     = adjustedColor;
    cb_s->premultiplyAlpha  = premultiplyAlpha_;

    cb_s.Update(ctx);
    ctx->PSSetConstantBuffers(1, 1, cb_s.buffer().GetAddressOf());
}

void WheelElement::SetShaderState(ID3D11DeviceContext* ctx, const fVector4& spriteDimensions, const ComPtr<ID3D11Buffer>& wheelCb, bool shadow, float hoverRatio) const
{
    glm::vec4 adjustedColor  = color_;
    adjustedColor.x          = Lerp(1, adjustedColor.x, colorizeAmount_);
    adjustedColor.y          = Lerp(1, adjustedColor.y, colorizeAmount_);
    adjustedColor.z          = Lerp(1, adjustedColor.z, colorizeAmount_);

    auto& sm                 = ShaderManager::i();

    cb_s->elementHoverFadeIn = hoverRatio;
    cb_s->adjustedColor      = shadow ? glm::vec4{ 0.f, 0.f, 0.f, shadowStrength_ } : adjustedColor;
    cb_s->premultiplyAlpha   = shadow ? false : premultiplyAlpha_;

    cb_s.Update(ctx);
    ID3D11Buffer* cbs[] = { wheelCb.Get(), cb_s.buffer().Get() };
    ctx->PSSetConstantBuffers(0, uint(std::size(cbs)), cbs);

    auto& vscb             = GetVSCB();
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

void WheelElement::Draw(ID3D11DeviceContext* ctx, int n, fVector4 spriteDimensions, size_t activeElementsCount, const mstime& currentTime, const WheelElement* elementHovered,
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

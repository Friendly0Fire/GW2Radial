#include <Core.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <ShaderManager.h>
#include <Utility.h>
#include <Wheel.h>
#include <WheelElement.h>
#include <imgui_internal.h>

namespace GW2Radial
{
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
    ImGui::AlignTextToFramePadding();
    if (!isBound() || props == ConditionalProperties::None)
        ImGui::PushFont(Core::i().fontItalic());
    auto displayName = displayName_;
    if (!keybind_.isSet())
        displayName += " [No keybind]";
    ImGui::TextUnformatted(displayName.c_str());
    if (!isBound() || props == ConditionalProperties::None)
        ImGui::PopFont();

    ImGui::TableNextColumn();

    if (!disableBehaviorControls_)
    {
        static const auto previewMaxDims = []
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
                ImGui::SetWindowFontScale(fontMultiplier);
                if (NotNone(props & v) && NotNone(props & u)) // Visible and usable
                    ImGui::TextUnformatted(ICON_FA_CHECK_DOUBLE);
                else if (NotNone(props & v) && IsNone(props & u)) // Visible but not usable
                    ImGui::TextUnformatted(ICON_FA_EYE);
                else
                    ImGui::TextUnformatted(ICON_FA_HAND_POINTER); // Usable but not visible
                ImGui::SetWindowFontScale(1.f);
                ImGui::SameLine(0.f, realItemSpacing);
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
    }

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
} // namespace GW2Radial

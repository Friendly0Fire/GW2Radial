#include <Core.h>
#include <GFXSettings.h>
#include <ImGuiExtensions.h>
#include <ImGuiPopup.h>
#include <Input.h>
#include <Main.h>
#include <MumbleLink.h>
#include <ShaderManager.h>
#include <Utility.h>
#include <Wheel.h>
#include <algorithm>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <utility>

namespace GW2Radial
{
Wheel::Favorite Wheel::MakeDefaultFavorite()
{
    Favorite fav;
    fav.bits.baseline   = 0;
    fav.bits.inWvW      = -1;
    fav.bits.inCombat   = -1;
    fav.bits.onWater    = -1;
    fav.bits.underwater = -1;
    return fav;
}

Wheel::Wheel(std::shared_ptr<Texture2D> bgTexture, std::string nickname, std::string displayName)
    : nickname_(std::move(nickname))
    , displayName_(std::move(displayName))
    , keybind_(nickname_, "Show on mouse", nickname_)
    , centralKeybind_(nickname_ + "_cl", "Show in center", nickname_)
    , centerBehaviorOption_("Center behavior", "center_behavior", "wheel_" + nickname_)
    , centerFavoriteOption_("Favorite choice##Center", "center_favorite.2", "wheel_" + nickname_, MakeDefaultFavorite())
    , delayFavoriteOption_("Favorite choice##Delay", "delay_favorite", "wheel_" + nickname_, MakeDefaultFavorite())
    , scaleOption_("Scale", "scale", "wheel_" + nickname_, 1.f)
    , centerScaleOption_("Center scale", "center_scale", "wheel_" + nickname_, 0.2f)
    , displayDelayOption_("Pop-up delay", "delay", "wheel_" + nickname_)
    , animationTimeOption_("Fade-in time", "anim_time", "wheel_" + nickname_, 750)
    , resetCursorOnLockedKeybindOption_("Reset cursor to center with Center Locked keybind", "reset_cursor_cl", "wheel_" + nickname_, true)
    , lockCameraWhenOverlayedOption_("Lock camera when overlay is displayed", "lock_camera", "wheel_" + nickname_, true)
    , triggerOption_("Region trigger mode", "trigger_mode", "wheel_" + nickname_, TriggerBehavior::Release)
    , behaviorOnReleaseBeforeDelay_("Behavior when released before pop-up delay has elapsed", "behavior_before_delay", "wheel_" + nickname_)
    , resetCursorAfterKeybindOption_("Move cursor to original location after release", "reset_cursor_after", "wheel_" + nickname_, true)
    , maximumConditionalWaitTimeOption_("Expiration time of queued input", "max_wait_cond", "wheel_" + nickname_, 30)
    , conditionalDelayDelayOption_("Delay before sending queued input", "min_delay_cond", "wheel_" + nickname_, 200)
    , showDelayTimerOption_("Show timer next to skill bar when waiting to send input", "timer_ooc", "wheel_" + nickname_, true)
    , centerCancelDelayedInputOption_("Cancel queued input with center region", "queue_center_cancel", "wheel_" + nickname_, false)
    , enableConditionsOption_("Enable conditional keybinds", "conditions_enabled", "wheel_" + nickname_, false)
    , enableQueuingOption_("Enable input queuing", "queuing_enabled", "wheel_" + nickname_, true)
    , enableSkipOWOption_("Enable fast on water mode", "skip_ow_enabled", "wheel_" + nickname_, true)
    , enableSkipUWOption_("Enable fast underwater mode", "skip_uw_enabled", "wheel_" + nickname_, true)
    , enableSkipWvWOption_("Enable fast WvW mode", "skip_wvw_enabled", "wheel_" + nickname_, true)
    , visibleInMenuOption_(displayName_ + "##Visible", "menu_visible", "wheel_" + nickname_, true)
    , opacityMultiplierOption_("Opacity multiplier", "opacity", "wheel_" + nickname_, 100)
    , animationScale_("Animation scale", "anim_scale", "wheel_" + nickname_, 1.f)
{
    conditions_ = std::make_shared<ConditionSet>("wheel_" + nickname_);
    conditions_->enable(enableConditionsOption_.value());
    keybind_.conditions(conditions_);
    centralKeybind_.conditions(conditions_);
    keybind_.callback([&](Activated a) { return KeybindEvent(false, a); });
    centralKeybind_.callback([&](Activated a) { return KeybindEvent(true, a); });

    mouseMoveCallbackID_   = Input::i().mouseMoveEvent().AddCallback([this](bool& rv) { OnMouseMove(rv); });
    mouseButtonCallbackID_ = Input::i().mouseButtonEvent().AddCallback([this](EventKey ek, bool& rv) { OnMouseButton(ek.sc, ek.down, rv); });

    SettingsMenu::i().AddImplementer(this);
}

Wheel::~Wheel()
{
    Input::f(
        [&](auto& i)
        {
            i.mouseMoveEvent().RemoveCallback(std::move(mouseMoveCallbackID_));
            i.mouseButtonEvent().RemoveCallback(std::move(mouseButtonCallbackID_));
        });
    SettingsMenu::f([&](auto& i) { i.RemoveImplementer(this); });
}

void Wheel::UpdateHover()
{
    const auto& io = ImGui::GetIO();

    glm::vec2   mousePos;
    mousePos.x = io.MousePos.x / float(Core::i().screenWidth());
    mousePos.y = io.MousePos.y / float(Core::i().screenHeight());
    mousePos.x -= currentPosition_.x;
    mousePos.y -= currentPosition_.y;

    mousePos.y *= float(Core::i().screenHeight()) / float(Core::i().screenWidth());

    WheelElement* lastHovered    = currentHovered_;

    auto          activeElements = GetVisibleElements(MumbleLink::i().currentState());

    float         mpLenSq        = mousePos.x * mousePos.x + mousePos.y * mousePos.y;

    // Middle circle does not count as a hover event
    if (!activeElements.empty() && mpLenSq > Square(scaleOption_.value() * 0.125f * 0.8f * centerScaleOption_.value()))
    {
        float mouseAngle = atan2(-mousePos.y, -mousePos.x) - 0.5f * float(M_PI);
        if (mouseAngle < 0)
            mouseAngle += float(2 * M_PI);

        const float elementAngle = float(2 * M_PI) / activeElements.size();
        const int   elementId    = int((mouseAngle - elementAngle / 2) / elementAngle + 1) % int(activeElements.size());

        currentHovered_          = activeElements[elementId];
    }
    else
        currentHovered_ = GetCenterHoveredElement();

    if (lastHovered != currentHovered_)
    {
        const auto time = TimeInMilliseconds();

        if (lastHovered)
            lastHovered->currentExitTime(time);
        if (currentHovered_)
            currentHovered_->currentHoverTime(time);
    }
}

void Wheel::DrawMenu(Keybind** currentEditedKeybind)
{
    ImGui::PushID((nickname_ + "Elements").c_str());

    UI::Title("In-game Keybinds");

    ImGui::TextUnformatted("Set the following to your in-game keybinds (F11, Control Options).");

    for (auto& we : wheelElements_)
        ImGui::KeybindInput(we->keybind(), currentEditedKeybind, nullptr);

    UI::Title("Keybinds");

    ImGui::KeybindInput((Keybind&)keybind_, currentEditedKeybind, "Pressing this key combination will open the radial menu at your cursor's current location.");
    ImGui::KeybindInput((Keybind&)centralKeybind_, currentEditedKeybind,
                        "Pressing this key combination will open the radial menu in the middle of the screen. Your cursor will be moved to the middle of the screen and moved back "
                        "after you have selected an option.");

    MenuSectionKeybinds(currentEditedKeybind);

    if (ImGui::ConfigurationWrapper(&ImGui::Checkbox, enableConditionsOption_) && conditions_)
        conditions_->enable(enableConditionsOption_.value());

    {
        UI::Scoped::Disable disable(!conditions_ || !enableConditionsOption_.value());

        ImGui::Indent();

        conditions_->DrawMenu();

        ImGui::Unindent();
    }

    UI::Title("Display Options");

    ImGui::ConfigurationWrapper(&ImGui::SliderInt, opacityMultiplierOption_, 0, 100, "%d %%", ImGuiSliderFlags_AlwaysClamp);
    UI::HelpTooltip("Transparency of the entire overlay. Setting to 0% hides it entirely.");

    ImGui::ConfigurationWrapper(&ImGui::SliderInt, animationTimeOption_, 0, 2000, "%d ms", ImGuiSliderFlags_AlwaysClamp);
    UI::HelpTooltip("Amount of time, in milliseconds, for the radial menu to fade in.");

    ImGui::ConfigurationWrapper(&ImGui::SliderFloat, animationScale_, 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    UI::HelpTooltip("Intensity of the 3D animations.");

    ImGui::ConfigurationWrapper(&ImGui::SliderFloat, scaleOption_, 0.25f, 4.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    UI::HelpTooltip("Scale factor for the size of the whole radial menu.");

    ImGui::ConfigurationWrapper(&ImGui::SliderFloat, centerScaleOption_, 0.05f, 0.5f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    UI::HelpTooltip("Scale factor for the size of just the central region of the radial menu.");

    ImGui::ConfigurationWrapper(&ImGui::SliderInt, displayDelayOption_, 0, 3000, "%d ms", ImGuiSliderFlags_AlwaysClamp);
    UI::HelpTooltip("Amount of time, in milliseconds, to wait before displaying the radial menu. The input bound to the central region can still be sent by releasing the key, "
                    "even before the menu is visible.");

    MenuSectionDisplay();

    UI::Title("Interaction Options");

    ImGui::Combo(triggerOption_.displayName().c_str(), reinterpret_cast<int*>(&triggerOption_.value()), "Release\0Click\0Hover\0");
    UI::HelpTooltip("Controls how the items of the menu are activated.\n"
                    "Release: Default, activates whatever item is under the mouse cursor when the wheel shortcut is released.\n"
                    "Click: activates the item by a left mouse button click. Any key pressed to activate the menu can be released immediately without dismissing it.\n"
                    "Hover: activates whichever item is hovered first. Any key pressed to activate the menu can be released immediately without dismissing it.");

    auto favoriteCombo = [](const std::vector<std::unique_ptr<WheelElement>>& elements, ConfigurationOption<Favorite>& opt)
    {
        Favorite fav    = opt.value();

        auto     single = [&opt](int v, const char* prefix, const char* tooltip, std::ranges::forward_range auto&& elements, bool showDefault = true)
        {
            static const char* defaultString = "(default value)";
            if (ImGui::BeginCombo((prefix + opt.displayName()).c_str(),
                                  v >= 0 ? std::get<1>(*std::ranges::next(std::ranges::begin(elements), v))->displayName().c_str() : defaultString))
            {
                if (showDefault)
                    if (ImGui::Selectable(defaultString, v == -1))
                        v = -1;

                for (auto&& [index, element] : elements)
                {
                    if (ImGui::Selectable(element->displayName().c_str(), index == v))
                        v = static_cast<int>(index);
                }
                ImGui::EndCombo();
            }
            UI::HelpTooltip(tooltip);

            return v;
        };

        namespace rv      = std::ranges::views;

        const auto& all   = elements | rv::enumerate;
        auto        wvw   = elements | rv::enumerate | rv::filter([](auto&& e) { return std::get<1>(e)->isUsable(ConditionalState::InWvW); });

        fav.bits.baseline = single(fav.bits.baseline, "Default ", "This determines the default favorite option.", all, false);
        if (!std::ranges::empty(wvw | rv::drop(1)))
            fav.bits.inWvW = single(fav.bits.inWvW, "In WvW ", "This determines the favorite option when in WvW.", wvw);
        else
            fav.bits.inWvW = 0;
        fav.bits.inCombat   = single(fav.bits.inCombat, "In combat ", "This determines the favorite option when in combat.", all);
        fav.bits.onWater    = single(fav.bits.onWater, "On water ", "This determines the favorite option when on the water surface.", all);
        fav.bits.underwater = single(fav.bits.underwater, "Underwater ", "This determines the favorite option when underwater.", all);

        if (fav.value != opt.value().value)
            opt.value(fav);
    };

    {
        UI::Scoped::Disable disable(displayDelayOption_.value() == 0);

        ImGui::Text((behaviorOnReleaseBeforeDelay_.displayName() + ":").c_str());

        bool (*rb)(const char*, int*, int) = &ImGui::RadioButton;
        ImGui::ConfigurationWrapper(rb, "Nothing##ReleaseBeforeDelay", behaviorOnReleaseBeforeDelay_, int(BehaviorBeforeDelay::Nothing));
        ImGui::SameLine();
        ImGui::ConfigurationWrapper(rb, "Previous##ReleaseBeforeDelay", behaviorOnReleaseBeforeDelay_, int(BehaviorBeforeDelay::Previous));
        ImGui::SameLine();
        ImGui::ConfigurationWrapper(rb, "Favorite##ReleaseBeforeDelay", behaviorOnReleaseBeforeDelay_, int(BehaviorBeforeDelay::Favorite));
        ImGui::SameLine();
        ImGui::ConfigurationWrapper(rb, "As if opened##ReleaseBeforeDelay", behaviorOnReleaseBeforeDelay_, int(BehaviorBeforeDelay::Direction));
        ImGui::SameLine();
        ImGui::ConfigurationWrapper(rb, "Pass to game##ReleaseBeforeDelay", behaviorOnReleaseBeforeDelay_, int(BehaviorBeforeDelay::PassToGame));
        UI::HelpTooltip(
            "Determines the behavior of the menu if it is dismissed before it becomes visible. By default, it does nothing, but it can also (1) trigger the last selected item; "
            "(2) trigger a fixed \"favorite\" option; (3) function exactly as if the menu was opened, making it possible to select an item with the mouse without seeing them; "
            "(4) forward the input to the game.");

        if (BehaviorBeforeDelay(behaviorOnReleaseBeforeDelay_.value()) == BehaviorBeforeDelay::Favorite)
        {
            const auto textSize = ImGui::CalcTextSize(delayFavoriteOption_.displayName().c_str());
            const auto itemSize = ImGui::CalcItemWidth() - textSize.x - ImGui::GetCurrentWindowRead()->WindowPadding.x;

            favoriteCombo(wheelElements_, delayFavoriteOption_);
        }
    }

    {
        UI::Scoped::Disable disable(triggerOption_.value() == TriggerBehavior::Hover);

        ImGui::Text((centerBehaviorOption_.displayName() + ":").c_str());
        ImGui::SameLine();

        bool (*rb)(const char*, int*, int) = &ImGui::RadioButton;
        ImGui::ConfigurationWrapper(rb, "Nothing##CenterBehavior", centerBehaviorOption_, int(CenterBehavior::Nothing));
        ImGui::SameLine();
        ImGui::ConfigurationWrapper(rb, "Previous##CenterBehavior", centerBehaviorOption_, int(CenterBehavior::Previous));
        ImGui::SameLine();
        ImGui::ConfigurationWrapper(rb, "Favorite##CenterBehavior", centerBehaviorOption_, int(CenterBehavior::Favorite));
        ImGui::SameLine();
        ImGui::ConfigurationWrapper(rb, "Pass to game##CenterBehavior", centerBehaviorOption_, int(CenterBehavior::PassToGame));
        UI::HelpTooltip("Determines the behavior of the central region of the menu. By default, it does nothing, but it can alternatively: "
                        "(1) trigger the last selected item; "
                        "(2) trigger a fixed \"favorite\" option;"
                        "(3) forward the input to the game.");

        if (CenterBehavior(centerBehaviorOption_.value()) == CenterBehavior::Favorite)
        {
            const auto textSize = ImGui::CalcTextSize(centerFavoriteOption_.displayName().c_str());
            const auto itemSize = ImGui::CalcItemWidth() - textSize.x - ImGui::GetCurrentWindowRead()->WindowPadding.x;

            favoriteCombo(wheelElements_, centerFavoriteOption_);
        }
    }

    ImGui::ConfigurationWrapper(&ImGui::Checkbox, resetCursorOnLockedKeybindOption_);
    UI::HelpTooltip("Moves the cursor to the center of the screen when the \"show in center\" keybind is used.");

    ImGui::ConfigurationWrapper(&ImGui::Checkbox, lockCameraWhenOverlayedOption_);
    UI::HelpTooltip("Prevents the camera from being affected by mouse movements while the menu is displayed.");

    if (!alwaysResetCursorPositionBeforeKeyPress_)
    {
        ImGui::ConfigurationWrapper(&ImGui::Checkbox, resetCursorAfterKeybindOption_);
        UI::HelpTooltip("Once the menu is dismissed, moves the cursor to where it was on screen before the menu was displayed.");
    }

    MenuSectionInteraction();

    UI::Title("Queuing Options");

    ImGui::ConfigurationWrapper(&ImGui::Checkbox, enableQueuingOption_);
    UI::HelpTooltip("If sending a keybind now would be ignored by the game (e.g., mounting while in combat), enabling queuing will \"queue\" the input until all necessary "
                    "conditions are satisfied.");

    ImGui::ConfigurationWrapper(&ImGui::Checkbox, enableSkipOWOption_);
    UI::HelpTooltip("If only one item is available on water (e.g., mounting while on water surface with Skimmer unlocked, but Turtle disabled), "
                    "bypass showing the radial menu and trigger the input immediately. May not always work in Janthir Wilds maps or later.");
    ImGui::ConfigurationWrapper(&ImGui::Checkbox, enableSkipUWOption_);
    UI::HelpTooltip("If only one item is available underwater (e.g., mounting while underwater with Skimmer unlocked and fully mastered, but Turtle disabled), "
                    "bypass showing the radial menu and trigger the input immediately. May not always work in Janthir Wilds maps or later.");
    ImGui::ConfigurationWrapper(&ImGui::Checkbox, enableSkipWvWOption_);
    UI::HelpTooltip("If only one item is available in WvW (e.g., mounting in WvW), bypass showing the radial menu and trigger the input immediately.");

    {
        UI::Scoped::Disable disable(!enableQueuingOption_.value());

        ImGui::PushItemWidth(0.33f * ImGui::GetWindowWidth());
        ImGui::ConfigurationWrapper(&ImGui::InputIntFormat, conditionalDelayDelayOption_, "%d ms", 1, 100, 0);
        UI::HelpTooltip("Time, in milliseconds, to wait before sending a queued input once all conditions pass. Increasing this value can make activation more reliable.");
        ImGui::ConfigurationWrapper(&ImGui::InputIntFormat, maximumConditionalWaitTimeOption_, "%d s", 1, 10, 0);
        UI::HelpTooltip("Maximum amount of time, in seconds, to wait with a queued input before dismissing it.");
        ImGui::PopItemWidth();

        {
            UI::Scoped::Disable disable2(triggerOption_.value() == TriggerBehavior::Hover);
            ImGui::ConfigurationWrapper(&ImGui::Checkbox, centerCancelDelayedInputOption_);
            UI::HelpTooltip("If the center region has no bound behavior, this option makes it cancel any queued input.");
        }

        MenuSectionQueuing();
    }

    UI::Title("Visibility & Ordering");

    ImGui::Text("Ordering top to bottom is clockwise starting at noon.");

    // FIXME
#if 0
    UI::HelpTooltip({
        {UI::HelpTooltipElementType::DEFAULT,
         "Show/Use Conditions control the behavior of each menu item depending on current circumstances. The character's state (in combat, on or under water, and in WvW) "
          "can be taken into account to modify displayed and usable items."                                                                                              },
        { UI::HelpTooltipElementType::BULLET, "A \"displayed\" item is shown in the menu."                                                                              },
        { UI::HelpTooltipElementType::BULLET,
         "A \"usable\" item is considered to be possible to activate in the current context, but is not necessarily displayed on the radial menu. "
          "This can be useful for \"fast mode\", e.g. having the Skimmer be the only usable mount underwater and not displaying it, "
          "making it hidden in normal play but instantly triggering it when underwater."                                                                                 },
        { UI::HelpTooltipElementType::BULLET, "A \"displayed\" but not \"usable\" item will be queued (if queuing is enabled) until such a time it is marked as usable."}
    });
#endif

    ImGui::PushStyleColor(ImGuiCol_TableRowBg, 0);
    ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, (ImGui::GetColorU32(ImGuiCol_FrameBg) & 0xFFFFFF) | 0x33000000);
    if (ImGui::BeginTable("##OrderingTable", 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Show/Use Conditions", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("##UpDown", ImGuiTableColumnFlags_WidthFixed);

        ImGui::TableHeadersRow();

        for (auto it = sortedWheelElements_.begin(); it != sortedWheelElements_.end(); ++it)
        {
            const auto extremum = it == sortedWheelElements_.begin() ? 1 : it == std::prev(sortedWheelElements_.end()) ? -1 : 0;
            auto&      e        = *it;
            if (const auto dir = e->DrawPriority(extremum); dir != 0)
            {
                if (dir == 1 && e == sortedWheelElements_.front() || dir == -1 && e == sortedWheelElements_.back())
                    continue;

                auto& eOther = dir == 1 ? *std::prev(it) : *std::next(it);
                std::swap(e, eOther);
                const auto tempPriority = eOther->sortingPriority();
                eOther->sortingPriority(e->sortingPriority());
                e->sortingPriority(tempPriority);
            }
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleColor(2);

    MenuSectionMisc();

    ImGui::PopID();
}

void Wheel::OnUpdate()
{
    if (showEmptyPopup_)
        ImGuiPopup("Radial menu missing keybinds")
            .Position({ 0.5f, 0.45f })
            .Size({ 400.f, 200.f }, false)
            .Display([&](const ImVec2&) { ImGui::TextWrapped("A radial menu was triggered, but no keybinds are currently bound for it, so nothing could be shown."); },
                     [&]() { showEmptyPopup_ = false; });

    auto& cd = conditionalDelay_;
    if (OptHasValue(cd.element))
    {
        const auto currentTime = TimeInMilliseconds();
        if (currentTime > cd.time + maximumConditionalWaitTimeOption_.value() * 1000ull)
            ResetConditionallyDelayed(true, currentTime);
        else
        {
            if (cd.immediate)
            {
                if (cd.time <= currentTime && (std::holds_alternative<Keybind*>(cd.element) || CanActivate(std::get<WheelElement*>(cd.element))))
                {
                    auto kb = GetKeybindFromOpt(cd.element);
                    if (kb)
                        Input::i().SendKeybind(kb->keyCombo(), std::nullopt);

                    if (clearConditionalDelayOnSend_)
                        ResetConditionallyDelayed(false, currentTime);
                    else
                    {
                        if (cd.testPasses)
                        {
                            cd.time       = currentTime + 1000;
                            cd.testPasses = false;
                        }
                        else
                            cd.hidden = cd.immediate = false;
                    }
                }
            }
            else
            {
                if (std::holds_alternative<Keybind*>(cd.element) || CanActivate(std::get<WheelElement*>(cd.element)))
                {
                    if (!cd.testPasses)
                        cd.testPassesTime = currentTime;
                    else if (currentTime >= cd.testPassesTime + conditionalDelayDelayOption_.value())
                    {
                        auto kb = GetKeybindFromOpt(cd.element);
                        if (kb)
                            Input::i().SendKeybind(kb->keyCombo(), std::nullopt);
                        if (clearConditionalDelayOnSend_)
                            ResetConditionallyDelayed(true, currentTime);
                        else
                            cd.testPassesTime = currentTime + 3000 - conditionalDelayDelayOption_.value(); // At most retry once every 3 seconds
                    }
                    cd.testPasses = true;
                }
                else
                    cd.testPasses = false;
            }
        }
    }

    const auto resetCursorPositionToCenter = [&]
    {
        RECT rect = {};
        if (GetWindowRect(Core::i().gameWindow(), &rect))
        {
            if (SetCursorPos((rect.right - rect.left) / 2 + rect.left, (rect.bottom - rect.top) / 2 + rect.top))
            {
                auto& io    = ImGui::GetIO();
                io.MousePos = io.DisplaySize * 0.5f;
            }
        }
        resetCursorPositionToCenter_ = false;
    };

    if (behaviorOnReleaseBeforeDelay_.value() == int(BehaviorBeforeDelay::Direction) && resetCursorPositionToCenter_)
        resetCursorPositionToCenter();

    const mstime currentTime = TimeInMilliseconds();

    if (isVisible_)
    {
        if (currentTime >= currentTriggerTime_ + displayDelayOption_.value())
        {
            if (resetCursorPositionToCenter_)
                resetCursorPositionToCenter();
        }
    }
}

void Wheel::OnMapChange(u32 prevId, u32 newId)
{
    ResetConditionallyDelayed(false);
}

void Wheel::OnCharacterChange(const std::wstring& prevCharacterName, const std::wstring& newCharacterName)
{
    ResetConditionallyDelayed(false);
}

void Wheel::OnFocusLost()
{
    currentHovered_     = nullptr;
    isVisible_          = false;
    currentTriggerTime_ = 0;

    conditionalDelay_   = {};
}

bool Wheel::CanActivate(const WheelElement* we) const
{
    const auto& mumble = MumbleLink::i();
    if (!mumble.gameHasFocus() || mumble.isMapOpen())
        return false;

    auto cs = mumble.currentState();

    return we->isUsable(cs);
}

std::generator<float> Wheel::GetHoveredFadeValues(mstime currentTime)
{
    for (const auto* e : GetRenderableElements())
    {
        co_yield e->hoverFadeIn(currentTime, *this);
    }

    switch (CenterBehavior(centerBehaviorOption_.value()))
    {
        case CenterBehavior::Previous:
            if (previousUsed_)
                co_yield previousUsed_->hoverFadeIn(currentTime, *this);
            break;
        case CenterBehavior::Favorite:
        {
            if (auto* fav = GetFavorite(centerFavoriteOption_.value()))
            {
                co_yield fav->hoverFadeIn(currentTime, *this);
                break;
            }
        }
            [[fallthrough]];
        default:
            break;
    }
}

void Wheel::Sort()
{
    sortedWheelElements_.resize(wheelElements_.size());
    std::ranges::transform(wheelElements_, sortedWheelElements_.begin(), [](auto& p) { return p.get(); });

    std::ranges::sort(sortedWheelElements_, [](const WheelElement* a, const WheelElement* b) { return a->sortingPriority() < b->sortingPriority(); });
    minElementSortingPriority_ = sortedWheelElements_.front()->sortingPriority();
}

WheelElement* Wheel::GetCenterHoveredElement()
{
    if (triggerOption_.value() == TriggerBehavior::Hover)
        return nullptr;

    if (centerCancelDelayedInputOption_.value() && OptHasValue(conditionalDelay_.element))
        return nullptr;

    switch (CenterBehavior(centerBehaviorOption_.value()))
    {
        case CenterBehavior::Previous:
            return previousUsed_;
        case CenterBehavior::Favorite:
            return GetFavorite(centerFavoriteOption_.value());
        default:
            return nullptr;
    }
}

WheelElement* Wheel::GetFavorite(Favorite fav) const
{
    ConditionalState cs = MumbleLink::i().currentState();

    int              favoriteId;
    if (NotNone(cs & ConditionalState::InWvW) && fav.bits.inWvW != -1)
        favoriteId = fav.bits.inWvW;
    else if (NotNone(cs & ConditionalState::InCombat) && fav.bits.inCombat != -1)
        favoriteId = fav.bits.inCombat;
    else if (NotNone(cs & ConditionalState::OnWater) && fav.bits.onWater != -1)
        favoriteId = fav.bits.onWater;
    else if (NotNone(cs & ConditionalState::Underwater) && fav.bits.underwater != -1)
        favoriteId = fav.bits.underwater;
    else
        favoriteId = fav.bits.baseline;

    if (favoriteId < 0 || favoriteId >= int(wheelElements_.size()))
        return nullptr;

    auto* elem = wheelElements_[favoriteId].get();

    return elem->isBound() ? elem : nullptr;
}

std::generator<WheelElement*> Wheel::GetVisibleElements(ConditionalState cs, bool sorted) const
{
    if (sorted)
    {
        for (auto& we : sortedWheelElements_)
            if (we->isVisible(cs))
                co_yield we;
    }
    else
    {
        for (auto& we : wheelElements_)
            if (we->isVisible(cs))
                co_yield we.get();
    }
}

std::generator<WheelElement*> Wheel::GetUsableElements(ConditionalState cs, bool sorted) const
{
    std::vector<WheelElement*> elems;
    if (sorted)
    {
        for (auto& we : sortedWheelElements_)
            if (we->isUsable(cs))
                co_yield we;
    }
    else
    {
        for (auto& we : wheelElements_)
            if (we->isUsable(cs))
                co_yield we.get();
    }
}

bool Wheel::HasVisibleOrUsableElements(ConditionalState cs) const
{
    for (auto& we : wheelElements_)
        if (we->isUsable(cs) || we->isVisible(cs))
            return true;

    return false;
}

void Wheel::OnMouseMove(bool& rv)
{
    if (isVisible_)
    {
        UpdateHover();

        // If holding down the button is not necessary, modify behavior
        if (triggerOption_.value() == TriggerBehavior::Hover && isVisible_ && currentHovered_ != nullptr)
            DeactivateWheel();
    }

    rv |= isVisible_ && lockCameraWhenOverlayedOption_.value();
}

void Wheel::OnMouseButton(ScanCode sc, bool down, bool& rv)
{
    if (triggerOption_.value() == TriggerBehavior::Click && isVisible_)
    {
        const bool previousVisibility = isVisible_;
        isVisible_                    = isVisible_ && sc != ScanCode::LButton;
        if (!isVisible_ && previousVisibility)
        {
            rv = true;
            DeactivateWheel();
        }
    }
}

PassToGame Wheel::KeybindEvent(bool center, Activated activated)
{
    const bool previousVisibility = isVisible_;

    if (MumbleLink::i().isMapOpen())
        isVisible_ = false;
    else
    {
        WheelElement* bypassElement = nullptr;
        Keybind*      bypassKeybind = nullptr;
        if (activated == Activated::Yes && !waitingForBypassComplete_ && (BypassCheck(bypassElement, bypassKeybind) || ShouldSkip(bypassElement)))
        {
            previousUsed_             = bypassElement;
            isVisible_                = false;
            waitingForBypassComplete_ = true;
            if (!bypassKeybind)
                SendKeybindOrDelay(bypassElement, std::nullopt);
            else
            {
                Log::i().Print(Severity::Debug, "Sending bypass keybind.");

                Input::i().KeyUpActive();
                Input::i().SendKeybind(bypassKeybind->keyCombo(), std::nullopt);
            }
        }

        if (waitingForBypassComplete_)
        {
            if (activated == Activated::No)
                waitingForBypassComplete_ = false;
        }
        else
        {
            isVisible_ = activated == Activated::Yes;

            // If holding down the button is not necessary, modify behavior
            if (previousVisibility && (triggerOption_.value() == TriggerBehavior::Click || triggerOption_.value() == TriggerBehavior::Hover && currentHovered_ == nullptr))
                isVisible_ = true;

            if (isVisible_ && !previousVisibility)
                ActivateWheel(center);
        }
    }

    if (!isVisible_ && previousVisibility)
        DeactivateWheel();

    return isVisible_ != previousVisibility ? PassToGame::Prevent : PassToGame::Allow;
}

void Wheel::ActivateWheel(bool isMountOverlayLocked)
{
    auto& io             = ImGui::GetIO();

    cursorResetPosition_ = { static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y) };
    LogDebug("Storing cursor position ({}, {}) for restore...", cursorResetPosition_.x, cursorResetPosition_.y);

    if (!HasVisibleOrUsableElements(MumbleLink::i().currentState()))
    {
        LogWarn("Triggered menu '{}', but no element is visible or usable!", displayName_);
        bool isAnyBound = false;
        for (auto& we : wheelElements_)
            if (we->isBound())
            {
                isAnyBound = true;
                break;
            }

        if (!isAnyBound)
            showEmptyPopup_ = true;
    }

    // Mount overlay is turned on
    if (isMountOverlayLocked)
    {
        currentPosition_.x = currentPosition_.y = 0.5f;

        if (resetCursorOnLockedKeybindOption_.value())
            resetCursorPositionToCenter_ = true;
    }
    else
    {
        resetCursorPositionToCenter_ = false;
        currentPosition_.x           = io.MousePos.x / float(Core::i().screenWidth());
        currentPosition_.y           = io.MousePos.y / float(Core::i().screenHeight());
    }

    currentTriggerTime_ = TimeInMilliseconds();

    wipeMaskData_       = { frand() * 0.20f + 0.40f, frand() * 0.20f + 0.40f, frand() * 2 * float(M_PI) };

    UpdateHover();
}

void Wheel::PassToGame()
{
    bool centerKeybind = currentPosition_.x == 0.5f && currentPosition_.y == 0.5f;
    Input::i().SendKeybind((centerKeybind ? centralKeybind_ : keybind_).keyCombo());
    currentHovered_ = nullptr;
}

void Wheel::Draw()
{
    if (opacityMultiplier() == 0.f)
        return;

    const mstime currentTime = TimeInMilliseconds();

    if (isVisible() && currentTime >= currentDisplayTime())
    {}
}

void Wheel::DeactivateWheel()
{
    isVisible_                   = false;
    resetCursorPositionToCenter_ = false;

    if (currentHovered_ == nullptr && OptHasValue(conditionalDelay_.element) && centerCancelDelayedInputOption_.value())
    {
        ResetConditionallyDelayed(true);
        return;
    }

    // If keybind release was done before the wheel is visible, check our behavior
    if (currentTriggerTime_ + displayDelayOption_.value() > TimeInMilliseconds())
    {
        if (!SpecialBehaviorBeforeDelay())
        {
            switch (BehaviorBeforeDelay(behaviorOnReleaseBeforeDelay_.value()))
            {
                default:
                    currentHovered_ = nullptr;
                    break;
                case BehaviorBeforeDelay::Favorite:
                    currentHovered_ = GetFavorite(delayFavoriteOption_.value());
                    break;
                case BehaviorBeforeDelay::Previous:
                    currentHovered_ = previousUsed_;
                    break;
                case BehaviorBeforeDelay::Direction:
                    if (!currentHovered_)
                        currentHovered_ = GetCenterHoveredElement();
                    break;
                case BehaviorBeforeDelay::PassToGame:
                    PassToGame();
                    break;
            }
        }
    }
    else if (!currentHovered_)
    {
        if (CenterBehavior(centerBehaviorOption_.value()) == CenterBehavior::PassToGame)
            PassToGame();
        else
            currentHovered_ = GetCenterHoveredElement();
    }

    bool resetMouse = alwaysResetCursorPositionBeforeKeyPress_ || resetCursorAfterKeybindOption_.value() || ResetMouseCheck(currentHovered_);

    // Mount overlay is turned off, send the keybind
    if (currentHovered_ != nullptr)
    {
        SendKeybindOrDelay(currentHovered_, resetMouse ? std::make_optional(cursorResetPosition_) : std::nullopt);
        previousUsed_   = currentHovered_;
        currentHovered_ = nullptr;
    }
    else
        SendKeybindOrDelay({}, resetMouse ? std::make_optional(cursorResetPosition_) : std::nullopt);
}

void Wheel::SendKeybindOrDelay(OptKeybindWheelElement kbwe, std::optional<Point> mousePos)
{
    if (!OptHasValue(kbwe))
    {
        if (mousePos)
        {
            Log::i().Print(Severity::Debug, "Moving cursor to position ({}, {}).", mousePos->x, mousePos->y);
            Input::i().SendKeybind({}, mousePos);
        }
        return;
    }

    auto cs                = MumbleLink::i().currentState();

    // We're not checking WvW here; no reason to enqueue an action that would require a map change to execute
    bool shouldAlwaysDelay = CustomDelayCheck(kbwe);
    bool shouldDelay       = shouldAlwaysDelay || (enableQueuingOption_.value() && std::holds_alternative<WheelElement*>(kbwe) && !std::get<WheelElement*>(kbwe)->isUsable(cs));

    if (mousePos)
        Log::i().Print(Severity::Debug, "Moving cursor to position ({}, {}) and queuing keybind.", mousePos->x, mousePos->y);
    else
        Log::i().Print(Severity::Debug, "Queuing keybind.");
    Input::i().SendKeybind({}, mousePos);

    auto& cd      = conditionalDelay_;
    cd.element    = kbwe;
    cd.time       = TimeInMilliseconds();
    cd.testPasses = cd.immediate = cd.hidden = !shouldDelay;
}

void Wheel::ResetConditionallyDelayed(bool withFadeOut, mstime currentTime)
{
    if (withFadeOut && std::holds_alternative<WheelElement*>(conditionalDelay_.element))
        conditionalDelayDisplay_ = std::get<WheelElement*>(conditionalDelay_.element);
    else
        conditionalDelayDisplay_ = nullptr;

    conditionalDelay_      = {};
    conditionalDelay_.time = currentTime - maximumConditionalWaitTimeOption_.value() * 1000ull;
    if (!withFadeOut)
        conditionalDelay_.time -= ConditionalDelay::FadeOutTime;
}
} // namespace GW2Radial

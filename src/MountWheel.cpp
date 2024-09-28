#include <MountWheel.h>
#include <MumbleLink.h>
#include <Wheel.h>

namespace GW2Radial
{
MountWheel::MountWheel(std::shared_ptr<Texture2D> bgTexture)
    : Wheel(std::move(bgTexture), "mounts", "Mounts")
    , dismountDelayOption_("Dismount delay", "dismount_delay", "wheel_" + nickname_, 0)
    , quickDismountOption_("Quick dismount", "quick_dismount", "wheel_" + nickname_, true)
    , dismountKeybind_("dismount", "Dismount", "wheel_" + nickname_)
    , showCancelOption_("Show cancel option", "cancel_option", "wheel_" + nickname_, false)
    , showForceOption_("Show force option", "force_option", "wheel_" + nickname_, false)
    , beforeDelayForceOption_("Override before delay choice with force option", "default_force_option", "wheel_" + nickname_, false)
{
    clearConditionalDelayOnSend_ = false;

    IterateEnum<MountType>(
        [&](auto i)
        {
            AddElement(std::make_unique<WheelElement>(static_cast<u32>(i), std::string("mount_") + GetMountNicknameFromType(i), "Mounts", GetMountNameFromType(i),
                                                      GetMountColorFromType(i), GetMountPropsFromType(i)));
        });

    auto cancel =
        std::make_unique<WheelElement>(ToUnderlying(MountSpecial::Cancel), "mount_special_cancel", "Mounts", "Cancel queue", glm::vec4(0.8f), ConditionalProperties::None);
    cancel->customBehavior([&](bool visibility) { return enableQueuingOption_.value() && showCancelOption_.value() && OptHasValue(conditionalDelay_.element); });
    AddElement(std::move(cancel));

    auto force = std::make_unique<WheelElement>(ToUnderlying(MountSpecial::Force), "mount_special_force", "Mounts", "Force mount", glm::vec4(0.8f), ConditionalProperties::None);
    force->customBehavior(
        [&](bool visibility)
        {
            if (enableQueuingOption_.value() && showForceOption_.value() && std::holds_alternative<WheelElement*>(conditionalDelay_.element))
            {
                const auto* element = std::get<WheelElement*>(conditionalDelay_.element);
                return element == wheelElements_[MountIndex(MountType::Skyscale)].get() || element == wheelElements_[MountIndex(MountType::Warclaw)].get();
            }
            else
                return false;
        });
    force_ = force.get();
    AddElement(std::move(force));
}

void MountWheel::OnUpdate()
{
    Wheel::OnUpdate();

    // If we mounted up while we had a mount queued, we don't want to try mounting again, that'd dismount us instead!
    if (conditionalDelay_.element.index() != 0 && MumbleLink::i().currentMount() != MumbleLink::MountType::None)
        ResetConditionallyDelayed(true);
}

glm::vec4 MountWheel::GetMountColorFromType(MountType m)
{
    switch (m)
    {
        case MountType::Raptor:
            return { 213 / 255.f, 100 / 255.f, 89 / 255.f, 1 };
        case MountType::Springer:
            return { 212 / 255.f, 198 / 255.f, 94 / 255.f, 1 };
        case MountType::Skimmer:
            return { 108 / 255.f, 128 / 255.f, 213 / 255.f, 1 };
        case MountType::Jackal:
            return { 120 / 255.f, 183 / 255.f, 197 / 255.f, 1 };
        case MountType::Griffon:
            return { 136 / 255.f, 123 / 255.f, 195 / 255.f, 1 };
        case MountType::Beetle:
            return { 199 / 255.f, 131 / 255.f, 68 / 255.f, 1 };
        case MountType::Warclaw:
            return { 181 / 255.f, 255 / 255.f, 244 / 255.f, 1 };
        case MountType::Skyscale:
            return { 211 / 255.f, 142 / 255.f, 244 / 255.f, 1 };
        case MountType::Turtle:
            return { 56 / 255.f, 228 / 255.f, 85 / 255.f, 1 };
        case MountType::Skiff:
            return { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1 };
        default:
            return { 1, 1, 1, 1 };
    }
}

void MountWheel::MenuSectionKeybinds(Keybind** currentlyModifiedKeybind)
{
    ImGui::KeybindInput(dismountKeybind_, currentlyModifiedKeybind, "Mount/Dismount keybind in game. If set, used to dismount instead of sending an arbitrary mount key.");
}

void MountWheel::MenuSectionInteraction()
{
    ImGui::ConfigurationWrapper(&ImGui::Checkbox, quickDismountOption_);
    UI::HelpTooltip("If enabled, using any keybind while mounted will directly send a mount keybind to dismount without showing the radial menu.");
    {
        UI::Scoped::Disable disable(!quickDismountOption_.value());
        ImGui::ConfigurationWrapper(&ImGui::SliderInt, dismountDelayOption_, 0, 3000, "%d ms", ImGuiSliderFlags_AlwaysClamp);
        UI::HelpTooltip("Amount of time, in milliseconds, to wait between pressing the keybind and dismounting.");
    }

    if (enableQueuingOption_.value())
    {
        ImGui::ConfigurationWrapper(&ImGui::Checkbox, showCancelOption_);
        UI::HelpTooltip("If enabled, will display an extra slice in the mount wheel when a mount has been queued. Selecting this option will cancel the queue.");

        ImGui::ConfigurationWrapper(&ImGui::Checkbox, showForceOption_);
        UI::HelpTooltip(
            "If enabled, will display an extra slice in the mount wheel when a mount has been queued. Selecting this option will force the queued mount key to be sent and "
            "clear the queue. Only available for Skyscale and Warclaw.");

        ImGui::ConfigurationWrapper(&ImGui::Checkbox, beforeDelayForceOption_);
        UI::HelpTooltip("If enabled, the behavior when released before delay has lapsed will be overridden to the force mount option. Only available for Skyscale and Warclaw.");
    }
}

bool MountWheel::BypassCheck(WheelElement*& we, Keybind*& kb)
{
    const auto& mumble = MumbleLink::i();
    if (quickDismountOption_.value() && mumble.isMounted())
    {
        if (previousUsed_ != nullptr)
            we = previousUsed_;
        else
        {
            const auto& activeElems = GetUsableElements(mumble.currentState());
            if (!activeElems.empty())
                we = activeElems.front();
        }

        if (dismountKeybind_.isSet())
            kb = &dismountKeybind_;

        if (we || kb)
        {
            if (dismountDelayOption_.value() > 0)
                dismountTriggerTime_ = TimeInMilliseconds() + dismountDelayOption_.value();
            else
                dismountTriggerTime_ = 0;
            return true;
        }

        return false;
    }

    return false;
}

bool MountWheel::CustomDelayCheck(OptKeybindWheelElement&)
{
    if (TimeInMilliseconds() < dismountTriggerTime_)
    {
        conditionalDelay_.hidden    = true;
        conditionalDelay_.immediate = true;

        return true;
    }

    return false;
}

bool MountWheel::ResetMouseCheck(WheelElement* we)
{
    return we == wheelElements_[MountIndex(MountType::Skiff)].get();
}

Keybind* MountWheel::GetKeybindFromOpt(OptKeybindWheelElement& o)
{
    if (std::holds_alternative<WheelElement*>(o))
    {
        auto* we = std::get<WheelElement*>(o);
        if (we == wheelElements_[MountIndex(MountSpecial::Cancel)].get())
        {
            ResetConditionallyDelayed(true);
            return nullptr;
        }
        else if (we == wheelElements_[MountIndex(MountSpecial::Force)].get())
        {
            auto delayedElement = conditionalDelay_.element;
            return GetKeybindFromOpt(delayedElement);
        }
    }

    return Wheel::GetKeybindFromOpt(o);
}

bool MountWheel::SpecialBehaviorBeforeDelay()
{
    if (!beforeDelayForceOption_.value())
        return false;

    if (MumbleLink::i().isInWvW())
        return false;

    if (!std::holds_alternative<WheelElement*>(conditionalDelay_.element))
        return false;

    const auto* element = std::get<WheelElement*>(conditionalDelay_.element);
    if (element != wheelElements_[MountIndex(MountType::Skyscale)].get() && element != wheelElements_[MountIndex(MountType::Warclaw)].get())
        return false;

    currentHovered_ = force_;
    return true;
}

} // namespace GW2Radial

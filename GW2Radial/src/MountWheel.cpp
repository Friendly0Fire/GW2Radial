#include <MountWheel.h>
#include <Wheel.h>
#include <MumbleLink.h>

namespace GW2Radial
{
MountWheel::MountWheel(std::shared_ptr<Texture2D> bgTexture)
    : Wheel(std::move(bgTexture), "mounts", "Mounts")
{
    struct MountExtraData : ExtraData
    {
        ConfigurationOption<bool> enableUnderwaterSkimmer;
        ConfigurationOption<int>  dismountDelayOption;
        ConfigurationOption<bool> quickDismountOption;
        mstime                    dismountTriggerTime;

        MountExtraData(
            ConfigurationOption<bool> &&eus,
            ConfigurationOption<int> && ddo,
            ConfigurationOption<bool> &&qdo)
            : enableUnderwaterSkimmer(std::move(eus))
            , dismountDelayOption(std::move(ddo))
            , quickDismountOption(std::move(qdo))
            , dismountTriggerTime(0)
        {}
    };

    extraData_ = std::make_shared<MountExtraData>(
        ConfigurationOption(
            "Enable underwater Skimmer", "underwater_skimmer",
            "wheel_" + nickname_, false),
        ConfigurationOption(
            "Dismount delay", "dismount_delay",
            "wheel_" + nickname_, 0),
        ConfigurationOption(
            "Quick dismount", "quick_dismount",
            "wheel_" + nickname_, true)
        );

    extraUI_.emplace();
    extraUI_->interaction = [this]()
    {
        auto extraData = static_cast<MountExtraData *>(extraData_.get());

        ImGuiConfigurationWrapper(&ImGui::Checkbox, extraData->quickDismountOption);
        ImGuiHelpTooltip(
            "If enabled, using any keybind while mounted will directly send a mount keybind to dismount without showing the radial menu.");
        {
            ImGuiDisabler disable(!extraData->quickDismountOption.value());
            ImGuiConfigurationWrapper(
                &ImGui::SliderInt, extraData->dismountDelayOption, 0, 3000, "%d ms",
                ImGuiSliderFlags_AlwaysClamp);
            ImGuiHelpTooltip("Amount of time, in milliseconds, to wait between pressing the keybind and dismounting.");
        }
    };

    custom_.test = [this]()
    {
        return TimeInMilliseconds() < static_cast<MountExtraData *>(extraData_.get())->dismountTriggerTime;
    };

    doBypassWheel_ = [this](WheelElement *&we)
    {
        auto        data   = static_cast<MountExtraData *>(extraData_.get());
        const auto &mumble = MumbleLink::i();
        if (data->quickDismountOption.value() && mumble.isMounted())
        {
            if (previousUsed_ != nullptr)
                we = previousUsed_;
            else
            {
                const auto &activeElems = GetActiveElements(ConditionalState::NONE);
                if (!activeElems.empty())
                    we = activeElems.front();
            }

            if (we)
            {
                if (data->dismountDelayOption.value() > 0)
                    data->dismountTriggerTime = TimeInMilliseconds() + data->dismountDelayOption.value();
                else
                    data->dismountTriggerTime = 0;
                return true;
            }

            return false;
        }

        return false;
    };

    iterateEnum<MountType>(
        [&](auto i)
        {
            AddElement(
                std::make_unique<WheelElement>(
                    static_cast<uint>(i),
                    std::string("mount_") +
                    GetMountNicknameFromType(i), "Mounts",
                    GetMountNameFromType(i),
                    GetMountColorFromType(i)));
        });
}

glm::vec4 MountWheel::GetMountColorFromType(MountType m)
{
    switch (m)
    {
        case MountType::RAPTOR:
            return { 213 / 255.f, 100 / 255.f, 89 / 255.f, 1 };
        case MountType::SPRINGER:
            return { 212 / 255.f, 198 / 255.f, 94 / 255.f, 1 };
        case MountType::SKIMMER:
            return { 108 / 255.f, 128 / 255.f, 213 / 255.f, 1 };
        case MountType::JACKAL:
            return { 120 / 255.f, 183 / 255.f, 197 / 255.f, 1 };
        case MountType::GRIFFON:
            return { 136 / 255.f, 123 / 255.f, 195 / 255.f, 1 };
        case MountType::BEETLE:
            return { 199 / 255.f, 131 / 255.f, 68 / 255.f, 1 };
        case MountType::WARCLAW:
            return { 181 / 255.f, 255 / 255.f, 244 / 255.f, 1 };
        case MountType::SKYSCALE:
            return { 211 / 255.f, 142 / 255.f, 244 / 255.f, 1 };
        case MountType::SKIFF:
            return { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1 };
        case MountType::TURTLE:
            return { 56 / 255.f, 228 / 255.f, 85 / 255.f, 1 };
        default:
            return { 1, 1, 1, 1 };
    }
}
}

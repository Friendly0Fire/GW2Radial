#pragma once
#include <Main.h>
#include <Wheel.h>

namespace GW2Radial
{

class MountWheel : public Wheel
{
public:
    MountWheel(std::shared_ptr<Texture2D> bgTexture);

protected:
    static const char* GetMountNameFromType(MountType m)
    {
        switch (m)
        {
            case MountType::RAPTOR:
                return "Raptor";
            case MountType::SPRINGER:
                return "Springer";
            case MountType::SKIMMER:
                return "Skimmer";
            case MountType::JACKAL:
                return "Jackal";
            case MountType::GRIFFON:
                return "Griffon";
            case MountType::BEETLE:
                return "Roller Beetle";
            case MountType::WARCLAW:
                return "Warclaw";
            case MountType::SKYSCALE:
                return "Skyscale";
            case MountType::TURTLE:
                return "Turtle";
            case MountType::SKIFF:
                return "Skiff";
            default:
                return "[Unknown]";
        }
    }
    static const char* GetMountNicknameFromType(MountType m)
    {
        switch (m)
        {
            case MountType::RAPTOR:
                return "raptor";
            case MountType::SPRINGER:
                return "springer";
            case MountType::SKIMMER:
                return "skimmer";
            case MountType::JACKAL:
                return "jackal";
            case MountType::GRIFFON:
                return "griffon";
            case MountType::BEETLE:
                return "beetle";
            case MountType::WARCLAW:
                return "warclaw";
            case MountType::SKYSCALE:
                return "skyscale";
            case MountType::TURTLE:
                return "turtle";
            case MountType::SKIFF:
                return "skiff";
            default:
                return "unknown";
        }
    }

    static ConditionalProperties GetMountPropsFromType(MountType m)
    {
        // Work under the assumption that queuing is enabled and players usually want an underwater mount when underwater,
        // which means all mounts should only be visible in combat, but NOT under/on water
        ConditionalProperties baseline = ConditionalProperties::VISIBLE_IN_COMBAT;

        switch (m)
        {
            case MountType::RAPTOR:
            case MountType::SPRINGER:
            case MountType::JACKAL:
            case MountType::GRIFFON:
            case MountType::BEETLE:
            case MountType::SKYSCALE:
                return baseline;

            case MountType::SKIMMER:
                // Skimmer is usable on water and, for most, underwater, so default to that
                return baseline | ConditionalProperties::USABLE_ON_WATER | ConditionalProperties::USABLE_UNDERWATER | ConditionalProperties::VISIBLE_ON_WATER |
                       ConditionalProperties::VISIBLE_UNDERWATER;

            case MountType::WARCLAW:
                // Warclaw is usable and visible in WvW
                return baseline | ConditionalProperties::USABLE_WVW | ConditionalProperties::VISIBLE_WVW;

            case MountType::TURTLE:
                // Turtle is usable on and underwater
                return baseline | ConditionalProperties::USABLE_ON_WATER | ConditionalProperties::USABLE_UNDERWATER | ConditionalProperties::VISIBLE_ON_WATER |
                       ConditionalProperties::VISIBLE_UNDERWATER;

            case MountType::SKIFF:
                // Skiff is usable on water
                return baseline | ConditionalProperties::USABLE_ON_WATER | ConditionalProperties::VISIBLE_ON_WATER;

            default:
                return ConditionalProperties::NONE;
        }
    }

    static glm::vec4          GetMountColorFromType(MountType m);

    void                      MenuSectionInteraction() override;
    bool                      BypassCheck(WheelElement*&) override;
    bool                      CustomDelayCheck(WheelElement*) override;
    bool                      ResetMouseCheck(WheelElement*) override;

    ConfigurationOption<int>  dismountDelayOption_;
    ConfigurationOption<bool> quickDismountOption_;
    mstime                    dismountTriggerTime_ = 0;
};

} // namespace GW2Radial

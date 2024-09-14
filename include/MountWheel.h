#pragma once
#include <Main.h>
#include <Wheel.h>

namespace GW2Radial
{

class MountWheel : public Wheel
{
public:
    MountWheel(std::shared_ptr<Texture2D> bgTexture);

    void OnUpdate() override;

protected:
    static const char* GetMountNameFromType(MountType m)
    {
        switch (m)
        {
            case MountType::Raptor:
                return "Raptor";
            case MountType::Springer:
                return "Springer";
            case MountType::Skimmer:
                return "Skimmer";
            case MountType::Jackal:
                return "Jackal";
            case MountType::Griffon:
                return "Griffon";
            case MountType::Beetle:
                return "Roller Beetle";
            case MountType::Warclaw:
                return "Warclaw";
            case MountType::Skyscale:
                return "Skyscale";
            case MountType::Turtle:
                return "Turtle";
            case MountType::Skiff:
                return "Skiff";
            default:
                return "[Unknown]";
        }
    }
    static const char* GetMountNicknameFromType(MountType m)
    {
        switch (m)
        {
            case MountType::Raptor:
                return "raptor";
            case MountType::Springer:
                return "springer";
            case MountType::Skimmer:
                return "skimmer";
            case MountType::Jackal:
                return "jackal";
            case MountType::Griffon:
                return "griffon";
            case MountType::Beetle:
                return "beetle";
            case MountType::Warclaw:
                return "warclaw";
            case MountType::Skyscale:
                return "skyscale";
            case MountType::Turtle:
                return "turtle";
            case MountType::Skiff:
                return "skiff";
            default:
                return "unknown";
        }
    }

    static ConditionalProperties GetMountPropsFromType(MountType m)
    {
        // Work under the assumption that queuing is enabled and players usually want an underwater mount when underwater,
        // which means all mounts should only be visible in combat, but NOT under/on water
        ConditionalProperties baseline = ConditionalProperties::UsableDefault | ConditionalProperties::VisibleDefault | ConditionalProperties::VisibleInCombat;

        switch (m)
        {
            case MountType::Raptor:
            case MountType::Springer:
            case MountType::Jackal:
            case MountType::Griffon:
            case MountType::Beetle:
            case MountType::Skyscale:
                return baseline;

            case MountType::Skimmer:
                // Skimmer is usable on water and, for most, underwater, so default to that
                return baseline | ConditionalProperties::UsableOnWater | ConditionalProperties::UsableUnderwater | ConditionalProperties::VisibleOnWater |
                       ConditionalProperties::VisibleUnderwater;

            case MountType::Warclaw:
                // Warclaw is usable in WvW
                return baseline | ConditionalProperties::UsableWvW;

            case MountType::Turtle:
                // Turtle is usable on and underwater
                return baseline | ConditionalProperties::UsableOnWater | ConditionalProperties::UsableUnderwater | ConditionalProperties::VisibleOnWater |
                       ConditionalProperties::VisibleUnderwater;

            case MountType::Skiff:
                // Skiff is usable on water
                return baseline | ConditionalProperties::UsableOnWater | ConditionalProperties::VisibleOnWater;

            default:
                return ConditionalProperties::None;
        }
    }

    static glm::vec4          GetMountColorFromType(MountType m);

    void                      MenuSectionKeybinds(Keybind**) override;
    void                      MenuSectionInteraction() override;
    bool                      BypassCheck(WheelElement*&, Keybind*&) override;
    bool                      CustomDelayCheck(OptKeybindWheelElement&) override;
    bool                      ResetMouseCheck(WheelElement*) override;
    Keybind*                  GetKeybindFromOpt(OptKeybindWheelElement& o) override;
    bool                      SpecialBehaviorBeforeDelay() override;

    ConfigurationOption<int>  dismountDelayOption_;
    ConfigurationOption<bool> quickDismountOption_;
    ConfigurationOption<bool> showCancelOption_;
    ConfigurationOption<bool> showForceOption_;
    ConfigurationOption<bool> beforeDelayForceOption_;
    mstime                    dismountTriggerTime_ = 0;
    Keybind                   dismountKeybind_;
    WheelElement*             force_;
};

} // namespace GW2Radial

#include <Mount.h>
#include <Wheel.h>
#include <MumbleLink.h>

namespace GW2Radial
{

Mount::Mount(MountType m, ID3D11Device* dev)
	: WheelElement(uint(m), std::string("mount_") + GetMountNicknameFromType(m), "Mounts", GetMountNameFromType(m), dev)
{ }

bool Mount::isActive() const
{
	if(!WheelElement::isActive())
		return false;

	if(elementId_ != uint(MountType::WARCLAW))
		return !MumbleLink::i().isInWvW();
	
	return true;
}

template<>
void Wheel::Setup<Mount>(ID3D11Device* dev)
{
	struct MountExtraData : Wheel::ExtraData {
	    ConfigurationOption<bool> enableUnderwaterSkimmer;
		ConfigurationOption<int> dismountDelayOption;
		ConfigurationOption<bool> quickDismountOption;
		ConfigurationOption<bool> quickWvWOption;
		mstime dismountTriggerTime;
		MountExtraData(ConfigurationOption<bool>&& eus,
			ConfigurationOption<int>&& ddo,
			ConfigurationOption<bool>&& qdo,
			ConfigurationOption<bool>&& qwvwo)
			: enableUnderwaterSkimmer(std::move(eus)),
			dismountDelayOption(std::move(ddo)),
			quickDismountOption(std::move(qdo)),
			quickWvWOption(std::move(qwvwo)),
			dismountTriggerTime(0) {}
	};

	extraData_ = std::make_shared<MountExtraData>(
		ConfigurationOption<bool>("Enable underwater Skimmer", "underwater_skimmer", "wheel_" + nickname_, false),
		ConfigurationOption<int>("Dismount delay", "dismount_delay", "wheel_" + nickname_, 0),
		ConfigurationOption<bool>("Quick dismount", "quick_dismount", "wheel_" + nickname_, true),
		ConfigurationOption<bool>("Quick Warclaw", "quick_wvw", "wheel_" + nickname_, true)
		);

	extraUI_.emplace();
	extraUI_->interaction = [this]() {
		MountExtraData* extraData = static_cast<MountExtraData*>(extraData_.get());
	    if(ImGuiConfigurationWrapper(&ImGui::Checkbox, extraData->enableUnderwaterSkimmer))
			aboveWater_.enabled = !extraData->enableUnderwaterSkimmer.value();
		ImGuiHelpTooltip("This enables Skimmer auto-mounting to work underwater (in addition to on the water surface) in conjunction with the Skimming the Depths mastery.");

		ImGuiConfigurationWrapper(&ImGui::Checkbox, extraData->quickWvWOption);
		ImGuiHelpTooltip("If enabled, using any keybind in WvW will mount the Warclaw without showing the radial menu.");

		ImGuiConfigurationWrapper(&ImGui::Checkbox, extraData->quickDismountOption);
		ImGuiHelpTooltip("If enabled, using any keybind while mounted will directly send a mount keybind to dismount without showing the radial menu.");
		{
			ImGuiDisabler disable(!extraData->quickDismountOption.value());
			ImGuiConfigurationWrapper(&ImGui::SliderInt, extraData->dismountDelayOption, 0, 3000, "%d ms", ImGuiSliderFlags_AlwaysClamp);
			ImGuiHelpTooltip("Amount of time, in milliseconds, to wait between pressing the keybind and dismounting.");
		}
	};

	aboveWater_.enabled = !static_cast<MountExtraData*>(extraData_.get())->enableUnderwaterSkimmer.value();
	aboveWater_.canToggleOff = true;
	outOfCombat_.enabled = true;
	outOfCombat_.canToggleOff = true;
	custom_.test = [this]()
	{
		return TimeInMilliseconds() < static_cast<MountExtraData*>(extraData_.get())->dismountTriggerTime;
	};
	custom_.enabled = true;

	doBypassWheel_ = [this](WheelElement*& we) {
		MountExtraData* data = static_cast<MountExtraData*>(extraData_.get());
		cref mumble = MumbleLink::i();
		if(data->quickDismountOption.value() && mumble.isMounted()) {
			if (previousUsed_ != nullptr)
				we = previousUsed_;
			else {
				const auto& activeElems = GetActiveElements();
				if (!activeElems.empty())
					we = activeElems.front();
			}

			if (we) {
				if (data->dismountDelayOption.value() > 0)
					data->dismountTriggerTime = TimeInMilliseconds() + data->dismountDelayOption.value();
				else
					data->dismountTriggerTime = 0;
				return true;
			}

			return false;
		}

		if(data->quickWvWOption.value() && mumble.isInWvW()) {
		    we = wheelElements_[uint(MountType::WARCLAW) - uint(MountType::FIRST)].get();
			return we != nullptr && we->isBound();
		}

		if(!mumble.isInWvW() && (mumble.isSwimmingOnSurface() || mumble.isUnderwater() && data->enableUnderwaterSkimmer.value())) {
		    we = wheelElements_[uint(MountType::SKIMMER) - uint(MountType::FIRST)].get();
			return we != nullptr && we->isBound();
		}

		return false;
	};

	for(auto i = MountType::FIRST; i <= MountType::LAST; i = MountType(uint(i) + 1))
		AddElement(std::make_unique<Mount>(i, dev));
}

fVector4 Mount::color()
{
	auto mt = MountType(elementId_);
	switch(mt)
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
	default:
		return { 1, 1, 1, 1 };
	}
}

}
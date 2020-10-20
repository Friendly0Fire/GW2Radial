#include <Mount.h>
#include <Wheel.h>
#include <MumbleLink.h>

namespace GW2Radial
{

Mount::Mount(MountType m, IDirect3DDevice9* dev)
	: WheelElement(uint(m), std::string("mount_") + GetMountNicknameFromType(m), "Mounts", GetMountNameFromType(m), dev)
{ }

bool Mount::isActive() const
{
	if(!WheelElement::isActive())
		return false;

	if(elementId_ != uint(MountType::WARCLAW))
		return !MumbleLink::i()->isInWvW();
	
	return true;
}

template<>
void Wheel::Setup<Mount>(IDirect3DDevice9* dev)
{
	struct MountExtraData : Wheel::ExtraData {
	    ConfigurationOption<bool> enableUnderwaterSkimmer;
		MountExtraData(ConfigurationOption<bool>&& eus) : enableUnderwaterSkimmer(std::move(eus)) {}
	};

	extraData_ = std::make_shared<MountExtraData>(ConfigurationOption<bool>("Enable underwater Skimmer", "underwater_skimmer", "wheel_" + nickname_, false));

	extraUI_.emplace();
	extraUI_->interaction = [this]() {
		auto* extraData = static_cast<MountExtraData*>(extraData_.get());
	    if(ImGuiConfigurationWrapper(&ImGui::Checkbox, extraData->enableUnderwaterSkimmer))
			aboveWater_.enabled = !extraData->enableUnderwaterSkimmer.value();
		ImGuiHelpTooltip("This enables Skimmer auto-mounting to work underwater (in addition to on the water surface) in conjunction with the Skimming the Depths mastery.");
	};

	aboveWater_.enabled = !static_cast<MountExtraData*>(extraData_.get())->enableUnderwaterSkimmer.value();
	aboveWater_.canToggleOff = true;
	outOfCombat_.enabled = true;
	outOfCombat_.canToggleOff = true;

	doBypassWheel_ = [this](WheelElement*& we) {
		cref mumble = MumbleLink::i();
		if(mumble->isMounted()) {
			const auto& activeElems = GetActiveElements();
			if(!activeElems.empty()) {
		        we = activeElems.front();
			    return we != nullptr;
			}
		}

		if(mumble->isInWvW()) {
		    we = wheelElements_[uint(MountType::WARCLAW) - uint(MountType::FIRST)].get();
			return we != nullptr && we->isBound();
		}

		if(mumble->isSwimmingOnSurface() || mumble->isUnderwater()) {
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
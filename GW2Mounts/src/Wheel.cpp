#include <Main.h>
#include <Wheel.h>
#include <Core.h>
#include <Utility.h>

namespace GW2Addons
{

Wheel::Wheel(uint resourceId, std::string nickname, std::string displayName, IDirect3DDevice9 * dev)
	: keybind_("Show on mouse", nickname), centralKeybind_("Show in center", nickname + "_cl"),
	  centerBehaviorOption_("Center behavior", "center_behavior", "Wheel" + nickname),
	  centerFavoriteOption_("Center favorite", "center_favorite", "Wheel" + nickname),
	  scaleOption_("Scale factor", "scale", "Wheel" + nickname),
	  centerScaleOption_("Center scale factor", "center_scale", "Wheel" + nickname),
	  displayDelayOption_("Show delay (ms)", "delay", "Wheel" + nickname)
{
	D3DXCreateTextureFromResource(dev, Core::i()->dllModule(), MAKEINTRESOURCE(resourceId), &appearance_);
}

Wheel::~Wheel()
{
	COM_RELEASE(appearance_);
}

void Wheel::UpdateHover()
{
	const auto io = ImGui::GetIO();

	D3DXVECTOR2 mousePos;
	mousePos.x = io.MousePos.x / static_cast<float>(Core::i()->screenWidth());
	mousePos.y = io.MousePos.y / static_cast<float>(Core::i()->screenHeight());
	mousePos -= currentPosition_;

	mousePos.y *= static_cast<float>(Core::i()->screenHeight()) / static_cast<float>(Core::i()->screenWidth());

	WheelElement* lastHovered = nullptr;

	// Middle circle does not count as a hover event
	if (!wheelElements_.empty() && D3DXVec2LengthSq(&mousePos) > SQUARE(scaleOption_.value() * 0.135f * centerScaleOption_.value()))
	{
		float mouseAngle = atan2(-mousePos.y, -mousePos.x) - 0.5f * float(M_PI);
		if (mouseAngle < 0)
			mouseAngle += float(2 * M_PI);

		const float elementAngle = float(2 * M_PI) / wheelElements_.size();
		const int elementId = int((mouseAngle - elementAngle / 2) / elementAngle + 1) % wheelElements_.size();

		currentHovered_ = wheelElements_[elementId].get();
	}
	else
		currentHovered_ = nullptr;

	const auto modifiedMountHovered = ModifyCenterHoveredElement(currentHovered_);
	const auto modifiedLastMountHovered = ModifyCenterHoveredElement(lastHovered);

	if (currentHovered_ && lastHovered != currentHovered_ && modifiedLastMountHovered != modifiedMountHovered)
		currentHovered_->currentHoverTime(max(currentTriggerTime_ + displayDelayOption_.value(), TimeInMilliseconds()));
}

WheelElement * Wheel::ModifyCenterHoveredElement(WheelElement * elem)
{
	if (elem)
		return elem;

	switch (centerBehaviorOption_.value())
	{
	case CenterBehavior::PREVIOUS:
		return previousHovered_;
	case CenterBehavior::FAVORITE:
		for(const auto& e : wheelElements_)
			if(int(e->elementId()) == centerFavoriteOption_.value())
				return e.get();
	default:
		return nullptr;
	}
}

}
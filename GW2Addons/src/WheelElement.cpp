#include <WheelElement.h>
#include <Core.h>
#include <UnitQuad.h>
#include <Utility.h>
#include <Wheel.h>

namespace GW2Addons
{

WheelElement::WheelElement(uint id, std::string nickname, std::string displayName, IDirect3DDevice9* dev)
	: nickname_(nickname), displayName_(displayName), elementId_(id), keybind_(nickname, displayName)
{
	D3DXCreateTextureFromResource(dev, Core::i()->dllModule(), MAKEINTRESOURCE(elementId_), &appearance_);
}

WheelElement::~WheelElement()
{
	COM_RELEASE(appearance_);
}

void WheelElement::Draw(int n, D3DXVECTOR4 spriteDimensions, size_t activeElementsCount, const mstime& currentTime, const WheelElement* elementHovered, const Wheel* parent)
{
	auto fx = Core::i()->mainEffect();
	auto& quad = Core::i()->quad();

	const float hoverTimer = hoverFadeIn(currentTime, parent);

	float mountAngle = float(n) / float(activeElementsCount) * 2 * float(M_PI);
	if (activeElementsCount == 1)
		mountAngle = 0;
	const D3DXVECTOR2 mountLocation = D3DXVECTOR2(cos(mountAngle - float(M_PI) / 2), sin(mountAngle - float(M_PI) / 2)) * 0.25f * 0.66f;

	spriteDimensions.x += mountLocation.x * spriteDimensions.z;
	spriteDimensions.y += mountLocation.y * spriteDimensions.w;

	float mountDiameter = float(sin((2 * M_PI / double(activeElementsCount)) / 2)) * 2.f * 0.2f * 0.66f;
	if (activeElementsCount == 1)
		mountDiameter = 2.f * 0.2f;
	else
		mountDiameter *= Lerp(1.f, 1.1f, SmoothStep(hoverTimer));

	switch (activeElementsCount)
	{
	case 1:
		spriteDimensions.z *= 0.8f;
		spriteDimensions.w *= 0.8f;
		break;
	case 2:
		spriteDimensions.z *= 0.85f;
		spriteDimensions.w *= 0.85f;
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

	spriteDimensions.z *= mountDiameter;
	spriteDimensions.w *= mountDiameter;
	
	fx->SetTexture("texElementImage", appearance_);
	fx->SetVector("g_vSpriteDimensions", &spriteDimensions);
	fx->SetInt("g_iElementID", elementId());
	fx->SetValue("g_vElementColor", color().data(), sizeof(D3DXVECTOR4));
	fx->CommitChanges();

	quad->Draw();
}

float WheelElement::hoverFadeIn(const mstime& currentTime, const Wheel* parent) const
{
	const auto hoverIn = std::min(1.f, (currentTime - std::max(currentHoverTime_, parent->currentTriggerTime_ + parent->displayDelayOption_.value())) / 1000.f * 6);
	const auto hoverOut = 1.f - std::min(1.f, (currentTime - std::max(currentExitTime_, parent->currentTriggerTime_ + parent->displayDelayOption_.value())) / 1000.f * 6);

	return parent->currentHovered_ == this ? hoverIn : std::min(hoverIn, hoverOut);
}

}
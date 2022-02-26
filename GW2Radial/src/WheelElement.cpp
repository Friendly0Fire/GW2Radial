#include <WheelElement.h>
#include <Core.h>
#include <Utility.h>
#include <Wheel.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <ShaderManager.h>
#include <VSCB.h>

namespace GW2Radial
{
ConstantBuffer<WheelElement::WheelElementCB> WheelElement::cb_s;

WheelElement::WheelElement(uint id, const std::string &nickname, const std::string &category,
							const std::string &displayName, ID3D11Device* dev, const Texture2D& tex)
	: nickname_(nickname), displayName_(displayName), elementId_(id),
	  isShownOption_(displayName + " Visible", nickname + "_visible", category, true),
	  sortingPriorityOption_(displayName + " Priority", nickname + "_priority", category, int(id)),
	  keybind_(nickname, displayName, category),
	  appearance_(tex)
{
	if (!appearance_.srv)
		appearance_ = CreateTextureFromResource<ID3D11Texture2D>(dev, Core::i().dllModule(), elementId_);

	GW2_ASSERT(appearance_.srv);

	D3D11_TEXTURE2D_DESC desc;
	appearance_.texture->GetDesc(&desc);

	aspectRatio_ = float(desc.Height) / float(desc.Width);
	texWidth_ = float(desc.Width);

	if (!cb_s.IsValid())
		cb_s = ShaderManager::i().MakeConstantBuffer<WheelElementCB>();
}

int WheelElement::DrawPriority(int extremumIndicator)
{
	ImVec4 col = ConvertVector(color());
	ImGui::PushStyleColor(ImGuiCol_Text, col);

	ImGuiConfigurationWrapper(&ImGui::Checkbox, ("##Displayed" + nickname_).c_str(), isShownOption_);
	ImGui::SameLine();
	if(!isShownOption_.value() || !isActive())
		ImGui::PushFont(Core::i().fontItalic());
	auto displayName = displayName_;
	if(!keybind_.isSet())
		displayName += " [No keybind]";
	ImGui::Text(displayName.c_str());
	if(!isShownOption_.value() || !isActive())
		ImGui::PopFont();

	ImGui::PushFont(Core::i().fontIcon());
	
	int rv = 0;
	if(extremumIndicator != 1)
	{
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - 2 * ImGui::GetFrameHeightWithSpacing());
		if(ImGui::Button((reinterpret_cast<const char*>(ICON_FA_ARROW_UP) + std::string("##PriorityValueUp") + nickname_).c_str()))
			rv = 1;
	}
	if(extremumIndicator != -1)
	{
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - ImGui::GetFrameHeightWithSpacing());
		if(ImGui::Button((reinterpret_cast<const char*>(ICON_FA_ARROW_DOWN) + std::string("##PriorityValueDown") + nickname_).c_str()))
			rv = -1;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor();

	return rv;
}

void WheelElement::SetShaderState(ID3D11DeviceContext* ctx)
{
	fVector4 adjustedColor = color();
	adjustedColor.x = Lerp(1, adjustedColor.x, colorizeAmount_);
	adjustedColor.y = Lerp(1, adjustedColor.y, colorizeAmount_);
	adjustedColor.z = Lerp(1, adjustedColor.z, colorizeAmount_);

	auto& sm = ShaderManager::i();

	cb_s->adjustedColor = adjustedColor;
	cb_s->premultiplyAlpha = premultiplyAlpha_;

	cb_s.Update();
	ctx->PSSetConstantBuffers(1, 1, cb_s.buffer().GetAddressOf());
}

void WheelElement::SetShaderState(ID3D11DeviceContext* ctx, const fVector4& spriteDimensions, ID3D11Buffer* wheelCb, bool shadow)
{
	fVector4 adjustedColor = color();
	adjustedColor.x = Lerp(1, adjustedColor.x, colorizeAmount_);
	adjustedColor.y = Lerp(1, adjustedColor.y, colorizeAmount_);
	adjustedColor.z = Lerp(1, adjustedColor.z, colorizeAmount_);
	
	auto& sm = ShaderManager::i();

	cb_s->elementId = elementId();
	cb_s->adjustedColor = shadow ? fVector4 { 0.f, 0.f, 0.f, shadowStrength_ } : adjustedColor;
	cb_s->premultiplyAlpha = shadow ? false : premultiplyAlpha_;

	cb_s.Update();
	ID3D11Buffer* cbs[] = {
		wheelCb,
		cb_s.buffer().Get()
	};
	ctx->PSSetConstantBuffers(0, std::size(cbs), cbs);

	auto& vscb = GetVSCB();
	vscb->spriteDimensions = spriteDimensions;
	if (shadow)
	{
		vscb->spriteDimensions.z *= 1.01f;
		vscb->spriteDimensions.w *= 1.01f;
	}
	vscb->spriteZ = shadow ? 0.f : 0.02f;
	vscb.Update();
	ctx->VSSetConstantBuffers(0, 1, vscb.buffer().GetAddressOf());
}

void WheelElement::Draw(ComPtr<ID3D11DeviceContext>& ctx, int n, fVector4 spriteDimensions, size_t activeElementsCount, const mstime& currentTime, const WheelElement* elementHovered, const Wheel* parent)
{
	const float hoverTimer = hoverFadeIn(currentTime, parent);

	float elementAngle = float(n) / float(activeElementsCount) * 2 * float(M_PI);
	if (activeElementsCount == 1)
		elementAngle = 0;
	const fVector2 elementLocation = fVector2{cos(elementAngle - float(M_PI) / 2) * 0.2f, sin(elementAngle - float(M_PI) / 2) * 0.2f };

	spriteDimensions.x += elementLocation.x * spriteDimensions.z;
	spriteDimensions.y += elementLocation.y * spriteDimensions.w;

	float elementDiameter = float(sin((2 * M_PI / double(activeElementsCount)) / 2)) * 2.f * 0.2f * 0.66f;
	if (activeElementsCount == 1)
		elementDiameter = 2.f * 0.2f;
	else
		elementDiameter *= Lerp(1.f, 1.1f, SmoothStep(hoverTimer));

	switch (activeElementsCount)
	{
	case 1:
		spriteDimensions.z *= 0.5f;
		spriteDimensions.w *= 0.5f;
		break;
	case 2:
		spriteDimensions.z *= 0.7f;
		spriteDimensions.w *= 0.7f;
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

	spriteDimensions.z *= elementDiameter;
	spriteDimensions.w *= elementDiameter;
	
	spriteDimensions.w *= aspectRatio_;

	ctx->PSSetShaderResources(1, 1, appearance_.srv.GetAddressOf());

	if (shadowStrength_ > 0.f)
	{
		SetShaderState(ctx.Get(), spriteDimensions, parent->GetConstantBuffer(), true);

		DrawScreenQuad(ctx);
	}

	SetShaderState(ctx.Get(), spriteDimensions, parent->GetConstantBuffer(), false);

	DrawScreenQuad(ctx);
}

float WheelElement::hoverFadeIn(const mstime& currentTime, const Wheel* parent) const
{
	const auto hoverIn = std::min(1.f, (currentTime - std::max(currentHoverTime_, parent->currentTriggerTime_ + parent->displayDelayOption_.value())) / 1000.f * 6);
	const auto hoverOut = 1.f - std::min(1.f, (currentTime - std::max(currentExitTime_, parent->currentTriggerTime_ + parent->displayDelayOption_.value())) / 1000.f * 6);

	return parent->currentHovered_ == this ? hoverIn : std::min(hoverIn, hoverOut);
}

}
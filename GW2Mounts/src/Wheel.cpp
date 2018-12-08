#include <Main.h>
#include <Wheel.h>
#include <Core.h>
#include <Utility.h>
#include <Unitquad.h>

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

void Wheel::Draw(IDirect3DDevice9* dev, ID3DXEffect* fx, UnitQuad* quad)
{
	if (isVisible_ && fx && quad)
	{
		const int screenWidth = Core::i()->screenWidth();
		const int screenHeight = Core::i()->screenHeight();

		quad->Bind();

		auto currentTime = TimeInMilliseconds();

		if (currentTime >= currentTriggerTime_ + displayDelayOption_.value())
		{
			uint passes = 0;

			// Setup viewport
			D3DVIEWPORT9 vp;
			vp.X = vp.Y = 0;
			vp.Width = screenWidth;
			vp.Height = screenHeight;
			vp.MinZ = 0.0f;
			vp.MaxZ = 1.0f;
			dev->SetViewport(&vp);

			D3DXVECTOR4 screenSize(float(screenWidth), float(screenHeight), 1.f / screenWidth, 1.f / screenHeight);

			auto ActiveMounts = GetActiveMounts();
			if (!ActiveMounts.empty())
			{
				D3DXVECTOR4 baseSpriteDimensions;
				baseSpriteDimensions.x = currentPosition_.x;
				baseSpriteDimensions.y = currentPosition_.y;
				baseSpriteDimensions.z = scaleOption_.value() * 0.5f * screenSize.y * screenSize.z;
				baseSpriteDimensions.w = scaleOption_.value() * 0.5f;

				const float fadeTimer = min(1.f, (currentTime - (currentTriggerTime_ + displayDelayOption_.value())) / 1000.f * 6);
				const float hoverTimer = min(1.f, (currentTime - max(MountHoverTime, currentTriggerTime_ + displayDelayOption_.value())) / 1000.f * 6);

				auto mountHovered = ModifyCenterHoveredElement(currentHovered_);

				fx->SetTechnique("BgImage");
				fx->SetTexture("texBgImage", appearance_);
				fx->SetVector("g_vSpriteDimensions", &baseSpriteDimensions);
				fx->SetFloat("g_fFadeTimer", fadeTimer);
				fx->SetFloat("g_fHoverTimer", hoverTimer);
				fx->SetFloat("g_fTimer", fmod(currentTime / 1010.f, 55000.f));
				fx->SetFloat("g_fDeadZoneScale", centerScaleOption_.value());
				fx->SetInt("g_iMountCount", int(ActiveMounts.size()));
				fx->SetInt("g_iMountHovered", int(std::find(ActiveMounts.begin(), ActiveMounts.end(), mountHovered) - ActiveMounts.begin()));
				fx->SetBool("g_bCenterGlow", mountHovered != currentHovered_);
				fx->Begin(&passes, 0);
				fx->BeginPass(0);
				quad->Draw();
				fx->EndPass();
				fx->End();

				fx->SetTechnique("MountImage");
				fx->SetTexture("texBgImage", appearance_);
				fx->SetVector("g_vScreenSize", &screenSize);
				fx->Begin(&passes, 0);
				fx->BeginPass(0);

				int n = 0;
				for (auto it : ActiveMounts)
				{
					D3DXVECTOR4 spriteDimensions = baseSpriteDimensions;

					float mountAngle = (float)n / (float)ActiveMounts.size() * 2 * (float)M_PI;
					if (ActiveMounts.size() == 1)
						mountAngle = 0;
					D3DXVECTOR2 mountLocation = D3DXVECTOR2(cos(mountAngle - (float)M_PI / 2), sin(mountAngle - (float)M_PI / 2)) * 0.25f * 0.66f;

					spriteDimensions.x += mountLocation.x * spriteDimensions.z;
					spriteDimensions.y += mountLocation.y * spriteDimensions.w;

					float mountDiameter = (float)sin((2 * M_PI / (double)ActiveMounts.size()) / 2) * 2.f * 0.2f * 0.66f;
					if (ActiveMounts.size() == 1)
						mountDiameter = 2.f * 0.2f;
					if (it == mountHovered)
						mountDiameter *= Lerp(1.f, 1.1f, SmoothStep(hoverTimer));
					else
						mountDiameter *= 0.9f;

					switch (ActiveMounts.size())
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
					}

					spriteDimensions.z *= mountDiameter;
					spriteDimensions.w *= mountDiameter;

					int v[3] = { (int)it, n, (int)ActiveMounts.size() };
					fx->SetValue("g_iMountID", v, sizeof(v));
					fx->SetBool("g_bMountHovered", mountHovered == it);
					fx->SetTexture("texMountImage", MountTextures[(uint)it]);
					fx->SetVector("g_vSpriteDimensions", &spriteDimensions);
					fx->SetValue("g_vColor", MountColors[(uint)it].data(), sizeof(D3DXVECTOR4));
					fx->CommitChanges();

					quad->Draw();
					n++;
				}

				fx->EndPass();
				fx->End();
			}

			{
				const auto& io = ImGui::GetIO();

				fx->SetTechnique("Cursor");
				fx->SetTexture("texBgImage", appearance_);
				D3DXVECTOR4 spriteDimensions(io.MousePos.x * screenSize.z, io.MousePos.y * screenSize.w, 0.05f  * screenSize.y * screenSize.z, 0.05f);
				fx->SetVector("g_vSpriteDimensions", &spriteDimensions);

				fx->Begin(&passes, 0);
				fx->BeginPass(0);
				quad->Draw();
				fx->EndPass();
				fx->End();
			}
		}
	}
}

void Wheel::OnFocusLost()
{
	currentHovered_ = previousHovered_ = nullptr;
	isVisible_ = false;
	currentTriggerTime_ = 0;
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
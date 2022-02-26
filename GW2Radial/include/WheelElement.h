#pragma once
#include <Main.h>
#include <ImGuiExtensions.h>
#include <SettingsMenu.h>
#include <ShaderManager.h>

namespace GW2Radial
{

class WheelElement
{
public:
	WheelElement(uint id, const std::string &nickname, const std::string &category, const std::string &displayName,
		ID3D11Device* dev, const Texture2D& tex = {});
	virtual ~WheelElement() = default;

	int DrawPriority(int extremumIndicator);

	void SetShaderState(ID3D11DeviceContext* ctx);
	void SetShaderState(ID3D11DeviceContext* ctx, const fVector4& spriteDimensions, ID3D11Buffer* wheelCb, bool shadow);
	void Draw(ComPtr<ID3D11DeviceContext>& ctx, int n, fVector4 spriteDimensions, size_t activeElementsCount, const mstime& currentTime, const WheelElement* elementHovered, const class Wheel* parent);

	uint elementId() const { return elementId_; }
	
	int sortingPriority() const { return sortingPriorityOption_.value(); }
	void sortingPriority(int value) { return sortingPriorityOption_.value(value); }
	
	const std::string& nickname() const { return nickname_; }
	const std::string& displayName() const { return displayName_; }

	mstime currentHoverTime() const { return currentHoverTime_; }
	void currentHoverTime(mstime cht) { currentHoverTime_ = cht; }

	mstime currentExitTime() const { return currentExitTime_; }
	void currentExitTime(mstime cht) { currentExitTime_ = cht; }

	float hoverFadeIn(const mstime& currentTime, const Wheel* parent) const;
	
	float shadowStrength() const { return shadowStrength_; }
	void shadowStrength(float ss) { shadowStrength_ = ss; }
	
	float colorizeAmount() const { return colorizeAmount_; }
	void colorizeAmount(float ca) { colorizeAmount_ = ca; }
	
	const Keybind& keybind() const { return keybind_; }
	Keybind& keybind() { return keybind_; }
	bool isBound() const { return keybind_.isSet(); }
	virtual bool isActive() const { return isBound() && isShownOption_.value(); }

	const Texture2D& appearance() const { return appearance_; }

protected:
	virtual fVector4 color() = 0;
	
	ConfigurationOption<bool> isShownOption_;
	ConfigurationOption<int> sortingPriorityOption_;

	std::string nickname_, displayName_;
	uint elementId_;
	Keybind keybind_;
	Texture2D appearance_;
	mstime currentHoverTime_ = 0;
	mstime currentExitTime_ = 0;
	float aspectRatio_ = 1.f;
	float shadowStrength_ = 0.8f;
	float colorizeAmount_ = 1.f;
	float texWidth_ = 0.f;
	bool premultiplyAlpha_ = false;

	struct WheelElementCB
	{
		fVector4 adjustedColor;
		int elementId;
		bool premultiplyAlpha;
	};

	static ConstantBuffer<WheelElementCB> cb_s;
};

}

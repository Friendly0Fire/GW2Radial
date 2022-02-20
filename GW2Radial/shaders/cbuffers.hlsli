#define PI 3.14159f
#define SQRT2 1.4142136f
#define ONE_OVER_SQRT2 0.707107f
#define WHEEL_MAX_ELEMENT_COUNT 12

SamplerState MainSampler : register(s0);
SamplerState SecondarySampler : register(s1);

Texture2D<float4> BackgroundTexture : register(t0);
Texture2D<float4> WipeMaskTexture : register(t1);
Texture2D<float4> IconTexture : register(t1);

cbuffer VS : register(b0)
{
	float4 spriteDimensions;
};

cbuffer Wheel : register(b0)
{
	float3 wipeMaskData;
	float wheelFadeIn;
	float animationTimer;
	float centerScale;
	int elementCount;
	float globalOpacity;
	float hoverFadeIns[WHEEL_MAX_ELEMENT_COUNT];
	float timeLeft;
	bool showIcon;
};

cbuffer WheelElement : register(b1)
{
	float4 adjustedColor;
	float4 shadowData;
	int elementId;
	bool premultiplyAlpha;
};
#define PI 3.14159f
#define SQRT2 1.4142136f
#define ONE_OVER_SQRT2 0.707107f
#define WHEEL_MAX_ELEMENT_COUNT 12
#include "noise.hlsl"

cbuffer Wheel : register(b0)
{
	float3 wipeMaskData;
	float wheelFadeIn;
	float animationTimer;
	float centerScale;
	int elementCount;
	float globalOpacity;
	float4 hoverFadeIns_[WHEEL_MAX_ELEMENT_COUNT/4];
	float timeLeft;
	bool showIcon;
};

static float hoverFadeIns[WHEEL_MAX_ELEMENT_COUNT] = (float[WHEEL_MAX_ELEMENT_COUNT])hoverFadeIns_;

cbuffer WheelElement : register(b1)
{
	float4 adjustedColor;
	float4 shadowData;
	int elementId;
	bool premultiplyAlpha;
};

SamplerState MainSampler : register(s0);
SamplerState SecondarySampler : register(s1);

Texture2D<float4> BackgroundTexture : register(t0);
Texture2D<float4> WipeMaskTexture : register(t1);
Texture2D<float4> IconTexture : register(t1);

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 UV : TEXCOORD0;
};

float2 makeSmoothRandom(float2 uv, float4 scales, float4 timeScales)
{
	float smoothrandom1 = sin(scales.x * uv.x + animationTimer * timeScales.x) + sin(scales.y * uv.y + animationTimer * timeScales.y);
	float smoothrandom2 = sin(scales.z * uv.x + animationTimer * timeScales.z) + sin(scales.w * uv.y + animationTimer * timeScales.w);

	return float2(smoothrandom1, smoothrandom2);
}

float2 rotate(float2 uv, float angle)
{
	float2x2 mat = float2x2(cos(angle), -sin(angle), sin(angle), cos(angle));
	return mul(uv, mat);
}

float rescale(float value, float2 bounds)
{
	return saturate((value - bounds.x) / (bounds.y - bounds.x));
}

float4 BaseMountImage(float2 uv, texture2D tex, SamplerState samp, out float shadow) {
	shadow = 0;
	float4 color = tex.Sample(samp, uv);
	if (premultiplyAlpha)
		color.rgb *= color.a;
	color *= adjustedColor;

	if (shadowData.x > 0.f)
		shadow = shadowData.x * tex.Sample(samp, uv + shadowData.yz).a;

	return color;
}
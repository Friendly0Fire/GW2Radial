#pragma once

#ifndef _WINDOWS
#define PI 3.14159f
#define SQRT2 1.4142136f
#define ONE_OVER_SQRT2 0.707107f
#endif
#define WHEEL_MAX_ELEMENT_COUNT 12

sampler2D MainSampler : register(s0);
sampler2D SecondarySampler : register(s1);

cbuffer Wheel : register(c0)
{
	float3 wipeMaskData;
	float wheelFadeIn;
	float4 spriteDimensions;
	float animationTimer;
	float centerScale;
	int elementCount;
	float globalOpacity;
	float4 hoverFadeIns[WHEEL_MAX_ELEMENT_COUNT];
	float timeLeft;
	bool showIcon;
};

cbuffer WheelElement : register(c1)
{
	float4 adjustedColor;
	float4 shadowData;
	float4 spriteDimensions;
	int elementId;
	bool premultiplyAlpha;
};
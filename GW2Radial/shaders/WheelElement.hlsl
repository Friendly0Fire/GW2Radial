#include "common.hlsli"

float4 WheelElement(PS_INPUT In) : SV_Target
{
	float shadow;
	float4 color = BaseMountImage(In.UV, IconTexture, MainSampler, shadow);
	
	const float3 lumaDot = float3(0.2126, 0.7152, 0.0722);
	float luma = dot(color.rgb, lumaDot);
	float3 fadedColor = lerp(color.rgb, luma, 0.4f);
	float3 finalColor = lerp(fadedColor, color.rgb, elementHoverFadeIn);

	return float4(finalColor.rgb, color.a) * wheelFadeIn.x * globalOpacity;
}
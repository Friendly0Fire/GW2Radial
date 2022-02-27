#include "common.hlsli"

float4 WheelElement(PS_INPUT In) : SV_Target
{
	float hoverFadeIn = hoverFadeIns[elementId];

	float shadow;
	float4 color = BaseMountImage(In.UV, IconTexture, MainSampler, shadow);
	
	const float3 lumaDot = float3(0.2126, 0.7152, 0.0722);
	float luma = dot(color.rgb, lumaDot);
	float3 fadedColor = lerp(color.rgb, luma, 0.33f);
	float3 finalColor = fadedColor;

	float3 glow = 0;
	if(hoverFadeIn > 0.f)
	{
		finalColor = lerp(fadedColor, color.rgb, hoverFadeIn);

		float glowMask = 0;
		glowMask += 1.f - IconTexture.Sample(MainSampler, In.UV + float2(0.01f, 0.01f)).r;
		glowMask += 1.f - IconTexture.Sample(MainSampler, In.UV + float2(-0.01f, 0.01f)).r;
		glowMask += 1.f - IconTexture.Sample(MainSampler, In.UV + float2(0.01f, -0.01f)).r;
		glowMask += 1.f - IconTexture.Sample(MainSampler, In.UV + float2(-0.01f, -0.01f)).r;

		glow = color.rgb * (glowMask / 4) * hoverFadeIn * 0.5f * (0.5f + 0.5f * srnoise(In.UV * 3.18f + 0.15f * float2(cos(animationTimer * 3), sin(animationTimer * 2))));
	}

	return float4(finalColor.rgb + glow, max(color.a, shadow)) * wheelFadeIn.x * globalOpacity;
}
#include "common.hlsli"

float4 DelayIndicator(PS_INPUT In) : SV_Target
{
	float2 centeredUV = 2 * (In.UV - 0.5f);
	float2 polar = float2(length(centeredUV), atan2(centeredUV.y, centeredUV.x));
	polar.y = fmod(10 - (polar.y / PI + 0.5f) * 0.5f, 1.f); // [0, 1]
	
	float2 smoothrandom = float2(srnoise(3 * In.UV * cos(0.5f * animationTimer) + animationTimer * 0.43f), srnoise(3 * In.UV * sin(0.79f * animationTimer) + animationTimer * 0.22f));
	float4 baseColor = BackgroundTexture.Sample(MainSampler, In.UV);

	float4 color = saturate(baseColor * float2(2, 1).xxxy) * wheelFadeIn;
	color.rgb *= lerp(0.9f, 1.3f, saturate((4 + smoothrandom.x + smoothrandom.y) / 8));
	color.rgb *= 1.25f - (1 - smoothstep(0.60f, 0.80f, polar.x));

	color.rgb *= 1.f - smoothstep(0.80f, 0.90f, polar.x);
	color.a *= 1.f - smoothstep(0.90f, 1.00f, polar.x);
	
	color *= 1.25f - smoothstep(timeLeft - 0.01f, timeLeft + 0.01f, polar.y);
	color.rgb *= smoothstep(0.f, 0.02f, abs(timeLeft - polar.y));

	if(showIcon) {
	    float iconShadow;
	    float4 iconColor = BaseMountImage(centeredUV * 0.85f + 0.5f, IconTexture, SecondarySampler, iconShadow);

	    color.rgb *= 1 - max(iconShadow, iconColor.a);
	    color.rgb += iconColor.rgb * wheelFadeIn;
	}

	return color;
}
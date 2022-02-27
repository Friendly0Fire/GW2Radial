#include "common.hlsli"

float4 Cursor(PS_INPUT In) : SV_Target
{
	float2 centeredUV = 2 * (In.UV - 0.5f);
	float2 polar = float2(length(centeredUV), atan2(centeredUV.y, centeredUV.x));
	polar.y = 2 * (polar.y / PI + 1);
	float smoothrandom = psrnoise(polar * float2(1.f, 0.5f) - float2(0.5f, 0.05f) * animationTimer, float2(100, 2), wipeMaskData.z / (2 * PI));

	float4 color = float4(194/255.f,189/255.f,149/255.f, 1.f);
	color *= pow(1.f - smoothstep(0.f, 1.f, polar.x), 4.f);
	color *= 1 - lerp(0.1f, 1.f, smoothstep(0.2f, 0.6f, polar.x)) * smoothrandom;

	return float4(color.rgb * globalOpacity, 0.f);
}
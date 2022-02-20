#include "cbuffers.hlsli"
#include "noise.hlsl"

struct PS_INPUT
{
	float4 pos : POSITION;
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

float GetWipeValue(in float2 uv, in float3 data, in float offset)
{
	uv -= 0.5f;
	uv /= 0.75f + pow(offset, 0.3f);
	uv += 0.5f + (rotate(0.5f, data.z) * pow(offset, 0.1f) - 0.5f) * 0.25f;

	float3 wipe = WipeMaskTexture.Sample(SecondarySampler, uv).rgb;
	return lerp(wipe.b, 1.f, pow(offset, 0.6f));
}

float4 BgImage_PS(PS_INPUT In) : SV_Target0
{
	float currentWheelFadeIn = GetWipeValue(In.UV, wipeMaskData, wheelFadeIn.x);

	// Multiply by -3 rather than 2 to mirror and scale down
	float2 coords = -3 * (In.UV - 0.5f);
	// Compute polar coordinates with theta \in [0, 2pi)
	float2 coordsPolar = float2(length(coords), atan2(coords.y, coords.x) + PI);
	// Compensate for different theta = 0 direction
	coordsPolar.y += 0.5f * PI;
	
	// Angular span covered by a single mount (e.g. if 4 mounts are shown, the span is 90 degrees)
	float singleMountAngle = 2.f * PI / float(elementCount);
    // Determine the local mount ID, making sure to wrap around
    int localMountId = (int) round(coordsPolar.y / singleMountAngle);
    if (localMountId >= elementCount)
        localMountId -= elementCount;
	float hoverFadeIn = hoverFadeIns[localMountId];
    bool isLocalMountHovered = hoverFadeIn > 0.f;

	hoverFadeIn = min(hoverFadeIn, wheelFadeIn.x);
    
	// Percentage along the mount's angular span: 0 is one edge, 1 is the other
	// Must also compensate since the spans are centered, but we want to start at one edge
    float localCoordPercentage = fmod(coordsPolar.y + 0.5f * singleMountAngle, singleMountAngle) / singleMountAngle;
	
	// Generate a pseudorandom background
	float2 smoothrandom = float2(srnoise(3 * In.UV * cos(0.1f * animationTimer) + animationTimer * 0.37f), srnoise(5 * In.UV * sin(0.13f * animationTimer) + animationTimer * 0.48f));
	float4 color = BackgroundTexture.Sample(MainSampler, In.UV);
	color.a = 1.f;
	color.rgb *= 2 * lerp(0.9f, 1.3f, saturate((4 + smoothrandom.x + smoothrandom.y) / 8));
	// Compute luma value for desaturation effects
	float luma = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));

	// Fade out the background at the periphery of the circle
	float edge_mask = lerp(1.f, 0.f, smoothstep(0.5f, 1.f, coordsPolar.x * (1 - luma * 0.2f)));
	// Fade out the background in the dead zone at the center of the circle
	float center_mask = lerp(0.f, 1.f, smoothstep(centerScale - 0.01f, centerScale + 0.01f, coordsPolar.x * (1 - luma * 0.2f)));
	
	// Fade out background for non-hovered sections, but don't affect the center area
    color.rgb *= lerp(1.f, lerp(1.f, 1.5f, hoverFadeIn), center_mask);
	
	// Increase brightness on hovered and edge regions of the circle
	float border_mask = 1.f;

	// Calculate edge regions (localCoordPercentage is near 0 or 1), but only for non-hovered mount spans
    if (hoverFadeIn < 1)
	{
		float min_thickness = 0.004f / (0.001f + coordsPolar.x);
		float max_thickness = 0.006f / (0.001f + coordsPolar.x);
	
		if(elementCount > 1)
		{
            border_mask *= lerp(2.f, 1.f, smoothstep(min_thickness, max_thickness, localCoordPercentage));
            border_mask *= lerp(1.f, 2.f, smoothstep(1 - max_thickness, 1 - min_thickness, localCoordPercentage));
			border_mask = lerp(1.f, border_mask, center_mask);
        }
	}

	// Reduce brightening when starting to hover to fade in gracefully
    if (isLocalMountHovered && hoverFadeIn < 1)
		border_mask = lerp(border_mask, 1.f, hoverFadeIn);
	
	// Add some flair to the inner region of the circle
	border_mask *= 2.f - smoothstep(centerScale + 0.01f, centerScale + 0.1f, coordsPolar.x);

	// Also brighten when the dead zone is hovered and has an action assigned to it
	if (hoverFadeIns[elementCount] > 0.f)
	{
		color.rgb *= lerp(lerp(1.f, 1.5f, hoverFadeIns[elementCount]), 1.f, center_mask);
		center_mask = lerp(center_mask, 1 - luma * 0.2f, hoverFadeIns[elementCount]);
	}

	// Combine all masks, ensuring that the edge and center masks never increase brightness when combined and that the border mask never darkens the circle
	return color * saturate(edge_mask * center_mask) * clamp(border_mask, 1.f, 2.f) * clamp(luma, 0.8f, 1.2f) * currentWheelFadeIn * float4(1, 1, 1, 1.2f) * globalOpacity;
}

float4 BaseMountImage(float2 uv, texture2D tex, SamplerState samp, out float shadow) {
    shadow = 0;
	float4 color = tex.Sample(samp, uv);
	if(premultiplyAlpha)
		color.rgb *= color.a;
	color *= adjustedColor;

	if(shadowData.x > 0.f)
		shadow = shadowData.x * tex.Sample(samp, uv + shadowData.yz).a;

	return color;
}

float4 MountImage_PS(PS_INPUT In) : SV_Target
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

float4 Cursor_PS(PS_INPUT In) : SV_Target
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

float LogNormal(float x, float sigma)
{
	x += 1e-4f;
	float exponent = log(x) / (2 * sigma);
    return 1.f / (x * sigma * sqrt(2 * PI)) * exp(-exponent * exponent);
}

float4 TimerCursor_PS(PS_INPUT In) : SV_Target
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
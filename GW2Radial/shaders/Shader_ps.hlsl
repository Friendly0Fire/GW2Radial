#pragma warning(disable : 4717)
#define SHADER_PS
#include "noise.hlsl"
#include "registers.h"

struct PS_INPUT
{
	float4 pos : POSITION;
	float2 UV: TEXCOORD0;
};

float2 makeSmoothRandom(float2 uv, float4 scales, float4 timeScales)
{
	float smoothrandom1 = sin(scales.x * uv.x + fAnimationTimer * timeScales.x) + sin(scales.y * uv.y + fAnimationTimer * timeScales.y);
	float smoothrandom2 = sin(scales.z * uv.x + fAnimationTimer * timeScales.z) + sin(scales.w * uv.y + fAnimationTimer * timeScales.w);

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
	uv += (rotate(0.5f, data.z) * pow(offset, 0.1f) - 0.5f) * 0.25f;

	float3 wipe = tex2D(texWipeMaskImageSampler, uv + 0.5f).rgb;
	return lerp(wipe.b, 1.f, pow(offset, 0.6f));
}

float fHoverFadeIns_lookup(int idx)
{
	return fHoverFadeIns[idx].x;
}

float4 BgImage_PS(PS_INPUT In) : COLOR0
{
	float wheelFadeIn = GetWipeValue(In.UV, fWipeMaskData, fWheelFadeIn.x);

	// Multiply by -3 rather than 2 to mirror and scale down
	float2 coords = -3 * (In.UV - 0.5f);
	// Compute polar coordinates with theta \in [0, 2pi)
	float2 coordsPolar = float2(length(coords), atan2(coords.y, coords.x) + PI);
	// Compensate for different theta = 0 direction
	coordsPolar.y += 0.5f * PI;
	
	// Angular span covered by a single mount (e.g. if 4 mounts are shown, the span is 90 degrees)
	float singleMountAngle = 2.f * PI / float(iElementCount);
    // Determine the local mount ID, making sure to wrap around
    int localMountId = (int) round(coordsPolar.y / singleMountAngle);
    if (localMountId >= iElementCount)
        localMountId -= iElementCount;
	float hoverFadeIn = fHoverFadeIns_lookup(localMountId);
    bool isLocalMountHovered = hoverFadeIn > 0.f;

	hoverFadeIn = min(hoverFadeIn, fWheelFadeIn.x);
    
	// Percentage along the mount's angular span: 0 is one edge, 1 is the other
	// Must also compensate since the spans are centered, but we want to start at one edge
    float localCoordPercentage = fmod(coordsPolar.y + 0.5f * singleMountAngle, singleMountAngle) / singleMountAngle;
	
	// Generate a pseudorandom background
	float2 smoothrandom = float2(srnoise(3 * In.UV * cos(0.1f * fAnimationTimer) + fAnimationTimer * 0.37f), srnoise(5 * In.UV * sin(0.13f * fAnimationTimer) + fAnimationTimer * 0.48f));
	float4 color = tex2D(texMainSampler, In.UV);
	color.a = 1.f;
	color.rgb *= 2 * lerp(0.9f, 1.3f, saturate((4 + smoothrandom.x + smoothrandom.y) / 8));
	// Compute luma value for desaturation effects
	float luma = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));

	// Fade out the background at the periphery of the circle
	float edge_mask = lerp(1.f, 0.f, smoothstep(0.5f, 1.f, coordsPolar.x * (1 - luma * 0.2f)));
	// Fade out the background in the dead zone at the center of the circle
	float center_mask = lerp(0.f, 1.f, smoothstep(fCenterScale - 0.01f, fCenterScale + 0.01f, coordsPolar.x * (1 - luma * 0.2f)));
	
	// Fade out background for non-hovered sections, but don't affect the center area
    color.rgb *= lerp(1.f, lerp(1.f, 1.5f, hoverFadeIn), center_mask);
	
	// Increase brightness on hovered and edge regions of the circle
	float border_mask = 1.f;

	// Calculate edge regions (localCoordPercentage is near 0 or 1), but only for non-hovered mount spans
    if (hoverFadeIn < 1)
	{
		float min_thickness = 0.004f / (0.001f + coordsPolar.x);
		float max_thickness = 0.006f / (0.001f + coordsPolar.x);
	
		if(iElementCount > 1)
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
	border_mask *= 2.f - smoothstep(fCenterScale + 0.01f, fCenterScale + 0.1f, coordsPolar.x);

	// Also brighten when the dead zone is hovered and has an action assigned to it
	if (fHoverFadeIns_lookup(iElementCount) > 0.f)
	{
		color.rgb *= lerp(lerp(1.f, 1.5f, fHoverFadeIns_lookup(iElementCount)), 1.f, center_mask);
		center_mask = lerp(center_mask, 1 - luma * 0.2f, fHoverFadeIns_lookup(iElementCount));
	}

	// Combine all masks, ensuring that the edge and center masks never increase brightness when combined and that the border mask never darkens the circle
	return color * saturate(edge_mask * center_mask) * clamp(border_mask, 1.f, 2.f) * clamp(luma, 0.8f, 1.2f) * wheelFadeIn * float4(1, 1, 1, 1.2f);
}

float4 MountImage_PS(PS_INPUT In) : COLOR0
{
	float hoverFadeIn = fHoverFadeIns_lookup(iElementID);

	float shadow = 0;
	float4 color = tex2D(texMainSampler, In.UV) * fElementColor;
	if(fShadowData.x > 0.f)
		shadow = fShadowData.x * (tex2D(texMainSampler, In.UV + fShadowData.yz).a);
	
	float3 fLumaDot = float3(0.2126, 0.7152, 0.0722);
	float luma = dot(color.rgb, fLumaDot);
	float3 fadedColor = lerp(color.rgb, luma, 0.33f);
	float3 finalColor = fadedColor;

	float3 glow = 0;
	if(hoverFadeIn > 0.f)
	{
		finalColor = lerp(fadedColor, color.rgb, hoverFadeIn);

		float glowMask = 0;
		glowMask += 1.f - tex2D(texMainSampler, In.UV + float2(0.01f, 0.01f)).r;
		glowMask += 1.f - tex2D(texMainSampler, In.UV + float2(-0.01f, 0.01f)).r;
		glowMask += 1.f - tex2D(texMainSampler, In.UV + float2(0.01f, -0.01f)).r;
		glowMask += 1.f - tex2D(texMainSampler, In.UV + float2(-0.01f, -0.01f)).r;

		glow = color.rgb * (glowMask / 4) * hoverFadeIn * 0.5f * (0.5f + 0.5f * srnoise(In.UV * 3.18f + 0.15f * float2(cos(fAnimationTimer * 3), sin(fAnimationTimer * 2))));
	}

	return float4(finalColor.rgb + glow, max(color.a, shadow)) * fWheelFadeIn.x;
}

float4 Cursor_PS(PS_INPUT In) : COLOR0
{
	float2 centeredUV = 2 * (In.UV - 0.5f);
	float2 polar = float2(length(centeredUV), atan2(centeredUV.y, centeredUV.x));
	polar.y = 2 * (polar.y / PI + 1);
	float smoothrandom = psrnoise(polar * float2(1.f, 0.5f) - float2(0.5f, 0.05f) * fAnimationTimer, float2(100, 2), fWipeMaskData.z / (2 * PI));

	float4 color = float4(194/255.f,189/255.f,149/255.f, 1.f);
	color *= pow(1.f - smoothstep(0.f, 1.f, polar.x), 4.f);
	color *= 1 - lerp(0.1f, 1.f, smoothstep(0.2f, 0.6f, polar.x)) * smoothrandom;

	return color;
}

float LogNormal(float x, float sigma)
{
	x += 1e-4f;
	float exponent = log(x) / (2 * sigma);
    return 1.f / (x * sigma * sqrt(2 * PI)) * exp(-exponent * exponent);
}

float4 TimerCursor_PS(PS_INPUT In) : COLOR0
{
	float2 centeredUV = 2 * (In.UV - 0.5f);
	float2 polar = float2(length(centeredUV), atan2(centeredUV.y, centeredUV.x));
	polar.y = 2 * (polar.y / PI + 1); // [0, 4]

	float rotatingPolarY = fmod(2 * fAnimationTimer - polar.y, 4.f);

	const float4 baseColor = float4(194/255.f,189/255.f,149/255.f, 0.f) * 0.75f * fWheelFadeIn;
	float4 color = baseColor;
	color *= pow(1.f - smoothstep(0.5f, 1.f, polar.x), 4.f);
	color *= pow(smoothstep(0.f, 0.5f, polar.x), 4.f);
	color *= LogNormal(rotatingPolarY, 0.5f) * (1 - smoothstep(3.5f, 4.f, rotatingPolarY));

	color += In.UV.y > 0.95f ? (In.UV.x < fCenterScale ? baseColor : (baseColor * 0.2f + float4(0, 0, 0, 0.6f * fWheelFadeIn))) : 0;

	return color;
}
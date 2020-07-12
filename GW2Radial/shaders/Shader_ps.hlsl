#pragma warning(disable : 4717)
#include "perlin.hlsl"

#define PI 3.14159f
#define SQRT2 1.4142136f
#define ONE_OVER_SQRT2 0.707107f
#define MAX_ELEMENT_COUNT 9

struct PS_INPUT
{
	float4 pos : POSITION;
	float2 UV: TEXCOORD0;
};

sampler2D texBgImageSampler : register ( s0 );
sampler2D texWipeMaskImageSampler : register ( s1 );
sampler2D texElementImageSampler : register ( s2 );

// Timer for miscellaneous, low importance animations
float g_fAnimationTimer : register ( c0 );
// [0..1] ratio of fade in when opening/dismissing the wheel
float2 g_fWheelFadeIn : register ( c1 );
// [0..1] ratio of the central gap's radius over the total wheel's radius
float g_fCenterScale : register ( c2 );
// Total number of elements, cannot be larger than MAX_ELEMENT_COUNT
float g_iElementCount : register ( c3 );
float3 g_vWipeMaskData : register ( c4 );
// Current element ID
float g_iElementID : register (c5);
// Color of the current element
float4 g_vElementColor : register (c6);

float techId : register (c7);

// [0..1] ratio of fade in when hovering over a wheel element, for every element in the wheel
// MAX_ELEMENT_COUNT-1 is hardcoded to be the center element
float4 g_fHoverFadeIns[MAX_ELEMENT_COUNT] : register (c8);

float2 makeSmoothRandom(float2 uv, float4 scales, float4 timeScales)
{
	float smoothrandom1 = sin(scales.x * uv.x + g_fAnimationTimer * timeScales.x) + sin(scales.y * uv.y + g_fAnimationTimer * timeScales.y);
	float smoothrandom2 = sin(scales.z * uv.x + g_fAnimationTimer * timeScales.z) + sin(scales.w * uv.y + g_fAnimationTimer * timeScales.w);

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
	float3 wipe = tex2D(texWipeMaskImageSampler, uv + float2(offset, 0)).rgb;
	return wipe.g;
}

float g_fHoverFadeIns_lookup(int idx)
{
	return g_fHoverFadeIns[idx].x;
}

float4 BgImage_PS(PS_INPUT In)
{
	float2 wheelFadeIn = GetWipeValue(In.UV, g_vWipeMaskData, g_fWheelFadeIn.y);
	wheelFadeIn *= smoothstep(0.f, 0.05f, g_fWheelFadeIn.x);
	wheelFadeIn = lerp(wheelFadeIn, 1.f, rescale(g_fWheelFadeIn.y, float2(0.8f, 1.f)));

	// Multiply by -3 rather than 2 to mirror and scale down
	float2 coords = -3 * (In.UV - 0.5f);
	// Compute polar coordinates with theta \in [0, 2pi)
	float2 coordsPolar = float2(length(coords), atan2(coords.y, coords.x) + PI);
	// Compensate for different theta = 0 direction
	coordsPolar.y += 0.5f * PI;
	
	// Angular span covered by a single mount (e.g. if 4 mounts are shown, the span is 90 degrees)
	float singleMountAngle = 2.f * PI / float(g_iElementCount);
    // Determine the local mount ID, making sure to wrap around
    int localMountId = (int) round(coordsPolar.y / singleMountAngle);
    if (localMountId >= g_iElementCount)
        localMountId -= g_iElementCount;
	float hoverFadeIn = g_fHoverFadeIns_lookup(localMountId);
    bool isLocalMountHovered = hoverFadeIn > 0.f;
    
	// Percentage along the mount's angular span: 0 is one edge, 1 is the other
	// Must also compensate since the spans are centered, but we want to start at one edge
    float localCoordPercentage = fmod(coordsPolar.y + 0.5f * singleMountAngle, singleMountAngle) / singleMountAngle;
	
	// Generate a pseudorandom background
	float2 smoothrandom = float2(snoise(3 * In.UV * cos(0.1f * g_fAnimationTimer) + g_fAnimationTimer * 0.37f), snoise(5 * In.UV * sin(0.13f * g_fAnimationTimer) + g_fAnimationTimer * 0.48f));
	float4 color = tex2D(texBgImageSampler, In.UV);
	color.rgb *= 1.5f * lerp(0.9f, 1.3f, saturate((4 + smoothrandom.x + smoothrandom.y) / 8));
	// Compute luma value for desaturation effects
	float luma = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));

	// Fade out the background at the periphery of the circle
	float edge_mask = lerp(1.f, 0.f, smoothstep(0.5f, 1.f, coordsPolar.x * (1 - luma * 0.2f)));
	// Fade out the background in the dead zone at the center of the circle
	float center_mask = lerp(0.f, 1.f, smoothstep(g_fCenterScale - 0.01f, g_fCenterScale + 0.01f, coordsPolar.x * (1 - luma * 0.2f)));
	
	// Fade out background for non-hovered sections, but don't affect the center area
    color.rgb *= lerp(1.f, lerp(1.f, 1.5f, hoverFadeIn), center_mask);
	
	// Increase brightness on hovered and edge regions of the circle
	float border_mask = 1.f;

	// Calculate edge regions (localCoordPercentage is near 0 or 1), but only for non-hovered mount spans
    if (hoverFadeIn < 1)
	{
		float min_thickness = 0.004f / (0.001f + coordsPolar.x);
		float max_thickness = 0.006f / (0.001f + coordsPolar.x);
	
		if(g_iElementCount > 1)
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
	border_mask *= 2.f - smoothstep(g_fCenterScale + 0.01f, g_fCenterScale + 0.1f, coordsPolar.x);

	// Also brighten when the dead zone is hovered and has an action assigned to it
	if (g_fHoverFadeIns_lookup(g_iElementCount) > 0.f)
	{
		color.rgb *= lerp(lerp(0.5f, 1.f, g_fHoverFadeIns_lookup(g_iElementCount)), 1.f, center_mask);
		center_mask = lerp(center_mask, 1 - luma * 0.2f, g_fHoverFadeIns_lookup(g_iElementCount));
	}

	// Combine all masks, ensuring that the edge and center masks never increase brightness when combined and that the border mask never darkens the circle
	return color * saturate(edge_mask * center_mask) * clamp(border_mask, 1.f, 2.f) * clamp(luma, 0.8f, 1.2f) * wheelFadeIn.xxxy;
}

float4 MountImage_PS(PS_INPUT In, float imageIsMask)
{
	float hoverFadeIn = g_fHoverFadeIns_lookup(g_iElementID);

	float mask = 1, shadow = 0;
	float4 color = 1;
	if(imageIsMask > 0)
	{
		mask = 1.f - tex2D(texElementImageSampler, In.UV).r;
		shadow = 1.f - tex2D(texElementImageSampler, In.UV + 0.01f).r;

		color = g_vElementColor;
	}
	else
	{
		color = tex2D(texElementImageSampler, In.UV);
	}
	
	float3 g_vLumaDot = float3(0.2126, 0.7152, 0.0722);
	float luma = dot(color.rgb, g_vLumaDot);
	float3 fadedColor = lerp(color.rgb, luma, 0.33f);
	float3 finalColor = fadedColor;

	float3 glow = 0;
	if(hoverFadeIn > 0.f)
	{
		finalColor = lerp(fadedColor, color.rgb, hoverFadeIn);

		float glowMask = 0;
		glowMask += 1.f - tex2D(texElementImageSampler, In.UV + float2(0.01f, 0.01f)).r;
		glowMask += 1.f - tex2D(texElementImageSampler, In.UV + float2(-0.01f, 0.01f)).r;
		glowMask += 1.f - tex2D(texElementImageSampler, In.UV + float2(0.01f, -0.01f)).r;
		glowMask += 1.f - tex2D(texElementImageSampler, In.UV + float2(-0.01f, -0.01f)).r;

		glow = color.rgb * (glowMask / 4) * hoverFadeIn * 0.5f * (0.5f + 0.5f * snoise(In.UV * 3.18f + 0.15f * float2(cos(g_fAnimationTimer * 3), sin(g_fAnimationTimer * 2))));
	}

	return float4(finalColor * mask + glow, color.a * max(mask, shadow)) * g_fWheelFadeIn.x;
}

float4 Cursor_PS(PS_INPUT In)
{
	float2 smoothrandom = makeSmoothRandom(In.UV, float4(15, 18, 18, 15), float4(2.4, 3.1, 2.9, 4.7));

	float4 baseImage = tex2D(texBgImageSampler, In.UV + smoothrandom * 0.003f);
	float radius = length(In.UV * 2 - 1);
	baseImage *= 1.25f * pow(1.f - smoothstep(0.f, 1.f, radius), 4.f);
	baseImage *= lerp(0.8f, 1.5f, saturate((4 + smoothrandom.x + smoothrandom.y) / 8));

	return baseImage;
}

float4 main(PS_INPUT input) : COLOR
{
	if (techId > 3)
		return BgImage_PS(input);		
	else if (techId > 2)
		return MountImage_PS(input, 0);		
	else if (techId > 1)
		return MountImage_PS(input, 1);
	else
		return Cursor_PS(input);
}
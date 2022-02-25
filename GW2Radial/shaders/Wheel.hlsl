#include "common.hlsli"

float GetWipeValue(in float2 uv, in float3 data, in float offset)
{
	uv -= 0.5f;
	uv /= 0.75f + pow(offset, 0.3f);
	uv += 0.5f + (rotate(0.5f, data.z) * pow(offset, 0.1f) - 0.5f) * 0.25f;

	float3 wipe = WipeMaskTexture.Sample(SecondarySampler, uv).rgb;
	return lerp(wipe.b, 1.f, pow(offset, 0.6f));
}

float4 Wheel(PS_INPUT In) : SV_Target0
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
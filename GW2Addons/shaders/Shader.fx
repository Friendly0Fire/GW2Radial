#pragma warning(disable : 4717)
#include "perlin.hlsl"

#define PI 3.14159f
#define SQRT2 1.4142136f
#define ONE_OVER_SQRT2 0.707107f
#define MAX_ELEMENT_COUNT 8

struct VS_SCREEN
{
	float4 Position : POSITION;
	float2 UV : TEXCOORD0;
};

texture texElementImage;

sampler2D texElementImageSampler =
sampler_state
{
    texture = <texElementImage>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

texture texBgImage;

sampler2D texBgImageSampler =
sampler_state
{
    texture = <texBgImage>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

texture texInkImage;

sampler2D texInkImageSampler =
sampler_state
{
    texture = <texInkImage>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

// Screen dimensions as (width, height, 1/width, 1/height)
float4 g_vScreenSize;
// Timer for miscellaneous, low importance animations
float g_fAnimationTimer;
// Current rendered sprite (wheel or wheel element) dimensions as ([0..1] position x, [0..1] position y, scaled size x, scaled size y)
float4 g_vSpriteDimensions;

// [0..1] ratio of fade in when opening/dismissing the wheel
float2 g_fWheelFadeIn;
// [0..1] ratio of the central gap's radius over the total wheel's radius
float g_fCenterScale;
// Total number of elements, cannot be larger than MAX_ELEMENT_COUNT
int g_iElementCount;
// [0..1] ratio of fade in when hovering over a wheel element, for every element in the wheel
// MAX_ELEMENT_COUNT-1 is hardcoded to be the center element
float g_fHoverFadeIns[MAX_ELEMENT_COUNT];
float3 g_vInkSpots[3];

float2 makeSmoothRandom(float2 uv, float4 scales, float4 timeScales)
{
	float smoothrandom1 = sin(scales.x * uv.x + g_fAnimationTimer * timeScales.x) + sin(scales.y * uv.y + g_fAnimationTimer * timeScales.y);
	float smoothrandom2 = sin(scales.z * uv.x + g_fAnimationTimer * timeScales.z) + sin(scales.w * uv.y + g_fAnimationTimer * timeScales.w);

	return float2(smoothrandom1, smoothrandom2);
}

VS_SCREEN Default_VS(in float2 UV : TEXCOORD0)
{
    VS_SCREEN Out = (VS_SCREEN)0;

	float2 dims = (UV * 2 - 1) * g_vSpriteDimensions.zw;

    Out.UV = UV;
    Out.Position = float4(dims + g_vSpriteDimensions.xy * 2 - 1, 0.5f, 1.f);
	Out.Position.y *= -1;

    return Out;
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

float2 GetInkValue(in float2 uv, in float3 centerRotate, in float scale)
{
	return tex2D(texInkImageSampler, rotate((uv - centerRotate.xy) / scale, centerRotate.z) + 0.5f).r * float2(1.f, 1.f + centerRotate.z / 6);
}

float4 BgImage_PS(VS_SCREEN In) : COLOR0
{
	float2 wheelFadeIn = 0.33f + GetInkValue(In.UV, g_vInkSpots[0], 0.02f + 2.5f * pow(g_fWheelFadeIn.y, 0.3f));
	wheelFadeIn = max(wheelFadeIn, GetInkValue(In.UV, g_vInkSpots[1], 0.02f + 3.5f * pow(rescale(g_fWheelFadeIn.y, float2(0.33f, 0.77f)), 0.3f)));
	wheelFadeIn = max(wheelFadeIn, GetInkValue(In.UV, g_vInkSpots[2], 0.02f + 3.5f * pow(rescale(g_fWheelFadeIn.y, float2(0.55f, 1.f)), 0.3f)));
	wheelFadeIn *= smoothstep(0.f, 0.05f, g_fWheelFadeIn.x);
	wheelFadeIn = lerp(saturate(wheelFadeIn), 1.f, rescale(g_fWheelFadeIn.y, float2(0.75f, 1.f)));

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
	float hoverFadeIn = g_fHoverFadeIns[localMountId];
    bool isLocalMountHovered = hoverFadeIn > 0.f;
    
	// Percentage along the mount's angular span: 0 is one edge, 1 is the other
	// Must also compensate since the spans are centered, but we want to start at one edge
    float localCoordPercentage = fmod(coordsPolar.y + 0.5f * singleMountAngle, singleMountAngle) / singleMountAngle;
	
	// Generate a pseudorandom background
	float2 smoothrandom = float2(snoise(3 * In.UV * cos(0.1f * g_fAnimationTimer) + g_fAnimationTimer * 0.37f), snoise(5 * In.UV * sin(0.13f * g_fAnimationTimer) + g_fAnimationTimer * 0.48f));
	float4 color = tex2D(texBgImageSampler, In.UV + smoothrandom * 0.003f);
	color.rgb *= lerp(0.9f, 1.3f, saturate((4 + smoothrandom.x + smoothrandom.y) / 8));
    color.rgb *= lerp(0.5f, 1.f, hoverFadeIn);
	// Compute luma value for desaturation effects
	float luma = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));

	// Fade out the background at the periphery of the circle
	float edge_mask = lerp(1.f, 0.f, smoothstep(0.5f, 1.f, coordsPolar.x * (1 - luma * 0.2f)));
	// Fade out the background in the dead zone at the center of the circle
	float center_mask = lerp(0.f, 1.f, smoothstep(g_fCenterScale - 0.025f, g_fCenterScale + 0.025f, coordsPolar.x * (1 - luma * 0.2f)));
	
	// Increase brightness on hovered and edge regions of the circle
	float border_mask = 1.f;

	// Calculate edge regions (localCoordPercentage is near 0 or 1), but only for non-hovered mount spans
    if (!isLocalMountHovered || hoverFadeIn < 1)
	{
		float min_thickness = 0.003f / (0.001f + coordsPolar.x);
		float max_thickness = 0.005f / (0.001f + coordsPolar.x);
	
		if(g_iElementCount > 1)
		{
            border_mask *= lerp(2.f, 1.f, smoothstep(min_thickness, max_thickness, localCoordPercentage));
            border_mask *= lerp(1.f, 2.f, smoothstep(1 - max_thickness, 1 - min_thickness, localCoordPercentage));
        }
	}

	// Reduce brightening when starting to hover to fade in gracefully
    if (isLocalMountHovered && hoverFadeIn < 1)
		border_mask = lerp(border_mask, 1.f, hoverFadeIn);

	// Also brighten when the dead zone is hovered and has an action assigned to it
	if (g_fHoverFadeIns[g_iElementCount] > 0.f)
	{
        if (isLocalMountHovered)
			border_mask *= 1 - hoverFadeIn;
		center_mask = lerp(center_mask, 1.f, g_fHoverFadeIns[g_iElementCount]);
	}
	
	// Add some flair to the inner region of the circle
	border_mask *= lerp(2.f, 1.f, smoothstep(g_fCenterScale, g_fCenterScale + 0.05f, coordsPolar.x));
	border_mask *= lerp(1.5f, 1.f, smoothstep(g_fCenterScale, min(0.5f, g_fCenterScale * 4), coordsPolar.x));

	// Combine all masks, ensuring that the edge and center masks never increase brightness when combined and that the border mask never darkens the circle
	return color * saturate(edge_mask * center_mask) * clamp(border_mask, 1.f, 2.f) * clamp(luma, 0.8f, 1.2f) * wheelFadeIn.xxxy;
}

technique BgImage
{
	pass P0
	{
		ZEnable = false;
		ZWriteEnable = false;
		CullMode = None;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		BlendOp = Add;

		VertexShader = compile vs_3_0 Default_VS();
		PixelShader = compile ps_3_0 BgImage_PS();
	}
}

// Current element ID
int g_iElementID;
// Color of the current element
float4 g_vElementColor;

float4 MountImage_PS(VS_SCREEN In) : COLOR0
{
	float hoverFadeIn = g_fHoverFadeIns[g_iElementID];

	float mask = 1.f - tex2D(texElementImageSampler, In.UV).r;
	float shadow = 1.f - tex2D(texElementImageSampler, In.UV + 0.01f).r;
	
	float luma = dot(g_vElementColor.rgb, float3(0.2126, 0.7152, 0.0722));

	float3 faded_color = lerp(g_vElementColor.rgb, luma, 0.33f);

	float3 color = faded_color;
	float3 glow = 0;
	if(hoverFadeIn > 0.f)
	{
		color = lerp(faded_color, g_vElementColor.rgb, hoverFadeIn);

		float glow_mask = 0;
		glow_mask += 1.f - tex2D(texElementImageSampler, In.UV + float2(0.01f, 0.01f)).r;
		glow_mask += 1.f - tex2D(texElementImageSampler, In.UV + float2(-0.01f, 0.01f)).r;
		glow_mask += 1.f - tex2D(texElementImageSampler, In.UV + float2(0.01f, -0.01f)).r;
		glow_mask += 1.f - tex2D(texElementImageSampler, In.UV + float2(-0.01f, -0.01f)).r;

		glow = g_vElementColor.rgb * (glow_mask / 4) * hoverFadeIn * 0.5f * (0.5f + 0.5f * snoise(In.UV * 3.18f + 0.15f * float2(cos(g_fAnimationTimer * 3), sin(g_fAnimationTimer * 2))));
	}

	return float4(color * mask + glow, g_vElementColor.a * max(mask, shadow)) * g_fWheelFadeIn.x;
}

technique MountImage
{
	pass P0
	{
		ZEnable = false;
		ZWriteEnable = false;
		CullMode = None;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		BlendOp = Add;

		VertexShader = compile vs_3_0 Default_VS();
		PixelShader = compile ps_3_0 MountImage_PS();
	}
}

float4 Cursor_PS(VS_SCREEN In) : COLOR0
{
	float2 smoothrandom = makeSmoothRandom(In.UV, float4(15, 18, 18, 15), float4(2.4, 3.1, 2.9, 4.7));

	float4 baseImage = tex2D(texBgImageSampler, In.UV + smoothrandom * 0.003f);
	float radius = length(In.UV * 2 - 1);
	baseImage *= 1.25f * pow(1.f - smoothstep(0.f, 1.f, radius), 4.f);
	baseImage *= lerp(0.8f, 1.5f, saturate((4 + smoothrandom.x + smoothrandom.y) / 8));

	return baseImage;
}

technique Cursor
{
	pass P0
	{
		ZEnable = false;
		ZWriteEnable = false;
		CullMode = None;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		SrcBlend = One;
		DestBlend = One;
		BlendOp = Add;

		VertexShader = compile vs_3_0 Default_VS();
		PixelShader = compile ps_3_0 Cursor_PS();
	}
}
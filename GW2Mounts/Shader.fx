#pragma warning(disable : 3571)
#pragma warning(disable : 4717)
#define PI 3.14159f
#define SQRT2 1.4142136f
#define ONE_OVER_SQRT2 0.707107f
#include "perlin.hlsl"

shared float4 g_fScreenSize;

struct VS_SCREEN
{
	float4 Position : POSITION;
	float2 UV : TEXCOORD0;
};

texture texMountImage;

sampler2D texMountImageSampler =
sampler_state
{
    texture = <texMountImage>;
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

float4 g_vSpriteDimensions, g_vScreenSize;
float g_fFadeTimer;
float g_fTimer;
float g_fHoverTimer;
int3 g_iMountID;
int g_iMountCount;
float g_fDeadZoneScale;
bool g_bMountHovered;
int g_iMountHovered;
float4 g_vColor;
bool g_bCenterGlow;

float2 makeSmoothRandom(float2 uv, float4 scales, float4 timeScales)
{
	float smoothrandom1 = sin(scales.x * uv.x + g_fTimer * timeScales.x) + sin(scales.y * uv.y + g_fTimer * timeScales.y);
	float smoothrandom2 = sin(scales.z * uv.x + g_fTimer * timeScales.z) + sin(scales.w * uv.y + g_fTimer * timeScales.w);

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

float4 BgImage_PS(VS_SCREEN In) : COLOR0
{
	// Multiply by -3 rather than 2 to mirror and scale down
	float2 coords = -3 * (In.UV - 0.5f);
	// Compute polar coordinates with theta \in [0, 2pi)
	float2 coordsPolar = float2(length(coords), atan2(coords.y, coords.x) + PI);
	// Compensate for different theta = 0 direction
	coordsPolar.y += 0.5f * PI;
	
	// Angular span covered by a single mount (e.g. if 4 mounts are shown, the span is 90 degrees)
	float singleMountAngle = 2.f * PI / float(g_iMountCount);
    int localMountId = (int(coordsPolar.y / singleMountAngle + 0.5001f)) % g_iMountCount;
    bool isLocalMountHovered = localMountId == g_iMountHovered;
	// Percentage along the mount's angular span: 0 is one edge, 1 is the other
	// Must also compensate since the spans are centered, but we want to start at one edge
    float localCoordPercentage = fmod(coordsPolar.y + 0.5f * singleMountAngle, singleMountAngle) / singleMountAngle;
	
	// Generate a pseudorandom background
	float2 smoothrandom = float2(snoise(3 * In.UV * cos(0.1f * g_fTimer) + g_fTimer * 0.37f), snoise(5 * In.UV * sin(0.13f * g_fTimer) + g_fTimer * 0.48f));
	float4 color = tex2D(texBgImageSampler, In.UV + smoothrandom * 0.003f);
	color.rgb *= lerp(0.9f, 1.3f, saturate((4 + smoothrandom.x + smoothrandom.y) / 8));
    color.rgb *= lerp(0.5f, 1.f, isLocalMountHovered ? g_fHoverTimer : 0.f);
	// Compute luma value for desaturation effects
	float luma = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));

	// Fade out the background at the periphery of the circle
	float edge_mask = lerp(1.f, 0.f, smoothstep(0.5f, 1.f, coordsPolar.x * (1 - luma * 0.2f)));
	// Fade out the background in the dead zone at the center of the circle
	float center_mask = lerp(0.f, 1.f, smoothstep(g_fDeadZoneScale - 0.025f, g_fDeadZoneScale + 0.025f, coordsPolar.x * (1 - luma * 0.2f)));
	
	// Increase brightness on hovered and edge regions of the circle
	float border_mask = 1.f;

	// Calculate edge regions (localCoordPercentage is near 0 or 1), but only for non-hovered mount spans
    if (!isLocalMountHovered || g_fHoverTimer < 1)
	{
		float min_thickness = 0.003f / (0.001f + coordsPolar.x);
		float max_thickness = 0.005f / (0.001f + coordsPolar.x);
	
		if(g_iMountCount > 1)
		{
            border_mask *= lerp(2.f, 1.f, smoothstep(min_thickness, max_thickness, localCoordPercentage));
            border_mask *= lerp(1.f, 2.f, smoothstep(1 - max_thickness, 1 - min_thickness, localCoordPercentage));
        }
	}

	// Reduce brightening when starting to hover to fade in gracefully
    if (isLocalMountHovered && g_fHoverTimer < 1)
		border_mask = lerp(border_mask, 1.f, g_fHoverTimer);

	// Also brighten when the dead zone is hovered and has an action assigned to it
	if (g_bCenterGlow)
	{
        if (isLocalMountHovered)
			border_mask *= 1 - g_fHoverTimer;
		center_mask = lerp(center_mask, 1.f, g_fHoverTimer);
	}
	
	// Add some flair to the inner region of the circle
	border_mask *= lerp(2.f, 1.f, smoothstep(g_fDeadZoneScale, g_fDeadZoneScale + 0.05f, coordsPolar.x));
	border_mask *= lerp(1.5f, 1.f, smoothstep(g_fDeadZoneScale, min(0.5f, g_fDeadZoneScale * 4), coordsPolar.x));

	// Combine all masks, ensuring that the edge and center masks never increase brightness when combined and that the border mask never darkens the circle
	return color * saturate(edge_mask * center_mask) * clamp(border_mask, 1.f, 2.f) * clamp(luma, 0.8f, 1.2f) * g_fFadeTimer;
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

float4 MountImage_PS(VS_SCREEN In) : COLOR0
{
	float mask = 1.f - tex2D(texMountImageSampler, In.UV).r;
	float shadow = 1.f - tex2D(texMountImageSampler, In.UV + 0.01f).r;
	
	float luma = dot(g_vColor.rgb, float3(0.2126, 0.7152, 0.0722));

	float3 faded_color = lerp(g_vColor.rgb, luma, 0.33f);

	float3 color = faded_color;
	float3 glow = 0;
	if(g_bMountHovered)
	{
		color = lerp(faded_color, g_vColor.rgb, g_fHoverTimer);

		float glow_mask = 0;
		glow_mask += 1.f - tex2D(texMountImageSampler, In.UV + float2(0.01f, 0.01f)).r;
		glow_mask += 1.f - tex2D(texMountImageSampler, In.UV + float2(-0.01f, 0.01f)).r;
		glow_mask += 1.f - tex2D(texMountImageSampler, In.UV + float2(0.01f, -0.01f)).r;
		glow_mask += 1.f - tex2D(texMountImageSampler, In.UV + float2(-0.01f, -0.01f)).r;

		glow = g_vColor.rgb * (glow_mask / 4) * g_fHoverTimer * 0.5f * (0.5f + 0.5f * snoise(In.UV * 3.18f + 0.15f * float2(cos(g_fTimer * 3), sin(g_fTimer * 2))));
	}

	return float4(color * mask + glow, g_vColor.a * max(mask, shadow)) * g_fFadeTimer;
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
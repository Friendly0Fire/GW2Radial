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
int3 g_iMountID;
int g_iMountCount;

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

	float2 coords = 2 * (In.UV - 0.5f);
	float2 coordsPolar = float2(length(coords), atan2(coords.y, coords.x) - 0.5f * PI);
	if(coordsPolar.y < 0)
		coordsPolar.y += 2 * PI;
		
	float mountAngle = 2 * PI / g_iMountCount;
	float localCoordAngle = fmod(coordsPolar.y, mountAngle) / mountAngle;
		
	float2 smoothrandom = float2(snoise(3 * In.UV * cos(0.1f * g_fTimer) + g_fTimer * 0.37f), snoise(5 * In.UV * sin(0.13f * g_fTimer) + g_fTimer * 0.48f));

	float4 color = tex2D(texBgImageSampler, In.UV + smoothrandom * 0.003f);
	color.rgb *= lerp(0.9f, 1.3f, saturate((4 + smoothrandom.x + smoothrandom.y) / 8));
	color.rgb *= 0.5f;
	float luma = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));

	float edge_mask = 1.f;
	
	edge_mask *= lerp(0.f, 1.f, smoothstep(0.095f, 0.1f, coordsPolar.x * (1 - luma * 0.2f)));
	edge_mask *= lerp(1.f, 0.f, smoothstep(0.5f, 1.f, coordsPolar.x * (1 - luma * 0.2f)));

	float border_mask = 1.f;
	
	float min_thickness = 0.003f / (0.001f + coordsPolar.x);
	float max_thickness = 0.005f / (0.001f + coordsPolar.x);
	
	// FIXME: Only works for odd number of mounts
	border_mask *= lerp(2.f, 1.f, smoothstep(min_thickness, max_thickness, localCoordAngle));
	border_mask *= lerp(1.f, 2.f, smoothstep(1 - max_thickness, 1 - min_thickness, localCoordAngle));
	border_mask *= lerp(2.f, 1.f, smoothstep(0.105f, 0.11f, coordsPolar.x));
	border_mask *= lerp(1.5f, 1.f, smoothstep(0.105f, 0.4f, coordsPolar.x));

	return color * saturate(edge_mask) * clamp(border_mask, 1.f, 2.f) * clamp(luma, 0.8f, 1.2f) * g_fFadeTimer;
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

	float4 color = float4(g_iMountID.x == 0 || g_iMountID.x == 3 || g_iMountID.x == 5, g_iMountID.x == 1 || g_iMountID.x == 3 || g_iMountID.x == 4, g_iMountID.x == 2 || g_iMountID.x == 4 || g_iMountID.x == 5, 1);

	return color * mask * g_fFadeTimer;
}

/*float4 g_vDirection;

float4 MountImageHighlight_PS(VS_SCREEN In, uniform bool griffon) : COLOR0
{
	float4 baseImage = 0;
	float2 border = (In.UV * 2 - 1);

	float d1 = abs(dot(border, g_vDirection.xy));
	float d2 = abs(dot(border, g_vDirection.yx));

	if (griffon || ((dot(border, g_vDirection.xy)) > 0 && d1 > d2))
	{
		float smoothrandom1 = sin(14 * In.UV.x + g_vDirection.z * 1.3f) + sin(17 * In.UV.y + g_vDirection.z * 2.3f);
		float smoothrandom2 = sin(17 * In.UV.x + g_vDirection.z * 1.7f) + sin(14 * In.UV.y + g_vDirection.z * 3.1f);

		baseImage = tex2D(texMountImageSampler, In.UV + float2(smoothrandom1, smoothrandom2) * 0.003f);
		float radius = length(border);
		baseImage *= (1.f - smoothstep(0.6f, 1.f, radius)) * smoothstep(0.0f, 0.2f, radius);
		if (!griffon) baseImage *= 1 - 0.5f * saturate(abs(d1 - d2));
		baseImage *= lerp(0.8f, 1.1f, saturate((4 + smoothrandom1 + smoothrandom2) / 8));
	}
	return baseImage * g_fTimer;
}*/

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
#pragma warning(disable : 3571)
#pragma warning(disable : 4717)
#define PI 3.14159f
#define SQRT2 1.4142136f
#define ONE_OVER_SQRT2 0.707107f

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
    MipFilter = NONE;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

float4 g_vSpriteDimensions, g_vScreenSize;
float g_fTimer;

VS_SCREEN MountImage_VS(in float2 UV : TEXCOORD0)
{
    VS_SCREEN Out = (VS_SCREEN)0;

	float2 dims = (UV * 2 - 1) * g_vSpriteDimensions.zw;

    Out.UV = UV;
    Out.Position = float4(dims + g_vSpriteDimensions.xy * 2 - 1, 0.5f, 1.f);
	Out.Position.y *= -1;

    return Out;
}

float4 MountImage_PS(VS_SCREEN In) : COLOR0
{
	float4 baseImage = tex2D(texMountImageSampler, In.UV);
	return baseImage * g_fTimer;
}

float4 g_vDirection;

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

		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		BlendOp = Add;

		VertexShader = compile vs_3_0 MountImage_VS();
		PixelShader = compile ps_3_0 MountImage_PS();
	}
}

technique MountImageHighlight
{
	pass P0
	{
		ZEnable = false;
		ZWriteEnable = false;
		CullMode = None;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		BlendOp = Add;

		VertexShader = compile vs_3_0 MountImage_VS();
		PixelShader = compile ps_3_0 MountImageHighlight_PS(false);
	}
}

technique MountImageHighlightGriffon
{
	pass P0
	{
		ZEnable = false;
		ZWriteEnable = false;
		CullMode = None;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		BlendOp = Add;

		VertexShader = compile vs_3_0 MountImage_VS();
		PixelShader = compile ps_3_0 MountImageHighlight_PS(true);
	}
}

float4 Cursor_PS(VS_SCREEN In) : COLOR0
{
	float smoothrandom1 = sin(15 * In.UV.x + g_fTimer * 1.4f) + sin(18 * In.UV.y + g_fTimer * 2.1f);
	float smoothrandom2 = sin(18 * In.UV.x + g_fTimer * 1.9f) + sin(15 * In.UV.y + g_fTimer * 3.7f);

	float4 baseImage = tex2D(texMountImageSampler, In.UV + float2(smoothrandom1, smoothrandom2) * 0.003f);
	float radius = length(In.UV * 2 - 1);
	baseImage *= pow(1.f - smoothstep(0.1f, 1.f, radius), 4.f);
	baseImage *= lerp(0.8f, 1.1f, saturate((4 + smoothrandom1 + smoothrandom2) / 8));

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

		VertexShader = compile vs_3_0 MountImage_VS();
		PixelShader = compile ps_3_0 Cursor_PS();
	}
}
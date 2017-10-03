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
texture texPattern;

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

sampler2D texPatternSampler =
sampler_state
{
    texture = <texMountImage>;
    MipFilter = NONE;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
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
	return tex2D(texMountImageSampler, In.UV) * g_fTimer;
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
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

float4 g_vSpriteCenter, g_vScreenSize;

VS_SCREEN MountImage_VS(in float2 UV : TEXCOORD0)
{
    VS_SCREEN Out = (VS_SCREEN)0;

	float2 dims = (UV * 2 - 1) * 0.4f;
	dims.x *= g_vScreenSize.y * g_vScreenSize.z;

    Out.UV = UV;
    Out.Position = float4(dims + g_vSpriteCenter.xy * 2 - 1, 0.5f, 1.f);
	Out.Position.y *= -1;

    return Out;
}

float4 MountImage_PS(VS_SCREEN In) : COLOR0
{
    return float4(In.UV, 0, 1);
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

        VertexShader = compile vs_3_0 MountImage_VS();
        PixelShader = compile ps_3_0 MountImage_PS();
    }
}
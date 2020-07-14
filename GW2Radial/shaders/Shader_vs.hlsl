#pragma warning(disable : 4717)
#define SHADER_VS
#include "registers.h"

struct VS_SCREEN
{
	float4 Position : POSITION;
	float2 UV : TEXCOORD0;
};

VS_SCREEN ScreenQuad_VS(in float2 UV : TEXCOORD0)
{
    VS_SCREEN Out = (VS_SCREEN)0;

	float2 dims = (UV * 2 - 1) * fSpriteDimensions.zw;

    Out.UV = UV;
    Out.Position = float4(dims + fSpriteDimensions.xy * 2 - 1, 0.5f, 1.f);
	Out.Position.y *= -1;

    return Out;
}
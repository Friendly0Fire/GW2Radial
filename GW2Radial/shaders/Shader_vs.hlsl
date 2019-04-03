#pragma warning(disable : 4717)

struct VS_SCREEN
{
	float4 Position : POSITION;
	float2 UV : TEXCOORD0;
};

// Current rendered sprite (wheel or wheel element) dimensions as ([0..1] position x, [0..1] position y, scaled size x, scaled size y)
float4 g_vSpriteDimensions : register (c0);

VS_SCREEN main(in float2 UV : TEXCOORD0)
{
    VS_SCREEN Out = (VS_SCREEN)0;

	float2 dims = (UV * 2 - 1) * g_vSpriteDimensions.zw;

    Out.UV = UV;
    Out.Position = float4(dims + g_vSpriteDimensions.xy * 2 - 1, 0.5f, 1.f);
	Out.Position.y *= -1;

    return Out;
}
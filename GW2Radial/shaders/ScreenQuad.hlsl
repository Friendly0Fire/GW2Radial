cbuffer VS : register(b0)
{
	float4 spriteDimensions;
	float4x4 tiltMatrix;
	float spriteZ;
};

struct VS_SCREEN
{
	float4 Position : SV_Position;
	float2 UV : TEXCOORD0;
};

VS_SCREEN ScreenQuad(in u32  id : SV_VertexID)
{
    VS_SCREEN Out = (VS_SCREEN)0;

	float2 UV = float2(id & 1, id >> 1);

	float2 dims = (UV * 2 - 1) * spriteDimensions.zw;

    Out.UV = UV;
    Out.Position = mul(float4(dims + spriteDimensions.xy * 2 - 1, spriteZ, 1.f), tiltMatrix);
	Out.Position.z += saturate(0.5f - spriteZ);
	Out.Position.y *= -1;

    return Out;
}
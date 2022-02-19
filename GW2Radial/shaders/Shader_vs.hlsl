struct VS_SCREEN
{
	float4 Position : POSITION;
	float2 UV : TEXCOORD0;
};

VS_SCREEN ScreenQuad_VS(in uint id : SV_VertexID)
{
    VS_SCREEN Out = (VS_SCREEN)0;

	float2 UV = float2(id & 1, id >> 1);

	float2 dims = (UV * 2 - 1) * fSpriteDimensions.zw;

    Out.UV = UV;
    Out.Position = float4(dims + fSpriteDimensions.xy * 2 - 1, 0.5f, 1.f);
	Out.Position.y *= -1;

    return Out;
}
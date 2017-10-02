#pragma warning(disable : 3571)
#pragma warning(disable : 4717)
#define PI 3.14159f
#define SQRT2 1.4142136f
#define ONE_OVER_SQRT2 0.707107f

shared float4 g_fScreenSize;

struct VS_SCREEN
{
	float4 Position : POSITION;   // vertex position 
	float2 vTexCoord0 : TEXCOORD0;  // vertex texture coords 
};

texture texMountImage;

sampler2D texMountImageSampler =
sampler_state
{
	texture = <texMountImage>;
	MipFilter = NONE;
	MinFilter = LINEAR;
	MagFilter = LINEAR;

};

float4 MountImage_PS(VS_SCREEN In) : COLOR0
{
	return 1;
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

		PixelShader = compile ps_3_0 MountImage_PS();
	}
}
#include "Effect.h"
#include "Utility.h"

namespace GW2Radial {

Effect::Effect(IDirect3DDevice9 * iDev)
{
	dev = iDev;

	iDev->CreateStateBlock(D3DSBT_ALL, &sb);

	ps = NULL;
	vs = NULL;
}

Effect::~Effect()
{
}

int Effect::Load()
{
	void* code;	
	size_t sz;

	if (LoadFontResource(IDR_SHADER_PS, code, sz))
		dev->CreatePixelShader((DWORD*)code, &ps);

	if (LoadFontResource(IDR_SHADER_VS, code, sz))
		dev->CreateVertexShader((DWORD*)code, &vs);

	return (ps && vs);
}

void Effect::SetTechnique(EffectTechnique val)
{
	SetFloat(EFF_VS_TECH_ID, val * 1.0f);

	switch (val)
	{
		case EFF_TC_BGIMAGE:
		case EFF_TC_MOUNTIMAGE:
			dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);			
			dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			break;
		case EFF_TC_MOUNTIMAGE_ALPHABLEND:			
			dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);			
			dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			break;
		case EFF_TC_CURSOR:
			dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
	}
}

void Effect::SetVector(EffectVarSlot slot, fVector4 * vec)
{
	SetVarToSlot(slot, (float*)vec, 16);
}

void Effect::SetValue(EffectVarSlot slot, void * val, int sz)
{
	SetVarToSlot(slot, (float*)val, sz);
}

void Effect::SetFloat(EffectVarSlot slot, float fv)
{
	SetVarToSlot(slot, &fv, 4);
}

static float floatArrayFiller[12 * 4] = { 0 };

void Effect::SetFloatArray(EffectVarSlot slot, float * fv, int sz)
{	
	for (int i = 0; i != sz; ++i)
		floatArrayFiller[i * 4] = fv[i];

	SetVarToSlot(slot, floatArrayFiller, sz*16);
}

void Effect::SetTexture(EffectTextureSlot slot, IDirect3DTexture9 * val)
{
	dev->SetTexture(slot, val);
}

void Effect::SceneBegin()
{
	sb->Capture();

	dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	dev->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
	dev->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
	dev->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	dev->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	dev->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	dev->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	dev->SetRenderState(D3DRS_ZENABLE, 0);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, 0);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	dev->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

	dev->SetPixelShader(ps);
	dev->SetVertexShader(vs);
}

void Effect::SceneEnd()
{
	sb->Apply();
}

void Effect::BeginPass(int whatever)
{

}

void Effect::Begin(uint * pass, int whatever)
{
}

void Effect::EndPass()
{
}

void Effect::End()
{
}

void Effect::SetVarToSlot(EffectVarSlot slot, float* mem, int sz)
{
	static int tgtType[] = {
		1,//EFF_VS_INK_SPOT,
		1,//EFF_VS_ELEMENT_COUNT,
		1,//EFF_VS_CENTER_SCALE,
		1,//EFF_VS_ANIM_TIMER,
		1,//EFF_VS_WHEEL_FADEIN,
		0,//EFF_VS_SPRITE_DIM,
		1,//EFF_VS_HOVER_FADEINS,
		2,//EFF_VS_SCREEN_SIZE,
		1,//EFF_VS_ELEMENT_ID,
		1,//EFF_VS_ELEMENT_COLOR,
		1,//EFF_VS_TECH_ID
	};

	static int tgtReg[] = {
		4,//EFF_VS_INK_SPOT,
		3,//EFF_VS_ELEMENT_COUNT,
		2,//EFF_VS_CENTER_SCALE,
		0,//EFF_VS_ANIM_TIMER,
		1,//EFF_VS_WHEEL_FADEIN,
		0,//EFF_VS_SPRITE_DIM,
		8,//EFF_VS_HOVER_FADEINS,
	   -1,//EFF_VS_SCREEN_SIZE,
		5,//EFF_VS_ELEMENT_ID,
		6,//EFF_VS_ELEMENT_COLOR,
		7,//EFF_VS_TECH_ID
	};

	float fv4t[4];

	if (sz < 16)
	{
		sz = sz / 4;
		for (int i = 0; i != sz; ++i)
			fv4t[i] = mem[i];
		mem = fv4t;
		sz = 1;
	}
	else if (sz & 0xC)
	{
		int rem = (sz & 0xC) >> 2;
		sz = sz >> 2;

		for (int i = 0; i != rem; ++i)
			fv4t[i] = mem[i+sz];

		sz = sz >> 2;

		if (tgtType[slot] == 0)
			dev->SetVertexShaderConstantF(tgtReg[slot]+sz, fv4t, 1);
		else if (tgtType[slot] == 1)
			dev->SetPixelShaderConstantF(tgtReg[slot]+sz, fv4t, 1);
	} else 
		sz = sz >> 4;

	if (tgtType[slot] == 0)
		dev->SetVertexShaderConstantF(tgtReg[slot], (float*)mem, sz);
	else if (tgtType[slot] == 1)
		dev->SetPixelShaderConstantF(tgtReg[slot], (float*)mem, sz);
}

}
#pragma once

#include "Main.h"

namespace GW2Radial {

typedef enum EffectTechnique {
	EFF_TC_BGIMAGE = 4,
	EFF_TC_MOUNTIMAGE_ALPHABLEND = 3,
	EFF_TC_MOUNTIMAGE = 2,
	EFF_TC_CURSOR = 1
} EffectTechnique;

typedef enum EffectVarSlot {
	EFF_VS_WIPE_MASK_DATA = 0,
	EFF_VS_ELEMENT_COUNT = 1,
	EFF_VS_CENTER_SCALE,
	EFF_VS_ANIM_TIMER,
	EFF_VS_WHEEL_FADEIN,
	EFF_VS_SPRITE_DIM,
	EFF_VS_HOVER_FADEINS,
	EFF_VS_SCREEN_SIZE,
	EFF_VS_ELEMENT_ID,
	EFF_VS_ELEMENT_COLOR,
	EFF_VS_TECH_ID
} EffectVarSlot;

typedef enum EffectTextureSlot {
	EFF_TS_BG = 0,
	EFF_TS_WIPE_MASK = 1,
	EFF_TS_ELEMENTIMG
} EffectTextureSlot;

class Effect
{
public:
	Effect(IDirect3DDevice9* iDev);
	~Effect();

	virtual int Load();

	virtual void SetTechnique(EffectTechnique val);
	void SetVector(EffectVarSlot slot, fVector4* vec);
	void SetValue(EffectVarSlot slot, void* val, int sz);
	void SetFloat(EffectVarSlot slot, float fv);
	void SetFloatArray(EffectVarSlot slot, float* fv, int sz);

	virtual void SetTexture(EffectTextureSlot slot, IDirect3DTexture9* val);

	virtual void SceneBegin(void* drawBuf);
	virtual void SceneEnd();

	virtual void BeginPass(int whatever);
	virtual void Begin(uint* pass, int whatever);
	virtual void EndPass();
	virtual void End();

	void SetVarToSlot(EffectVarSlot slot, float* mem, int sz);

protected:
	IDirect3DDevice9* dev;
	IDirect3DPixelShader9* ps;
	IDirect3DVertexShader9* vs;

private:	
	IDirect3DStateBlock9* sb;
};

}
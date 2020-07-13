#pragma once
#include "Effect.h"

namespace GW2Radial {

class Effect_dx12 : public Effect
{
public:
	Effect_dx12(IDirect3DDevice9* iDev);
	~Effect_dx12();

	int Load();

	void SetTechnique(EffectTechnique val);

	void SetTexture(EffectTextureSlot slot, IDirect3DTexture9* val);

	void SceneBegin(void* drawBuf);
	void SceneEnd();

private:
	static void* PSO_bgImg;
	static void* PSO_mountAlpha;
	static void* PSO_cursor;

	DWORD tsSamplers[4] = { 0,0,0,0 };
	DWORD tsTexId[4] = { 0,0,0,0 };

	DWORD** perTechPSO[5];
};

};
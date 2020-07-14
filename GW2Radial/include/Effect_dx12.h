#pragma once
#include "Effect.h"

namespace GW2Radial {

struct PSOTag_ {};
using PSOTag = PSOTag_*;

struct SamplerId { DWORD id = 0; };
struct TextureId { DWORD id = 0; };

class Effect_dx12 : public Effect
{
public:
	using Effect::Effect;

	void SetTexture(uint slot, IDirect3DTexture9* val) override;
	void SetRenderStates(std::initializer_list<ShaderState> states) override;
	void SetSamplerStates(uint slot, std::initializer_list<ShaderState> states) override;
	void ApplyStates() override;

	void Begin() override;
	void OnBind(IDirect3DVertexDeclaration9* vd) override;
	void End() override;

	void Clear() override;

private:
	IDirect3DVertexDeclaration9* currentVertexDecl_ = nullptr;

	std::map<uint, PSOTag> cachedPSOs_;
	std::map<uint, SamplerId> cachedSamplers_;
	
	SamplerId samplers_[4];
	TextureId textures_[4];

	uint rsHash_;
};

};
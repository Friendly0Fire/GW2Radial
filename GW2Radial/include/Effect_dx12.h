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
    Effect_dx12(IDirect3DDevice9* dev);

	void SetTexture(uint slot, IDirect3DTexture9* val) override;
	void SetRenderStates(std::initializer_list<ShaderState> states) override;
	void SetSamplerStates(uint slot, std::initializer_list<ShaderState> states) override;
	void ApplyStates() override;

	void Begin() override;
	void OnBind(IDirect3DVertexDeclaration9* vd) override;
	void End() override;

	void Clear() override;

private:
	void ApplyPixelShader(IDirect3DPixelShader9* ps) override;
	void ApplyVertexShader(IDirect3DVertexShader9* vs) override;
	void ResetStates();

	IDirect3DVertexDeclaration9* currentVertexDecl_ = nullptr;
	IDirect3DPixelShader9* currentPS_ = nullptr;
	IDirect3DVertexShader9* currentVS_ = nullptr;

	std::map<uint, PSOTag> cachedPSOs_;
	std::map<uint, SamplerId> cachedSamplers_;
	
	std::vector<SamplerId> samplers_;
	std::vector<TextureId> textures_;
	int maxSamplerSlot_ = -1;
	int maxTextureSlot_ = -1;

	std::vector<ShaderState> renderStates_;
};

};
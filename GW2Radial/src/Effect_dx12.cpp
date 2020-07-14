#include <Effect_dx12.h>
#include <UnitQuad.h>
#include <xxhash/xxhash.h>

//D3D9 API extenders =======================

#define D3DRS_ENABLE_D912PXY_API_HACKS (D3DRENDERSTATETYPE)220
#define D3DRS_D912PXY_ENQUEUE_PSO_COMPILE (D3DRENDERSTATETYPE)221
#define D3DRS_D912PXY_SETUP_PSO (D3DRENDERSTATETYPE)222
#define D3DRS_D912PXY_GPU_WRITE (D3DRENDERSTATETYPE)223
#define D3DRS_D912PXY_DRAW (D3DRENDERSTATETYPE)224
#define D3DRS_D912PXY_SAMPLER_ID (D3DRENDERSTATETYPE)225

#define D3DDECLMETHOD_PER_VERTEX_CONSTANT 8
#define D3DUSAGE_D912PXY_FORCE_RT 0x0F000000L

#define D912PXY_ENCODE_GPU_WRITE_DSC(sz, offset) ((sz & 0xFFFF) | ((offset & 0xFFFF) << 16))

#define D912PXY_GPU_WRITE_OFFSET_TEXBIND 0
#define D912PXY_GPU_WRITE_OFFSET_SAMPLER 8 
#define D912PXY_GPU_WRITE_OFFSET_VS_VARS 16
#define D912PXY_GPU_WRITE_OFFSET_PS_VARS 16 + 256

//========

namespace GW2Radial {
	
void Effect_dx12::SetRenderStates(std::initializer_list<ShaderState> states)
{
	rsHash_ = 0;
	for(const auto& s : states)
	{
		device_->SetRenderState(static_cast<D3DRENDERSTATETYPE>(s.stateId), s.stateValue);
	    rsHash_ = XXH32(&s, sizeof(ShaderState), rsHash_);
	}
}
	
void Effect_dx12::SetSamplerStates(uint slot, std::initializer_list<ShaderState> states)
{
	uint hash = 0;
	for(const auto& s : states)
	{
		device_->SetSamplerState(0, static_cast<D3DSAMPLERSTATETYPE>(s.stateId), s.stateValue);
	    hash = XXH32(&s, sizeof(ShaderState), hash);
	}
	
	auto it = cachedSamplers_.find(hash);
	if(it == cachedSamplers_.end())
	{
		SamplerId samp;
	    assert(SUCCEEDED(device_->GetRenderState(D3DRS_D912PXY_SAMPLER_ID, &samp.id)));
		it = cachedSamplers_.insert({ hash, samp }).first;
	}

	samplers_[slot] = it->second;
}

void Effect_dx12::SetTexture(uint slot, IDirect3DTexture9* val)
{
	// megai2: this will get us texture id
	textures_[slot] = { val->GetPriority() };
}

void Effect_dx12::OnBind(IDirect3DVertexDeclaration9* vd)
{
    currentVertexDecl_ = vd;
}


void Effect_dx12::Begin()
{
	// megai2: mark draw start so we can see that app is issuing some not default dx9 api approach
	device_->SetRenderState(D3DRS_D912PXY_DRAW, 0);

	for(uint i = 0; i < std::size(samplers_); i++)
	{
	    textures_[i] = { 0 };
	    samplers_[i] = { 0 };
	}

	currentVertexDecl_ = nullptr;
	
	SetDefaults();
}

void Effect_dx12::ApplyStates()
{
	uint64_t hashData[] = {
	    reinterpret_cast<uint64_t>(currentVertexDecl_),
	    reinterpret_cast<uint64_t>(currentPS_),
	    reinterpret_cast<uint64_t>(currentVS_)
	};
	uint psoHash = XXH32(hashData, sizeof(hashData), rsHash_);

	auto it = cachedPSOs_.find(psoHash);
	if(it == cachedPSOs_.end())
	{
		PSOTag pso = nullptr;
		assert(SUCCEEDED(device_->GetRenderState(D3DRS_D912PXY_ENQUEUE_PSO_COMPILE, reinterpret_cast<DWORD*>(&pso))));
		it = cachedPSOs_.insert({ psoHash, pso }).first;
	}
	
	device_->GetRenderState(D3DRS_D912PXY_SETUP_PSO, reinterpret_cast<DWORD*>(it->second));
	
	device_->SetRenderState(D3DRS_D912PXY_GPU_WRITE, D912PXY_ENCODE_GPU_WRITE_DSC(1, D912PXY_GPU_WRITE_OFFSET_SAMPLER));
	device_->GetRenderState(D3DRS_D912PXY_GPU_WRITE, &samplers_[0].id);
	
	device_->SetRenderState(D3DRS_D912PXY_GPU_WRITE, D912PXY_ENCODE_GPU_WRITE_DSC(1, D912PXY_GPU_WRITE_OFFSET_TEXBIND));
	device_->GetRenderState(D3DRS_D912PXY_GPU_WRITE, &textures_[0].id);
}


void Effect_dx12::End()
{
	// megai2: update dirty flags so we transfer to dx9 mode safely
	device_->SetRenderState(D3DRS_D912PXY_DRAW, 0x701);
	device_->SetRenderState(D3DRS_D912PXY_SETUP_PSO, 0);
}

void Effect_dx12::Clear()
{
    Effect::Clear();

	cachedPSOs_.clear();
	cachedSamplers_.clear();

	for(uint i = 0; i < std::size(samplers_); i++)
	{
	    textures_[i] = { 0 };
	    samplers_[i] = { 0 };
	}

	currentVertexDecl_ = nullptr;
}


}
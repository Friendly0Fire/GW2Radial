#include <Effect_dx12.h>
#include <UnitQuad.h>

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

void* Effect_dx12::PSO_bgImg = 0;
void* Effect_dx12::PSO_cursor = 0;
void* Effect_dx12::PSO_mountAlpha = 0;

Effect_dx12::Effect_dx12(IDirect3DDevice9 * iDev) : Effect(iDev)
{

}

Effect_dx12::~Effect_dx12()
{

}

int Effect_dx12::Load()
{
	if (!Effect::Load())
		return 0;

	//megai2: prepare pso for drawing
	IDirect3DVertexDeclaration9* vdcl = NULL;
	dev->CreateVertexDeclaration(UnitQuad::def(), &vdcl);

	dev->SetPixelShader(ps);
	dev->SetVertexShader(vs);
	dev->SetVertexDeclaration(vdcl);

	dev->SetRenderState(D3DRS_ZENABLE, 0);
	dev->SetRenderState(D3DRS_ZWRITEENABLE, 0);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	dev->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

	dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);

	PSO_bgImg = 0;
	if (dev->GetRenderState(D3DRS_D912PXY_ENQUEUE_PSO_COMPILE, (DWORD*)&PSO_bgImg) < 0)
		return 0;

	dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);

	PSO_mountAlpha = 0;
	if (dev->GetRenderState(D3DRS_D912PXY_ENQUEUE_PSO_COMPILE, (DWORD*)&PSO_mountAlpha) < 0)
		return 0;
			
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	PSO_cursor = 0;
	if (dev->GetRenderState(D3DRS_D912PXY_ENQUEUE_PSO_COMPILE, (DWORD*)&PSO_cursor) < 0)
		return 0;

	vdcl->Release();

	//megai2: set and save desired sampler
	dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	dev->GetRenderState(D3DRS_D912PXY_SAMPLER_ID, &tsSamplers[0]);

	dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	dev->GetRenderState(D3DRS_D912PXY_SAMPLER_ID, &tsSamplers[1]);

	dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	dev->GetRenderState(D3DRS_D912PXY_SAMPLER_ID, &tsSamplers[2]);

	perTechPSO[EFF_TC_BGIMAGE] = (DWORD**)&PSO_bgImg;
	perTechPSO[EFF_TC_MOUNTIMAGE] = (DWORD**)&PSO_bgImg;
	perTechPSO[EFF_TC_CURSOR] = (DWORD**)&PSO_cursor;
	perTechPSO[EFF_TC_MOUNTIMAGE_ALPHABLEND] = (DWORD**)&PSO_mountAlpha;
	   
	return true;
}

void Effect_dx12::SetTechnique(EffectTechnique val)
{
	SetVariable(true, EFF_VS_TECH_ID, val);

	DWORD* pso = *perTechPSO[val];

	if (pso)
		dev->GetRenderState(D3DRS_D912PXY_SETUP_PSO, pso);
}

void Effect_dx12::SetTexture(EffectTextureSlot slot, IDirect3DTexture9 * val)
{
	//megai2: this will get us texture id 
	tsTexId[slot] = val->GetPriority();

	dev->GetRenderState(D3DRS_D912PXY_GPU_WRITE, &tsTexId[0]);	
}

void Effect_dx12::SceneBegin()
{
	//megai2: mark draw start so we can see that app is issuing some not default dx9 api approach
	dev->SetRenderState(D3DRS_D912PXY_DRAW, 0);

	//setup saved sampler by writing directly into gpu buffer
	dev->SetRenderState(D3DRS_D912PXY_GPU_WRITE, D912PXY_ENCODE_GPU_WRITE_DSC(1, D912PXY_GPU_WRITE_OFFSET_SAMPLER));
	dev->GetRenderState(D3DRS_D912PXY_GPU_WRITE, &tsSamplers[0]);

	//prepare to write texture id for draws
	dev->SetRenderState(D3DRS_D912PXY_GPU_WRITE, D912PXY_ENCODE_GPU_WRITE_DSC(1, D912PXY_GPU_WRITE_OFFSET_TEXBIND));
}

void Effect_dx12::SceneEnd()
{
	//megai2: update dirty flags so we transfer to dx9 mode safely
	dev->SetRenderState(D3DRS_D912PXY_DRAW, 0x701);
	dev->SetRenderState(D3DRS_D912PXY_SETUP_PSO, 0);
}

}
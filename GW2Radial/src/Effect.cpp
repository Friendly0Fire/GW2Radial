#include "Effect.h"
#include "Utility.h"
#include <UnitQuad.h>
#include <assert.h>

#ifdef HOT_RELOAD_SHADERS
#include <fstream>
#include <d3dcompiler.h>
#endif

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
	COM_RELEASE(ps);
	COM_RELEASE(vs);
	COM_RELEASE(sb);
}

void Effect::CompileShader(std::wstring filename, bool isPixelShader, std::vector<byte>& data) {
#ifdef HOT_RELOAD_SHADERS
	filename = SHADERS_DIR + filename;
	ID3DBlob* blob = nullptr;
	while (blob == nullptr) {
		blob = nullptr;
		ID3DBlob* errors = nullptr;
		auto hr = D3DCompileFromFile(filename.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", isPixelShader ? "ps_3_0" : "vs_3_0", D3DCOMPILE_IEEE_STRICTNESS, 0, &blob, &errors);

		if (FAILED(hr)) {
			FormattedOutputDebugString(L"Compilation failed: %x\n", hr);

			if (errors) {
				auto errorsText = (const char*)errors->GetBufferPointer();
				auto errorsTextLength = errors->GetBufferSize();

				OutputDebugStringA("Compilation errors:\n");
				OutputDebugStringA(errorsText);

				errors->Release();
			}

			if (blob)
				blob->Release();
			blob = nullptr;

			// Break to fix errors
			assert(false);
		}
	}

	data.resize(blob->GetBufferSize() + 1);
	std::copy((byte*)blob->GetBufferPointer(), (byte*)blob->GetBufferPointer() + blob->GetBufferSize(), data.data());
	data.back() = 0;

	blob->Release();
#endif
}

int Effect::Load()
{
#ifdef HOT_RELOAD_SHADERS
	std::vector<byte> data;

	COM_RELEASE(ps);
	CompileShader(L"Shader_ps.hlsl", true, data);
	assert(SUCCEEDED(dev->CreatePixelShader((DWORD*)data.data(), &ps)));

	COM_RELEASE(vs);
	CompileShader(L"Shader_vs.hlsl", false, data);
	assert(SUCCEEDED(dev->CreateVertexShader((DWORD*)data.data(), &vs)));
#else
	void* code;
	size_t sz;

	if (LoadResource(IDR_SHADER_PS, code, sz))
		dev->CreatePixelShader((DWORD*)code, &ps);

	if (LoadResource(IDR_SHADER_VS, code, sz))
		dev->CreateVertexShader((DWORD*)code, &vs);
#endif

	return (ps && vs);
}

void Effect::SetTechnique(EffectTechnique val)
{
	SetVariable(true, EFF_VS_TECH_ID, val);

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

void Effect::SetTexture(EffectTextureSlot slot, IDirect3DTexture9 * val)
{
	dev->SetTexture(slot, val);
}

void Effect::SceneBegin(void* drawBuf)
{
	sb->Capture();

	//megai2: FIXME set UnitQuad* type to method parameter and make it compile
	((UnitQuad*)drawBuf)->Bind();

	dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	dev->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	dev->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
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

}
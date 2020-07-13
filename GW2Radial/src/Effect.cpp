#include "Effect.h"
#include "Utility.h"
#include <UnitQuad.h>
#include <assert.h>


#if defined(_DEBUG) && defined(SHADERS_DIR)
#define HOT_RELOAD_SHADERS
#endif

#ifdef HOT_RELOAD_SHADERS
#include <fstream>
#include <d3dcompiler.h>
#endif

namespace GW2Radial {

Effect::Effect(IDirect3DDevice9 * dev) : device_(dev)
{
	device_->CreateStateBlock(D3DSBT_ALL, &stateBlock_);
}

Effect::~Effect()
{
	for (auto& ps : pixelShaders_)
		ps.second->Release();
	for (auto& vs : vertexShaders_)
		vs.second->Release();
}

void Effect::CompileShader(const ShaderType st, const std::string& entrypoint, std::vector<byte>& data) const {
	const std::wstring filename = st == ShaderType::PIXEL_SHADER ? SHADERS_DIR L"Shader_ps.hlsl" : SHADERS_DIR L"Shader_vs.hlsl";

	ID3DBlob* blob = nullptr;
	while (blob == nullptr) {
		ID3DBlob* errors = nullptr;
        const auto hr = D3DCompileFromFile(filename.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(), st == ShaderType::PIXEL_SHADER ? "ps_3_0" : "vs_3_0", D3DCOMPILE_IEEE_STRICTNESS, 0, &blob, &errors);

		if (FAILED(hr)) {
			FormattedOutputDebugString(L"Compilation failed: %x\n", hr);

			if (errors) {
                const char* errorsText = static_cast<const char*>(errors->GetBufferPointer());
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
	std::copy(static_cast<byte*>(blob->GetBufferPointer()), static_cast<byte*>(blob->GetBufferPointer()) + blob->GetBufferSize(), data.data());
	data.back() = 0;

	blob->Release();
}

void Effect::SetShaders(const std::string& entrypointPS, const std::string& entrypointVS)
{
	auto itPs = pixelShaders_.find(entrypointPS);
	if (itPs == pixelShaders_.end())
	{
		IDirect3DPixelShader9* ps;

		std::vector<byte> data;
		CompileShader(ShaderType::PIXEL_SHADER, entrypointPS, data);
		if(SUCCEEDED(device_->CreatePixelShader(reinterpret_cast<DWORD*>(data.data()), &ps)))
			itPs = pixelShaders_.insert({ entrypointPS, ps }).first;
	}

	auto itVs = vertexShaders_.find(entrypointPS);
	if (itVs == vertexShaders_.end()) {
		IDirect3DVertexShader9* vs;

		std::vector<byte> data;
		CompileShader(ShaderType::VERTEX_SHADER, entrypointVS, data);
		if (SUCCEEDED(device_->CreateVertexShader(reinterpret_cast<DWORD*>(data.data()), &vs)))
			itVs = vertexShaders_.insert({ entrypointVS, vs }).first;
	}

	assert(itPs != pixelShaders_.end() && itVs != vertexShaders_.end());

	device_->SetPixelShader(itPs->second);
	device_->SetVertexShader(itVs->second);

#ifdef FIXME
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
#endif
}

void Effect::SetTexture(EffectTextureSlot slot, IDirect3DTexture9 * val)
{
	device_->SetTexture(slot, val);
}

void Effect::SceneBegin()
{
	stateBlock_->Capture();

	device_->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	device_->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	device_->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	device_->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	device_->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	device_->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	device_->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	device_->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	device_->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	device_->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	device_->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	device_->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	device_->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	device_->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	device_->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	device_->SetRenderState(D3DRS_ZENABLE, 0);
	device_->SetRenderState(D3DRS_ZWRITEENABLE, 0);
	device_->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	device_->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	device_->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	device_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
}

void Effect::SceneEnd()
{
	stateBlock_->Apply();
}

}
#include "Effect.h"
#include <assert.h>
#include <fstream>
#include <sstream>
#include <UnitQuad.h>
#include "Utility.h"

#ifdef HOT_RELOAD_SHADERS
#include <d3dcompiler.h>
#endif

namespace GW2Radial {

class ShaderInclude final : public ID3DInclude
{
	Effect* parent_ = nullptr;
	std::map<LPCVOID, std::string> openFiles_;

public:
    virtual ~ShaderInclude() = default;
    ShaderInclude(Effect* parent) : parent_(parent) {}

    HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData,
        UINT* pBytes) override
	{
	    auto file = parent_->shadersZip_->GetEntry(pFileName);
		if(!file)
			return E_INVALIDARG;

		auto* const contentStream = file->GetDecompressionStream();
	    if(contentStream)
	    {
			auto str = ReadFile(*contentStream);
		    file->CloseDecompressionStream();
			*ppData = str.c_str();
			*pBytes = str.size();

			openFiles_[str.c_str()] = std::move(str);
		    return S_OK;
	    }

		return E_FAIL;
	}

    HRESULT Close(LPCVOID pData) override
	{
	    openFiles_.erase(pData);
		return S_OK;
	}
};

Effect::Effect(IDirect3DDevice9 * dev) : device_(dev)
{
	device_->CreateStateBlock(D3DSBT_ALL, &stateBlock_);
}

[[nodiscard]] std::string Effect::LoadShaderFile(const std::wstring& filename)
{
#ifdef HOT_RELOAD_SHADERS
	const std::wstring fullPath = SHADERS_DIR + filename;
	std::ifstream file(fullPath);
	return ReadFile(file);
#else
	if(!shadersZip_)
	{
		const auto data = LoadResource(IDR_SHADERS);

		auto* const iss = new std::istringstream(std::string(reinterpret_cast<char*>(data.data()), data.size()), std::ios_base::binary);

		shadersZip_ = ZipArchive::Create(iss, true);

		shaderIncludeManager_ = std::make_unique<ShaderInclude>(this);
		shaderIncludeManagerPtr_ = shaderIncludeManager_.get();
	}

	auto file = shadersZip_->GetEntry(utf8_encode(filename));
    auto* const contentStream = file->GetDecompressionStream();
	if(contentStream)
	{
	    auto fileContents = ReadFile(*contentStream);
		file->CloseDecompressionStream();
		return fileContents;
	}

	return "";

#endif
}

[[nodiscard]] std::variant<IDirect3DPixelShader9*, IDirect3DVertexShader9*> Effect::CompileShader(const std::wstring& filename, ShaderType st, const std::string& entrypoint) {
	ComPtr<ID3DBlob> blob = nullptr;
	while (blob == nullptr) {
		auto shaderContents = LoadShaderFile(filename);

#ifdef HOT_RELOAD_SHADERS
		std::string filePath = utf8_encode(SHADERS_DIR + filename);
#else
		std::string filePath = utf8_encode(filename);
#endif

		ComPtr<ID3DBlob> errors = nullptr;
        const auto hr = D3DCompile(
			shaderContents.data(),
			shaderContents.size(),
			filePath.c_str(),
			nullptr,
			shaderIncludeManagerPtr_,
			entrypoint.c_str(), st == ShaderType::PIXEL_SHADER ? "ps_3_0" : "vs_3_0", 
			D3DCOMPILE_IEEE_STRICTNESS, 0,
			&blob, &errors);

		if (FAILED(hr)) {
#ifdef HOT_RELOAD_SHADERS
			FormattedOutputDebugString(L"Compilation failed: %x\n", hr);

			if (errors) {
                const char* errorsText = static_cast<const char*>(errors->GetBufferPointer());
				auto errorsTextLength = errors->GetBufferSize();

				OutputDebugStringA("Compilation errors:\n");
				OutputDebugStringA(errorsText);
			}

			blob.Reset();

			// Break to fix errors
			assert(false);
#else
			exit(1);
#endif
		}
	}

	if(st == ShaderType::PIXEL_SHADER)
	{
	    IDirect3DPixelShader9* ps = nullptr;
	    device_->CreatePixelShader(static_cast<DWORD*>(blob->GetBufferPointer()), &ps);
		return ps;
	} else {
	    IDirect3DVertexShader9* vs = nullptr;
	    device_->CreateVertexShader(static_cast<DWORD*>(blob->GetBufferPointer()), &vs);
		return vs;
	}
}

void Effect::SetShader(const ShaderType st, const std::wstring& filename, const std::string& entrypoint)
{
	std::string key = utf8_encode(filename) + "::" + entrypoint;

	if(st == ShaderType::PIXEL_SHADER)
	{
	    auto itPs = pixelShaders_.find(key);
	    if (itPs == pixelShaders_.end())
	    {
		    IDirect3DPixelShader9* ps = std::get<IDirect3DPixelShader9*>(CompileShader(filename, st, entrypoint));
			itPs = pixelShaders_.insert({key, ps}).first;
	    }
		
	    assert(itPs != pixelShaders_.end());
		currentPS_ = itPs->second.Get();
	    device_->SetPixelShader(itPs->second.Get());
	} else {
	    auto itVs = vertexShaders_.find(key);
	    if (itVs == vertexShaders_.end())
	    {
		    IDirect3DVertexShader9* vs = std::get<IDirect3DVertexShader9*>(CompileShader(filename, st, entrypoint));
			itVs = vertexShaders_.insert({key, vs}).first;
	    }
		
	    assert(itVs != vertexShaders_.end());
		currentVS_ = itVs->second.Get();
	    device_->SetVertexShader(itVs->second.Get());
	}
}

	
void Effect::SetRenderStates(std::initializer_list<ShaderState> states)
{
	SetDefaultRenderStates();

	for(const auto& s : states)
		device_->SetRenderState(static_cast<D3DRENDERSTATETYPE>(s.stateId), s.stateValue);
}
	
void Effect::SetSamplerStates(uint slot, std::initializer_list<ShaderState> states)
{
	SetDefaultSamplerStates(slot);

	for(const auto& s : states)
		device_->SetSamplerState(slot, static_cast<D3DSAMPLERSTATETYPE>(s.stateId), s.stateValue);
}

void Effect::SetTexture(uint slot, IDirect3DTexture9* val)
{
	device_->SetTexture(slot, val);
}

void Effect::Begin()
{
	// Save current device state to reapply at the end
	stateBlock_->Capture();
}

void Effect::SetDefaultSamplerStates(uint slot) const
{
	device_->SetSamplerState(slot, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	device_->SetSamplerState(slot, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	device_->SetSamplerState(slot, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	device_->SetSamplerState(slot, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	device_->SetSamplerState(slot, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}

void Effect::SetDefaultRenderStates() const
{
    // Reset render states to most common overlay-style defaults
	device_->SetRenderState(D3DRS_ZENABLE, false);
	device_->SetRenderState(D3DRS_ZWRITEENABLE, false);
	device_->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	device_->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	device_->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	device_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
}

void Effect::End()
{
	// Restore prior device state
	stateBlock_->Apply();
}

void Effect::Clear()
{
#ifdef HOT_RELOAD_SHADERS
    currentPS_ = nullptr;
	currentVS_ = nullptr;

	pixelShaders_.clear();
	vertexShaders_.clear();
#endif
}


}
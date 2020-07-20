#include "Effect.h"
#include <fstream>
#include <sstream>
#include <UnitQuad.h>
#include "Utility.h"
#include <d3dcompiler.h>

#include "FileSystem.h"

namespace GW2Radial {

class ShaderInclude final : public ID3DInclude
{
	Effect* parent_ = nullptr;
	std::map<LPCVOID, std::vector<byte>> openFiles_;

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
			auto data = FileSystem::ReadFile(*contentStream);
		    file->CloseDecompressionStream();
			*ppData = data.data();
			*pBytes = UINT(data.size());

			openFiles_[data.data()] = std::move(data);
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
	auto vec = FileSystem::ReadFile(file);
	return std::string(reinterpret_cast<char*>(vec.data()), vec.size());
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
	if(file) {
        auto* const contentStream = file->GetDecompressionStream();
	    if(contentStream) {
	        auto vec = FileSystem::ReadFile(*contentStream);
		    file->CloseDecompressionStream();
	        return std::string(reinterpret_cast<char*>(vec.data()), vec.size());
	    }
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
			0, 0,
			&blob, &errors);

		if (FAILED(hr)) {
#ifdef _DEBUG
			FormattedOutputDebugString(L"Compilation failed: 0x%X\n", hr);

			if (errors) {
                const char* errorsText = static_cast<const char*>(errors->GetBufferPointer());
				auto errorsTextLength = errors->GetBufferSize();

				FormattedOutputDebugString("Compilation errors:\n%s", errorsText);
			}

			blob.Reset();

			// Break to fix errors
			GW2_ASSERT(false);
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
	
void Effect::ApplyPixelShader(IDirect3DPixelShader9* ps)
{
	device_->SetPixelShader(ps);
}
void Effect::ApplyVertexShader(IDirect3DVertexShader9* vs)
{
	device_->SetVertexShader(vs);
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
		
	    GW2_ASSERT(itPs != pixelShaders_.end());
	    ApplyPixelShader(itPs->second.Get());
	} else {
	    auto itVs = vertexShaders_.find(key);
	    if (itVs == vertexShaders_.end())
	    {
		    IDirect3DVertexShader9* vs = std::get<IDirect3DVertexShader9*>(CompileShader(filename, st, entrypoint));
			itVs = vertexShaders_.insert({key, vs}).first;
	    }
		
	    GW2_ASSERT(itVs != vertexShaders_.end());
	    ApplyVertexShader(itVs->second.Get());
	}
}

	
void Effect::SetRenderStates(std::initializer_list<ShaderState> states)
{
	SetDefaultRenderStates();

	for(cref s : states)
		device_->SetRenderState(static_cast<D3DRENDERSTATETYPE>(s.stateId), s.stateValue);
}

void Effect::SetSamplerStates(uint slot, std::initializer_list<ShaderState> states)
{
	SetDefaultSamplerStates(slot);

	for(cref s : states)
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

void Effect::OnBind(IDirect3DVertexDeclaration9* vd)
{
	device_->SetVertexDeclaration(vd);
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
	pixelShaders_.clear();
	vertexShaders_.clear();
#endif
}


}

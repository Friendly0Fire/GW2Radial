#include "Effect.h"
#include <fstream>
#include <sstream>
#include <UnitQuad.h>
#include "Utility.h"
#include <d3dcompiler.h>

#include "FileSystem.h"

namespace GW2Radial {

class ShaderInclude final : public ID3DInclude {
    Effect*                              parent_ = nullptr;
    std::map<LPCVOID, std::vector<byte>> openFiles_;

public:
    virtual  ~ShaderInclude() = default;
    explicit ShaderInclude(Effect* parent) : parent_(parent) {}

    HRESULT COM_DECLSPEC_NOTHROW Open(
        [[maybe_unused]] D3D_INCLUDE_TYPE includeType,
        LPCSTR                            pFileName,
        [[maybe_unused]] LPCVOID          pParentData,
        LPCVOID*                          ppData,
        UINT*                             pBytes) override {
        auto file = parent_->shadersZip_->GetEntry(pFileName);
        if (!file)
            return E_INVALIDARG;

        auto* const contentStream = file->GetDecompressionStream();
        if (contentStream) {
            auto data = FileSystem::ReadFile(*contentStream);
            file->CloseDecompressionStream();
            *ppData = data.data();
            *pBytes = UINT(data.size());

            openFiles_[data.data()] = std::move(data);
            return S_OK;
        }

        return E_FAIL;
    }

    HRESULT COM_DECLSPEC_NOTHROW Close(LPCVOID pData) override {
        openFiles_.erase(pData);
        return S_OK;
    }
};

Effect::Effect(IDirect3DDevice9* dev)
    : device_(dev) {
    device_->CreateStateBlock(D3DSBT_ALL, &stateBlock_);
    CheckHotReload();
}

[[nodiscard]] std::string Effect::LoadShaderFile(const std::wstring& filename) {
    LoadShadersArchive();

    if(hotReloadFolderExists_ && FileSystem::Exists(GetShaderFilename(filename))) {
        std::ifstream      file(GetShaderFilename(filename));
        auto               vec = FileSystem::ReadFile(file);
        return std::string(reinterpret_cast<char*>(vec.data()), vec.size());
    } else {
        auto file = shadersZip_->GetEntry(EncodeShaderFilename(filename));
        GW2_ASSERT(file != nullptr);
        auto* const contentStream = file->GetDecompressionStream();
        GW2_ASSERT(contentStream != nullptr);
        
        auto vec = FileSystem::ReadFile(*contentStream);
        file->CloseDecompressionStream();
        return std::string(reinterpret_cast<char*>(vec.data()), vec.size());
    }
}

void HandleFailedShaderCompile(HRESULT hr, ID3DBlob* errors) {
    if(SUCCEEDED(hr))
        return;

#ifndef _DEBUG
    CriticalMessageBox(L"Fatal error: shader compilation failed! Error code was 0x%X. Please report this to https://github.com/Friendly0Fire/GW2Radial/issues", hr);
#endif

    FormattedOutputDebugString(L"Compilation failed: 0x%X\n", hr);

    if (errors) {
        const char* errorsText = static_cast<const char*>(errors->GetBufferPointer());

        FormattedOutputDebugString("Compilation errors:\n%s", errorsText);

        // Break to fix errors
        GW2_ASSERT(errors != nullptr);
    }
}

[[nodiscard]] std::variant<ComPtr<IDirect3DPixelShader9>, ComPtr<IDirect3DVertexShader9>> Effect::CompileShader(
    const std::wstring& filename, ShaderType st, const std::string& entrypoint) {
    ComPtr<ID3DBlob> blob = nullptr;
    while (blob == nullptr) {
        auto shaderContents = LoadShaderFile(filename);
        std::string filePath = EncodeShaderFilename(filename);
        auto* includePtr = GetIncludeManager();

        ComPtr<ID3DBlob> errors = nullptr;
        const auto       hr     = D3DCompile(
                                             shaderContents.data(),
                                             shaderContents.size(),
                                             filePath.c_str(),
                                             nullptr,
                                             includePtr,
                                             entrypoint.c_str(),
                                             st == ShaderType::PIXEL_SHADER ? "ps_3_0" : "vs_3_0",
                                             0, 0,
                                             &blob, &errors);

        HandleFailedShaderCompile(hr, errors.Get());
        errors.Reset();
    }

    if (st == ShaderType::PIXEL_SHADER) {
        ComPtr<IDirect3DPixelShader9> ps;
        device_->CreatePixelShader(static_cast<DWORD*>(blob->GetBufferPointer()), &ps);
        return ps;
    } else {
        ComPtr<IDirect3DVertexShader9> vs;
        device_->CreateVertexShader(static_cast<DWORD*>(blob->GetBufferPointer()), &vs);
        return vs;
    }
}

void Effect::ApplyPixelShader(IDirect3DPixelShader9* ps) {
    device_->SetPixelShader(ps);
}

void Effect::ApplyVertexShader(IDirect3DVertexShader9* vs) {
    device_->SetVertexShader(vs);
}

void Effect::LoadShadersArchive() {
    if (shadersZip_)
        return;

    const auto data = LoadResource(IDR_SHADERS);

    auto* const iss = new std::istringstream(
                                             std::string(reinterpret_cast<char*>(data.data()), data.size()),
                                             std::ios_base::binary);

    shadersZip_ = ZipArchive::Create(iss, true);

    shaderIncludeManager_    = std::make_unique<ShaderInclude>(this);
}

std::wstring Effect::GetShaderFilename(const std::wstring& filename) const {
#ifdef HOT_RELOAD_SHADERS
    if (hotReloadFolderExists_)
        return SHADERS_DIR + filename;
#endif
    return filename;
}

ID3DInclude* Effect::GetIncludeManager() const {
#ifdef HOT_RELOAD_SHADERS
    if (hotReloadFolderExists_)
        return D3D_COMPILE_STANDARD_FILE_INCLUDE;
#endif
    return shaderIncludeManager_.get();
}

void Effect::CheckHotReload() {
#ifdef HOT_RELOAD_SHADERS
    hotReloadFolderExists_ = std::filesystem::exists(SHADERS_DIR);
#endif
}

void Effect::SetShader(const ShaderType st, const std::wstring& filename, const std::string& entrypoint) {
    std::string key = utf8_encode(filename) + "::" + entrypoint;

    if (st == ShaderType::PIXEL_SHADER) {
        auto itPs = pixelShaders_.find(key);
        if (itPs == pixelShaders_.end()) {
            auto ps = std::get<ComPtr<IDirect3DPixelShader9>>(CompileShader(filename, st, entrypoint));
            itPs    = pixelShaders_.insert({key, ps}).first;
        }

        GW2_ASSERT(itPs != pixelShaders_.end());
        ApplyPixelShader(itPs->second.Get());
    } else {
        auto itVs = vertexShaders_.find(key);
        if (itVs == vertexShaders_.end()) {
            auto vs = std::get<ComPtr<IDirect3DVertexShader9>>(CompileShader(filename, st, entrypoint));
            itVs    = vertexShaders_.insert({key, vs}).first;
        }

        GW2_ASSERT(itVs != vertexShaders_.end());
        ApplyVertexShader(itVs->second.Get());
    }
}


void Effect::SetRenderStates(std::initializer_list<ShaderState> states) {
    SetDefaultRenderStates();

    for (cref s : states)
        device_->SetRenderState(static_cast<D3DRENDERSTATETYPE>(s.stateId), s.stateValue);
}

void Effect::SetSamplerStates(uint slot, std::initializer_list<ShaderState> states) {
    SetDefaultSamplerStates(slot);

    for (cref s : states)
        device_->SetSamplerState(slot, static_cast<D3DSAMPLERSTATETYPE>(s.stateId), s.stateValue);
}

void Effect::SetTexture(uint slot, IDirect3DTexture9* val) {
    device_->SetTexture(slot, val);
}

void Effect::Begin() {
    // Save current device state to reapply at the end
    stateBlock_->Capture();
}

void Effect::OnBind(IDirect3DVertexDeclaration9* vd) {
    device_->SetVertexDeclaration(vd);
}

void Effect::SetDefaultSamplerStates(uint slot) const {
    device_->SetSamplerState(slot, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    device_->SetSamplerState(slot, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    device_->SetSamplerState(slot, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    device_->SetSamplerState(slot, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    device_->SetSamplerState(slot, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}

void Effect::SetDefaultRenderStates() const {
    // Reset render states to most common overlay-style defaults
    device_->SetRenderState(D3DRS_ZENABLE, false);
    device_->SetRenderState(D3DRS_ZWRITEENABLE, false);
    device_->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device_->SetRenderState(D3DRS_ALPHATESTENABLE, false);
    device_->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    device_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
}

void Effect::End() {
    // Restore prior device state
    stateBlock_->Apply();
}

void Effect::Clear() {
#ifdef HOT_RELOAD_SHADERS
    pixelShaders_.clear();
    vertexShaders_.clear();
#endif
}


}

#include "ShaderManager.h"
#include <fstream>
#include <sstream>
#include "Utility.h"
#include <d3dcompiler.h>

#include "FileSystem.h"

namespace GW2Radial {

class ShaderInclude final : public ID3DInclude {
    std::map<LPCVOID, std::vector<byte>> openFiles_;

public:
    virtual  ~ShaderInclude() = default;
    ShaderInclude() = default;

    HRESULT COM_DECLSPEC_NOTHROW Open(
        [[maybe_unused]] D3D_INCLUDE_TYPE includeType,
        LPCSTR                            pFileName,
        [[maybe_unused]] LPCVOID          pParentData,
        LPCVOID*                          ppData,
        UINT*                             pBytes) override {
        auto file = ShaderManager::i().shadersZip_->GetEntry(pFileName);
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

ShaderManager::ShaderManager(ID3D11Device* dev)
    : device_(dev) {
    CheckHotReload();
    device_->GetImmediateContext(&context_);
}

void ShaderManager::SetShaders(ShaderId vs, ShaderId ps)
{
    context_->PSSetShader(std::get<ComPtr<ID3D11PixelShader>>(shaders_[ps.id].shader).Get(), nullptr, 0);
    context_->VSSetShader(std::get<ComPtr<ID3D11VertexShader>>(shaders_[vs.id].shader).Get(), nullptr, 0);
}

ShaderId ShaderManager::GetShader(const std::wstring& filename, D3D11_SHADER_VERSION_TYPE st, const std::string& entrypoint)
{
    for (uint i = 0; i < shaders_.size(); i++)
    {
        auto& sd = shaders_[i];
        if (sd.filename == filename && sd.entrypoint == entrypoint)
            return { i };
    }

    auto shader = CompileShader(filename, st, entrypoint);
    uint id = uint(shaders_.size());
    shaders_.push_back({
        .shader = shader,
        .filename = filename,
        .st = st,
        .entrypoint = entrypoint
        });

    return { id };
}

void ShaderManager::ReloadAll()
{
#ifdef HOT_RELOAD_SHADERS
    for (auto& sd : shaders_)
        sd.shader = CompileShader(sd.filename, sd.st, sd.entrypoint);
#endif
}

[[nodiscard]] std::string ShaderManager::LoadShaderFile(const std::wstring& filename) {
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

ComPtr<ID3D11Buffer> ShaderManager::MakeConstantBuffer(size_t dataSize, const void* data)
{
    D3D11_BUFFER_DESC desc {
        .ByteWidth = uint(dataSize),
        .Usage = D3D11_USAGE_DYNAMIC,
        .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
        .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
        .MiscFlags = 0,
        .StructureByteStride = 0
    };
    D3D11_SUBRESOURCE_DATA idata
    {
        .pSysMem = data,
        .SysMemPitch = uint(dataSize),
        .SysMemSlicePitch = 0
    };
    ComPtr<ID3D11Buffer> buf;
    device_->CreateBuffer(&desc, data ? &idata : nullptr, &buf);

    return buf;
}

void HandleFailedShaderCompile(HRESULT hr, ID3DBlob* errors) {
    if(SUCCEEDED(hr))
        return;

    Log::i().Print(Severity::Error, L"Compilation failed: 0x{:x}", uint(hr));

    if (errors) {
        const char* errorsText = static_cast<const char*>(errors->GetBufferPointer());

        Log::i().Print(Severity::Error, "Compilation errors:\n{}", errorsText);
    }

#ifndef _DEBUG
    if (hr == 0x88760b59)
        CriticalMessageBox(L"Fatal error: outdated shader compiler. Please install Windows Update KB4019990 or upgrade to a more modern operating system.");
    else
        CriticalMessageBox(L"Fatal error: shader compilation failed! Error code was 0x%X. Please report this to https://github.com/Friendly0Fire/GW2Radial/issues", hr);
#endif

    // Break to fix errors
    GW2_ASSERT(errors == nullptr);
}

[[nodiscard]] ShaderManager::AnyShaderComPtr ShaderManager::CompileShader(
    const std::wstring& filename, D3D11_SHADER_VERSION_TYPE st, const std::string& entrypoint) {
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
                                             st == D3D11_SHVER_PIXEL_SHADER ? "ps_4_0" : "vs_4_0",
                                             0, 0,
                                             &blob, &errors);

        HandleFailedShaderCompile(hr, errors.Get());
        errors.Reset();
    }

    if (st == D3D11_SHVER_PIXEL_SHADER) {
        ComPtr<ID3D11PixelShader> ps;
        device_->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &ps);
        return ps;
    } else {
        ComPtr<ID3D11VertexShader> vs;
        device_->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vs);
        return vs;
    }
}

void ShaderManager::LoadShadersArchive() {
    if (shadersZip_)
        return;

    const auto data = LoadResource(IDR_SHADERS);

    auto* const iss = new std::istringstream(
                                             std::string(reinterpret_cast<char*>(data.data()), data.size()),
                                             std::ios_base::binary);

    shadersZip_ = ZipArchive::Create(iss, true);

    shaderIncludeManager_ = std::make_unique<ShaderInclude>();
}

std::wstring ShaderManager::GetShaderFilename(const std::wstring& filename) const {
#ifdef HOT_RELOAD_SHADERS
    if (hotReloadFolderExists_)
        return SHADERS_DIR + filename;
#endif
    return filename;
}

ID3DInclude* ShaderManager::GetIncludeManager() const {
#ifdef HOT_RELOAD_SHADERS
    if (hotReloadFolderExists_)
        return D3D_COMPILE_STANDARD_FILE_INCLUDE;
#endif
    return shaderIncludeManager_.get();
}

void ShaderManager::CheckHotReload() {
#ifdef HOT_RELOAD_SHADERS
    hotReloadFolderExists_ = std::filesystem::exists(SHADERS_DIR);
#endif
}

void ConstantBufferBase::Upload(void* data, size_t size)
{
    D3D11_MAPPED_SUBRESOURCE map;
    ctx->Map(buf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
    memcpy_s(map.pData, size, data, size);
    ctx->Unmap(buf.Get(), 0);
}

}

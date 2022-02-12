#pragma once

#include <Main.h>
#include <d3dcompiler.h>
#include <Utility.h>
#include <span>
#include <map>
#include <variant>
#include <ZipArchive.h>
#include <d3d11.h>

namespace GW2Radial {

class ShaderId
{
	ShaderId() : id(std::numeric_limits<uint>::max()) {}
	ShaderId(uint i) : id(i) {}
	uint id;

	friend class ShaderManager;
};

class ConstantBufferBase
{
	ConstantBufferBase(ComPtr<ID3D11DeviceContext> ctx, ComPtr<ID3D11Buffer> buf) : ctx(ctx), buf(buf) {}
	ComPtr<ID3D11Buffer> buf;
	ComPtr<ID3D11DeviceContext> ctx;

	friend class ShaderManager;

protected:
	void Upload(void* data, size_t size);

public:
	ConstantBufferBase() = default;
	bool IsValid() const { return buf != nullptr; }
};

template<typename T>
class ConstantBuffer : public ConstantBufferBase
{
	ConstantBuffer(ComPtr<ID3D11DeviceContext> ctx, ComPtr<ID3D11Buffer> buf, std::optional<const T&> data) : ConstantBufferBase(ctx, buf)
	{
		if (data.has_value())
			this->data = *data;
	}

	friend class ShaderManager;
	T data;

public:
	ConstantBuffer() = default;

	ConstantBuffer(const ConstantBuffer&) = delete;
	ConstantBuffer(ConstantBuffer&&) = default;
	ConstantBuffer& operator=(const ConstantBuffer&) = delete;
	ConstantBuffer& operator=(ConstantBuffer&&) = default;

	T* operator->() { return &data; }
	void Update()
	{
		Upload(&data, sizeof(T));
	}
};

class ShaderManager : public Singleton<ShaderManager, false>
{
public:
	using AnyShaderComPtr = std::variant<ComPtr<ID3D11VertexShader>, ComPtr<ID3D11PixelShader>>;

    ShaderManager(ID3D11Device* dev);

	void SetShaders(ShaderId vs, ShaderId ps);
	ShaderId GetShader(const std::wstring& filename, D3D11_SHADER_VERSION_TYPE st, const std::string& entrypoint);

	template<typename T>
	ConstantBuffer<T> MakeConstantBuffer(std::optional<const T&> data = std::nullopt_t)
	{
		auto buf = MakeConstantBuffer(sizeof(T), data.has_value() ? &data : nullptr);
		return { context_, buf, data };
	}

	template<typename... Args>
	void SetConstantBuffers(Args& ...cbs)
	{
		auto getbuf = [](auto& cb) { return cb.buf.get(); };
		ID3D11Buffer* cbPtrs[] = { getbuf(cbs)... };
		context_->VSSetConstantBuffers(0, sizeof...(cbs), cbPtrs);
		context_->PSSetConstantBuffers(0, sizeof...(cbs), cbPtrs);
	}

	void ReloadAll();

protected:
	[[nodiscard]] std::string LoadShaderFile(const std::wstring& filename);

	ComPtr<ID3D11Buffer> MakeConstantBuffer(size_t dataSize, const void* data);

	void LoadShadersArchive();

	[[nodiscard]] AnyShaderComPtr CompileShader(const std::wstring& filename, D3D11_SHADER_VERSION_TYPE st, const std::string& entrypoint);

	[[nodiscard]] std::wstring GetShaderFilename(const std::wstring& filename) const;
    [[nodiscard]] std::string EncodeShaderFilename(const std::wstring& filename) const {
        return utf8_encode(GetShaderFilename(filename));
    }
    [[nodiscard]] ID3DInclude* GetIncludeManager() const;
	void CheckHotReload();

	bool hotReloadFolderExists_ = false;

	ComPtr<ID3D11Device> device_;
	ComPtr<ID3D11DeviceContext> context_;

	struct ShaderData
	{
		AnyShaderComPtr shader;

		std::wstring filename;
		D3D11_SHADER_VERSION_TYPE st;
		std::string entrypoint;
	};
	std::vector<ShaderData> shaders_;

	ZipArchive::Ptr shadersZip_;
	std::unique_ptr<ID3DInclude> shaderIncludeManager_;

	friend class ShaderInclude;
};

}

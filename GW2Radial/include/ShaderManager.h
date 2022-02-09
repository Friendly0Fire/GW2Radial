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

class ShaderIdx
{
	ShaderIdx() : id(std::numeric_limits<uint>::max()) {}
	ShaderIdx(uint i) : id(i) {}
	uint id;
	friend class ShaderManager;
};

class ShaderManager : public Singleton<ShaderManager, false>
{
public:
	using AnyShaderComPtr = std::variant<ComPtr<ID3D11VertexShader>, ComPtr<ID3D11PixelShader>>;

    ShaderManager(ID3D11Device* dev);

	void SetShaders(ShaderIdx vs, ShaderIdx ps);
	ShaderIdx GetShader(const std::wstring& filename, D3D11_SHADER_VERSION_TYPE st, const std::string& entrypoint);

	void ReloadAll();

protected:
	[[nodiscard]] std::string LoadShaderFile(const std::wstring& filename);

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

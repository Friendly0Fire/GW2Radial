#pragma once

#include <d3dcompiler.h>
#include <Main.h>
#include <Utility.h>
#include <span>
#include <map>
#include <variant>
#include <ZipArchive.h>

namespace GW2Radial {

enum class ShaderType {
	VERTEX_SHADER = 0,
	PIXEL_SHADER = 1
};

struct ShaderState
{
    uint stateId;
	uint stateValue;
};

class Effect
{
public:
	Effect(IDirect3DDevice9* dev);

	virtual void SetShader(ShaderType st, const std::wstring& filename, const std::string& entrypoint = "main");

	template<typename T>
	void SetVariable(ShaderType st, uint slot, const T& val) {
		if constexpr (std::is_same_v<T, fVector4> || std::is_same_v<T, iVector4>) {
			SetVariableInternal(st, slot, val);
		} else {
			auto v = ConvertToVector4(val);
			SetVariableInternal(st, slot, v);
		}
	}
	template<typename T>
	void SetVariableArray(ShaderType st, uint slot, const std::span<T>& arr) {
		if constexpr (std::is_same_v<T, fVector4> || std::is_same_v<T, iVector4>) {
			SetVariableArrayInternal(st, slot, arr);
		} else {
			using vector_t = decltype(ConvertToVector4(arr[0]));
			std::vector<vector_t> varr(arr.size());
			std::transform(arr.begin(), arr.end(), varr.begin(), [](const auto& val) {
				return ConvertToVector4(val);
			});
			SetVariableArrayInternal(st, slot, static_cast<const std::span<vector_t>&>(varr));
		}
	}

	virtual void SetTexture(uint slot, IDirect3DTexture9* val);
	virtual void SetRenderStates(std::initializer_list<ShaderState> states);
	virtual void SetSamplerStates(uint slot, std::initializer_list<ShaderState> states);
	virtual void ApplyStates() {}

	virtual void Begin();
	virtual void OnBind(IDirect3DVertexDeclaration9* vd) {}
	virtual void End();

	virtual void Clear();

protected:
	template<typename T>
	void SetVariableInternal(ShaderType st, uint slot, const T& val) {
		static_assert(std::is_same_v<T, fVector4>);

		if (st == ShaderType::PIXEL_SHADER)
			device_->SetPixelShaderConstantF(slot, reinterpret_cast<const float*>(&val), 1);
		else
			device_->SetVertexShaderConstantF(slot, reinterpret_cast<const float*>(&val), 1);
	}

	template<typename T>
	void SetVariableArrayInternal(ShaderType st, uint slot, const std::span<T>& arr) {
		static_assert(std::is_same_v<T, fVector4>);

		if (st == ShaderType::PIXEL_SHADER)
			device_->SetPixelShaderConstantF(slot, reinterpret_cast<const float*>(arr.data()), uint(arr.size()));
		else
			device_->SetVertexShaderConstantF(slot, reinterpret_cast<const float*>(arr.data()), uint(arr.size()));
	}

	[[nodiscard]] std::string LoadShaderFile(const std::wstring& filename);
	[[nodiscard]] std::variant<IDirect3DPixelShader9*, IDirect3DVertexShader9*> CompileShader(const std::wstring& filename, ShaderType st, const std::string& entrypoint);

	void SetDefaults();

	IDirect3DDevice9* device_;

	IDirect3DStateBlock9* stateBlock_;

	std::map<std::string, ComPtr<IDirect3DPixelShader9>> pixelShaders_;
	std::map<std::string, ComPtr<IDirect3DVertexShader9>> vertexShaders_;
	
	IDirect3DPixelShader9* currentPS_ = nullptr;
	IDirect3DVertexShader9* currentVS_ = nullptr;

	ZipArchive::Ptr shadersZip_;
	std::unique_ptr<ID3DInclude> shaderIncludeManager_;
	// This is used because unique_ptr would break with the standard include being a bad pointer
	ID3DInclude* shaderIncludeManagerPtr_ = D3D_COMPILE_STANDARD_FILE_INCLUDE;

	friend class ShaderInclude;
};

}

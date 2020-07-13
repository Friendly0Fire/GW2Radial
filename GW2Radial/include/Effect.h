#pragma once

#include <Main.h>
#include <Utility.h>
#include <span>
#include <map>

namespace GW2Radial {

typedef enum EffectTechnique {
	EFF_TC_BGIMAGE = 4,
	EFF_TC_MOUNTIMAGE_ALPHABLEND = 3,
	EFF_TC_MOUNTIMAGE = 2,
	EFF_TC_CURSOR = 1
} EffectTechnique;

typedef enum EffectVarSlot {
	EFF_VS_SPRITE_DIM = 0,
	EFF_VS_ANIM_TIMER = 0,
	EFF_VS_WHEEL_FADEIN = 1,
	EFF_VS_CENTER_SCALE = 2,
	EFF_VS_ELEMENT_COUNT = 3,
	EFF_VS_WIPE_MASK_DATA = 4,
	EFF_VS_ELEMENT_ID = 5,
	EFF_VS_ELEMENT_COLOR = 6,
	EFF_VS_TECH_ID = 7,
	EFF_VS_HOVER_FADEINS = 8,
} EffectVarSlot;

typedef enum EffectTextureSlot {
	EFF_TS_BG = 0,
	EFF_TS_WIPE_MASK = 1,
	EFF_TS_ELEMENTIMG
} EffectTextureSlot;

enum class ShaderType {
	VERTEX_SHADER = 0,
	PIXEL_SHADER = 1
};

class Effect
{
public:
	Effect(IDirect3DDevice9* dev);
	~Effect();

	virtual int Load();

	virtual void SetShaders(const std::string& entrypointPS, const std::string& entrypointVS);

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
			SetVariableArrayInternal(st, slot, (const std::span<vector_t>&)varr);
		}
	}

	virtual void SetTexture(EffectTextureSlot slot, IDirect3DTexture9* val);

	virtual void SceneBegin();
	virtual void SceneEnd();

protected:
	template<typename T>
	void SetVariableInternal(ShaderType st, uint slot, const T& val) {
		static_assert(std::is_same_v<T, fVector4>);

		if (st == ShaderType::PIXEL_SHADER)
			dev->SetPixelShaderConstantF(slot, reinterpret_cast<const float*>(&val), 1);
		else
			dev->SetVertexShaderConstantF(slot, reinterpret_cast<const float*>(&val), 1);
	}

	template<typename T>
	void SetVariableArrayInternal(ShaderType st, uint slot, const std::span<T>& arr) {
		static_assert(std::is_same_v<T, fVector4>);

		if (st == ShaderType::PIXEL_SHADER)
			dev->SetPixelShaderConstantF(slot, reinterpret_cast<const float*>(arr.data()), uint(arr.size()));
		else
			dev->SetVertexShaderConstantF(slot, reinterpret_cast<const float*>(arr.data()), uint(arr.size()));
	}

	void CompileShader(ShaderType st, const std::string& entrypoint, std::vector<byte>& data) const;

	IDirect3DDevice9* device_;

private:	
	IDirect3DStateBlock9* stateBlock_;

	std::map<std::string, IDirect3DPixelShader9*> pixelShaders_;
	std::map<std::string, IDirect3DVertexShader9*> vertexShaders_;
};

}
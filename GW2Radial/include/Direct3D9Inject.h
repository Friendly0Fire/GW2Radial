#pragma once

#include <Main.h>
#include <xxhash/xxhash.h>
#include <d3d9.h>
#include <functional>
#include <Singleton.h>

namespace GW2Radial
{

class Direct3D9Inject : public Singleton<Direct3D9Inject, false>
{
public:
	using DrawCallback = std::function<void(IDirect3DDevice9*, bool, bool)>;
	using PreResetCallback = std::function<void()>;
	using PostResetCallback = std::function<void(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*)>;
	using PreCreateDeviceCallback = std::function<void(HWND)>;
	using PostCreateDeviceCallback = std::function<void(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*)>;

	typedef IDirect3D9* (WINAPI *Direct3DCreate9_t)(UINT sdkVersion);
	typedef HRESULT (WINAPI *Direct3DCreate9Ex_t)(UINT sdkVersion, IDirect3D9Ex** output);

	DrawCallback drawUnderCallback, drawOverCallback;
	PreResetCallback preResetCallback;
	PostResetCallback postResetCallback;
	PreCreateDeviceCallback preCreateDeviceCallback;
	PostCreateDeviceCallback postCreateDeviceCallback;

protected:
	Direct3D9Inject() = default;

	const XXH64_hash_t preUiVertexShaderHash_ = 0x1fe3c6cd77e6e9f0;
	const XXH64_hash_t preUiPixelShaderHash_ = 0xccc38027cdd6cd51;
	IDirect3DVertexShader9* preUiVertexShader_ = nullptr;
	IDirect3DPixelShader9* preUiPixelShader_ = nullptr;

	bool isFrameDrawn_ = false;
	bool isInShaderHook_ = false;	
	bool isDirect3DHooked_ = false;
};

}

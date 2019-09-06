#pragma once

#include <Main.h>
#include <xxhash/xxhash.h>
#include <d3d9.h>
#include <functional>
#include <Singleton.h>

namespace GW2Radial
{

class Direct3D9Hooks : public Singleton<Direct3D9Hooks>
{
public:
	using DrawCallback = std::function<void(IDirect3DDevice9*, bool, bool)>;
	using PreResetCallback = std::function<void()>;
	using PostResetCallback = std::function<void(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*)>;
	using PreCreateDeviceCallback = std::function<void(HWND)>;
	using PostCreateDeviceCallback = std::function<void(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*)>;

	typedef IDirect3D9* (WINAPI *Direct3DCreate9_t)(UINT sdkVersion);
	typedef HRESULT (WINAPI *Direct3DCreate9Ex_t)(UINT sdkVersion, IDirect3D9Ex** output);
	
	Direct3D9Hooks();


	const DrawCallback & drawUnderCallback() const { return drawUnderCallback_; }
	void drawUnderCallback(const DrawCallback &drawUnderCallback) { drawUnderCallback_ = drawUnderCallback; }

	const DrawCallback & drawOverCallback() const { return drawOverCallback_; }
	void drawOverCallback(const DrawCallback &drawOverCallback) { drawOverCallback_ = drawOverCallback; }

	const PreResetCallback & preResetCallback() const { return preResetCallback_; }
	void preResetCallback(const PreResetCallback &preResetCallback) { preResetCallback_ = preResetCallback; }

	const PostResetCallback & postResetCallback() const { return postResetCallback_; }
	void postResetCallback(const PostResetCallback &postResetCallback) { postResetCallback_ = postResetCallback; }

	const PreCreateDeviceCallback & preCreateDeviceCallback() const { return preCreateDeviceCallback_; }
	void preCreateDeviceCallback(const PreCreateDeviceCallback &preCreateDeviceCallback)
	{
		preCreateDeviceCallback_ = preCreateDeviceCallback;
	}

	const PostCreateDeviceCallback & postCreateDeviceCallback() const { return postCreateDeviceCallback_; }
	void postCreateDeviceCallback(const PostCreateDeviceCallback &postCreateDeviceCallback)
	{
		postCreateDeviceCallback_ = postCreateDeviceCallback;
	}
	

	void DevPrePresent(IDirect3DDevice9 *sThis);
	void DevPreReset();
	void DevPostReset(IDirect3DDevice9 *sThis, D3DPRESENT_PARAMETERS *pPresentationParameters, HRESULT hr);
	void DevPostCreateVertexShader(const DWORD *pFunction, IDirect3DVertexShader9 **ppShader);
	void DevPostSetVertexShader(IDirect3DDevice9 *sThis, IDirect3DVertexShader9 *pShader);
	void DevPostCreatePixelShader(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader);
	void DevPostSetPixelShader(IDirect3DDevice9 *sThis, IDirect3DPixelShader9 *pShader);
	void ObjPreCreateDevice(HWND hFocusWindow);
	void ObjPostCreateDevice(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters);

protected:	
	const XXH64_hash_t preUiVertexShaderHash_ = 0x1fe3c6cd77e6e9f0;
	const XXH64_hash_t preUiPixelShaderHash_ = 0xccc38027cdd6cd51;
	IDirect3DVertexShader9* preUiVertexShader_ = nullptr;
	IDirect3DPixelShader9* preUiPixelShader_ = nullptr;

	bool isFrameDrawn_ = false;
	bool isInShaderHook_ = false;	

	DrawCallback drawUnderCallback_, drawOverCallback_;
	PreResetCallback preResetCallback_;
	PostResetCallback postResetCallback_;
	PreCreateDeviceCallback preCreateDeviceCallback_;
	PostCreateDeviceCallback postCreateDeviceCallback_;
};

}

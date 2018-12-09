#pragma once

#include <Main.h>
#include <xxhash/xxhash.h>
#include <d3d9.h>
#include <Direct3DVirtualFunctionTable.h>
#include <functional>
#include <Singleton.h>
#include <type_traits>

namespace GW2Addons
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

	void OnD3DCreate();

	D3DPRESENT_PARAMETERS SetupHookDevice(HWND &hWnd);
	void DeleteHookDevice(IDirect3DDevice9 *pDev, HWND hWnd);

	void LoadOriginalDevicePointers(IDirect3D9 *d3d);
	void LoadOriginalDevicePointers(IDirect3D9Ex *d3d);

	IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion);
	HRESULT WINAPI Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** output);

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
	
protected:
	Present_t Present_real = nullptr;
	HRESULT WINAPI Present_hook(IDirect3DDevice9 *sThis, CONST RECT *pSourceRect, CONST RECT *pDestRect,
	                            HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion);

	PresentEx_t PresentEx_real = nullptr;
	HRESULT WINAPI PresentEx_hook(IDirect3DDevice9Ex *sThis, CONST RECT *pSourceRect, CONST RECT *pDestRect,
	                              HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion, DWORD dwFlags);

	Reset_t Reset_real = nullptr;
	HRESULT WINAPI Reset_hook(IDirect3DDevice9 *sThis, D3DPRESENT_PARAMETERS *pPresentationParameters);

	ResetEx_t ResetEx_real = nullptr;
	HRESULT WINAPI ResetEx_hook(IDirect3DDevice9Ex *sThis, D3DPRESENT_PARAMETERS *pPresentationParameters,
	                            D3DDISPLAYMODEEX *pFullscreenDisplayMode);

	Release_t Release_real = nullptr;
	ULONG WINAPI Release_hook(IDirect3DDevice9 *sThis);

	AddRef_t AddRef_real = nullptr;
	ULONG WINAPI AddRef_hook(IDirect3DDevice9 *sThis);

	CreateVertexShader_t CreateVertexShader_real = nullptr;
	HRESULT WINAPI CreateVertexShader_hook(IDirect3DDevice9 *sThis, const DWORD *pFunction, IDirect3DVertexShader9 **ppShader);

	SetVertexShader_t SetVertexShader_real = nullptr;
	HRESULT WINAPI SetVertexShader_hook(IDirect3DDevice9 *sThis, IDirect3DVertexShader9 *pShader);

	CreatePixelShader_t CreatePixelShader_real = nullptr;
	HRESULT WINAPI CreatePixelShader_hook(IDirect3DDevice9 *sThis, const DWORD *pFunction, IDirect3DPixelShader9 **ppShader);

	SetPixelShader_t SetPixelShader_real = nullptr;
	HRESULT WINAPI SetPixelShader_hook(IDirect3DDevice9 *sThis, IDirect3DPixelShader9 *pShader);

	CreateDevice_t CreateDevice_real = nullptr;
	HRESULT WINAPI CreateDevice_hook(IDirect3D9 *sThis, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
	                                 DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters,
	                                 IDirect3DDevice9 **ppReturnedDeviceInterface);

	CreateDeviceEx_t CreateDeviceEx_real = nullptr;
	HRESULT WINAPI CreateDeviceEx_hook(IDirect3D9Ex *sThis, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
	                                   DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters,
	                                   D3DDISPLAYMODEEX *pFullscreenDisplayMode,
	                                   IDirect3DDevice9 **ppReturnedDeviceInterface);
	
	const XXH64_hash_t preUiVertexShaderHash_ = 0x1fe3c6cd77e6e9f0;
	const XXH64_hash_t preUiPixelShaderHash_ = 0xccc38027cdd6cd51;
	IDirect3DVertexShader9* preUiVertexShader_ = nullptr;
	IDirect3DPixelShader9* preUiPixelShader_ = nullptr;

	HMODULE realD3D9Module_ = nullptr;
	HMODULE chainD3D9Module_ = nullptr;
	Direct3DDevice9VirtualFunctionTable_t originalDeviceVfTable_ = { nullptr };

	bool isFrameDrawn_ = false;
	bool isInShaderHook_ = false;
	bool isDirect3DHooked_ = false;

	DrawCallback drawUnderCallback_, drawOverCallback_;
	PreResetCallback preResetCallback_;
	PostResetCallback postResetCallback_;
	PreCreateDeviceCallback preCreateDeviceCallback_;
	PostCreateDeviceCallback postCreateDeviceCallback_;
	
	Direct3D9VirtualFunctionTable_t direct3D9VirtualFunctionTable_ = { };
	Direct3DDevice9VirtualFunctionTable_t direct3DDevice9VirtualFunctionTable_ = { };
};

}

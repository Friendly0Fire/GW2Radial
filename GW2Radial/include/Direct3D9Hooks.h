#pragma once

#include <Main.h>
#include <d3d9.h>
#include <Direct3DVirtualFunctionTable.h>
#include <functional>

#include <Direct3D9Inject.h>

namespace GW2Radial
{

class Direct3D9Hooks : public Direct3D9Inject
{
public:
	Direct3D9Hooks();

	void OnD3DCreate();

	D3DPRESENT_PARAMETERS SetupHookDevice(HWND &hWnd);
	void DeleteHookDevice(IDirect3DDevice9 *pDev, HWND hWnd);

	void LoadOriginalDevicePointers(IDirect3D9 *d3d);
	void LoadOriginalDevicePointers(IDirect3D9Ex *d3d);

	IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion);
	HRESULT WINAPI Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** output);
	
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

	HMODULE realD3D9Module_ = nullptr;
	HMODULE chainD3D9Module_ = nullptr;

	Direct3DDevice9VirtualFunctionTable_t originalDeviceVfTable_ = { nullptr };
	Direct3D9VirtualFunctionTable_t direct3D9VirtualFunctionTable_ = { };
	Direct3DDevice9VirtualFunctionTable_t direct3DDevice9VirtualFunctionTable_ = { };
};
inline Direct3D9Hooks* GetD3D9Hooks() { return static_cast<Direct3D9Hooks*>(Direct3D9Inject::i()); }

}

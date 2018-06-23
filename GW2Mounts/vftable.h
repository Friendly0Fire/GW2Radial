#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct
	{
		LPVOID CreateDevice;
		LPVOID CreateDeviceEx;
	} D3D9_vftable;

	D3D9_vftable GetVirtualFunctionTableD3D9(IDirect3D9* obj);
	D3D9_vftable GetVirtualFunctionTableD3D9Ex(IDirect3D9Ex* obj);

	typedef HRESULT(WINAPI *CreateDevice_t)(IDirect3D9* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface);
	typedef HRESULT(WINAPI *CreateDeviceEx_t)(IDirect3D9Ex* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface);

	typedef struct
	{
		LPVOID Present;
		LPVOID PresentEx;
		LPVOID Reset;
		LPVOID ResetEx;
		LPVOID Release;
		LPVOID AddRef;
		LPVOID CreateVertexShader;
		LPVOID SetVertexShader;
		LPVOID CreatePixelShader;
		LPVOID SetPixelShader;
	} D3DDevice9_vftable;

	D3DDevice9_vftable GetVirtualFunctionTableD3DDevice9(IDirect3DDevice9* obj);
	D3DDevice9_vftable GetVirtualFunctionTableD3DDevice9Ex(IDirect3DDevice9Ex* obj);

	typedef HRESULT(WINAPI *Present_t)(IDirect3DDevice9* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
	typedef HRESULT(WINAPI *PresentEx_t)(IDirect3DDevice9Ex* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
	typedef HRESULT(WINAPI *Reset_t)(IDirect3DDevice9* _this, D3DPRESENT_PARAMETERS* pPresentationParameters);
	typedef HRESULT(WINAPI *ResetEx_t)(IDirect3DDevice9Ex* _this, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode);
	typedef ULONG(WINAPI *Release_t)(IDirect3DDevice9* _this);
	typedef ULONG(WINAPI *AddRef_t)(IDirect3DDevice9* _this);
	typedef HRESULT(WINAPI *CreateVertexShader_t)(IDirect3DDevice9* _this, CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader);
	typedef HRESULT(WINAPI *SetVertexShader_t)(IDirect3DDevice9* _this, IDirect3DVertexShader9* pShader);
	typedef HRESULT(WINAPI* CreatePixelShader_t)(IDirect3DDevice9* _this, CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader);
	typedef HRESULT(WINAPI *SetPixelShader_t)(IDirect3DDevice9* _this, IDirect3DPixelShader9* pShader);

#ifdef __cplusplus
}
#endif
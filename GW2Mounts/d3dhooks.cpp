#include "main.h"
#include <d3d9.h>
#include "vftable.h"
#include "minhook/include/MinHook.h"
#include "Utility.h"
#include <tchar.h>
#include "xxhash/xxhash.h"

XXH64_hash_t PreUIVertexShaderHash = 0x1fe3c6cd77e6e9f0;
XXH64_hash_t PreUIPixelShaderHash = 0xccc38027cdd6cd51;
IDirect3DVertexShader9* PreUIVertexShader = nullptr;
IDirect3DPixelShader9* PreUIPixelShader = nullptr;

typedef IDirect3D9* (WINAPI *Direct3DCreate9_t)(UINT SDKVersion);
typedef IDirect3D9Ex* (WINAPI *Direct3DCreate9Ex_t)(UINT SDKVersion);

void PreCreateDevice(HWND hFocusWindow);
void PostCreateDevice(IDirect3DDevice9* temp_device, D3DPRESENT_PARAMETERS *pPresentationParameters);

void Draw(IDirect3DDevice9* dev, bool FrameDrawn, bool SceneEnded);

void PreReset();
void PostReset(IDirect3DDevice9* dev, D3DPRESENT_PARAMETERS *pPresentationParameters);

void OnD3DCreate();

HMODULE RealD3D9Module = nullptr;
HMODULE ChainD3D9Module = nullptr;
D3DDevice9_vftable OriginalDeviceVFTable = { 0 };

bool FrameDrawn = false;

Present_t Present_real = nullptr;
HRESULT WINAPI Present_hook(IDirect3DDevice9* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
	Draw(_this, FrameDrawn, true);
	FrameDrawn = false;

	return Present_real(_this, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

PresentEx_t PresentEx_real = nullptr;
HRESULT WINAPI PresentEx_hook(IDirect3DDevice9Ex* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
	Draw(_this, FrameDrawn, true);
	FrameDrawn = false;

	return PresentEx_real(_this, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}

Reset_t Reset_real = nullptr;
HRESULT WINAPI Reset_hook(IDirect3DDevice9* _this, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	PreReset();

	HRESULT hr = Reset_real(_this, pPresentationParameters);
	if (hr != D3D_OK)
		return hr;

	PostReset(_this, pPresentationParameters);

	return D3D_OK;
}

ResetEx_t ResetEx_real = nullptr;
HRESULT WINAPI ResetEx_hook(IDirect3DDevice9Ex* _this, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode)
{
	PreReset();

	HRESULT hr = ResetEx_real(_this, pPresentationParameters, pFullscreenDisplayMode);
	if (hr != D3D_OK)
		return hr;

	PostReset(_this, pPresentationParameters);

	return D3D_OK;
}

Release_t Release_real = nullptr;
ULONG WINAPI Release_hook(IDirect3DDevice9* _this)
{
	ULONG refcount = Release_real(_this);

	return refcount;
}

AddRef_t AddRef_real = nullptr;
ULONG WINAPI AddRef_hook(IDirect3DDevice9* _this)
{
	return AddRef_real(_this);
}

CreateVertexShader_t CreateVertexShader_real = nullptr;
HRESULT CreateVertexShader_hook(IDirect3DDevice9* _this, const DWORD *pFunction, IDirect3DVertexShader9 **ppShader)
{
	HRESULT hr = CreateVertexShader_real(_this, pFunction, ppShader);

	if (!PreUIVertexShader)
	{
		int l = GetShaderFuncLength(pFunction);
		XXH64_hash_t hash = XXH64(pFunction, l, 0);
		if (hash == PreUIVertexShaderHash)
			PreUIVertexShader = *ppShader;
	}

	return hr;
}

SetVertexShader_t SetVertexShader_real = nullptr;
HRESULT SetVertexShader_hook(IDirect3DDevice9* _this, IDirect3DVertexShader9 *pShader)
{
	HRESULT hr = SetVertexShader_real(_this, pShader);

	if (!FrameDrawn && pShader == PreUIVertexShader)
	{
		Draw(_this, FrameDrawn, false);
		FrameDrawn = true;
	}

	return hr;
}

CreatePixelShader_t CreatePixelShader_real = nullptr;
HRESULT CreatePixelShader_hook(IDirect3DDevice9* _this, const DWORD *pFunction, IDirect3DPixelShader9 **ppShader)
{
	HRESULT hr = CreatePixelShader_real(_this, pFunction, ppShader);

	if (!PreUIPixelShader)
	{
		int l = GetShaderFuncLength(pFunction);
		XXH64_hash_t hash = XXH64(pFunction, l, 0);
		if (hash == PreUIPixelShaderHash)
			PreUIPixelShader = *ppShader;
	}

	return hr;
}

SetPixelShader_t SetPixelShader_real = nullptr;
HRESULT SetPixelShader_hook(IDirect3DDevice9* _this, IDirect3DPixelShader9 *pShader)
{
	HRESULT hr = SetPixelShader_real(_this, pShader);

	if (!FrameDrawn && pShader == PreUIPixelShader)
	{
		Draw(_this, FrameDrawn, false);
		FrameDrawn = true;
	}

	return hr;
}

CreateDevice_t CreateDevice_real = nullptr;
HRESULT WINAPI CreateDevice_hook(IDirect3D9* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
	PreCreateDevice(hFocusWindow);

	IDirect3DDevice9* temp_device = nullptr;
	HRESULT hr = CreateDevice_real(_this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &temp_device);
	if (hr != D3D_OK)
		return hr;

	*ppReturnedDeviceInterface = temp_device;

	auto vftd = GetVirtualFunctionTableD3DDevice9(temp_device);

	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.Reset, vftd.Reset), (LPVOID)&Reset_hook, (LPVOID*)&Reset_real);
	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.Present, vftd.Present), (LPVOID)&Present_hook, (LPVOID*)&Present_real);
	//MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.Release, vftd.Release), (LPVOID)&Release_hook, (LPVOID*)&Release_real);
	//MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.AddRef, vftd.AddRef), (LPVOID)&AddRef_hook, (LPVOID*)&AddRef_real);
	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.CreateVertexShader, vftd.CreateVertexShader), (LPVOID)&CreateVertexShader_hook, (LPVOID*)&CreateVertexShader_real);
	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.SetVertexShader, vftd.SetVertexShader), (LPVOID)&SetVertexShader_hook, (LPVOID*)&SetVertexShader_real);
	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.CreatePixelShader, vftd.CreatePixelShader), (LPVOID)&CreatePixelShader_hook, (LPVOID*)&CreatePixelShader_real);
	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.SetPixelShader, vftd.SetPixelShader), (LPVOID)&SetPixelShader_hook, (LPVOID*)&SetPixelShader_real);
	MH_EnableHook(MH_ALL_HOOKS);

	PostCreateDevice(temp_device, pPresentationParameters);

	return hr;
}

CreateDeviceEx_t CreateDeviceEx_real = nullptr;
HRESULT WINAPI CreateDeviceEx_hook(IDirect3D9Ex* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9** ppReturnedDeviceInterface)
{
	PreCreateDevice(hFocusWindow);

	IDirect3DDevice9Ex* temp_device = nullptr;
	HRESULT hr = CreateDeviceEx_real(_this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, &temp_device);
	if (hr != D3D_OK)
		return hr;

	*ppReturnedDeviceInterface = temp_device;

	auto vftd = GetVirtualFunctionTableD3DDevice9Ex(temp_device);

	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.Reset, vftd.Reset), (LPVOID)&Reset_hook, (LPVOID*)&Reset_real);
	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.Present, vftd.Present), (LPVOID)&Present_hook, (LPVOID*)&Present_real);
	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.ResetEx, vftd.ResetEx), (LPVOID)&ResetEx_hook, (LPVOID*)&ResetEx_real);
	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.PresentEx, vftd.PresentEx), (LPVOID)&PresentEx_hook, (LPVOID*)&PresentEx_real);
	//MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.Release, vftd.Release), (LPVOID)&Release_hook, (LPVOID*)&Release_real);
	//MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.AddRef, vftd.AddRef), (LPVOID)&AddRef_hook, (LPVOID*)&AddRef_real);
	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.CreateVertexShader, vftd.CreateVertexShader), (LPVOID)&CreateVertexShader_hook, (LPVOID*)&CreateVertexShader_real);
	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.SetVertexShader, vftd.SetVertexShader), (LPVOID)&SetVertexShader_hook, (LPVOID*)&SetVertexShader_real);
	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.CreatePixelShader, vftd.CreatePixelShader), (LPVOID)&CreatePixelShader_hook, (LPVOID*)&CreatePixelShader_real);
	MH_CreateHook(NULL_COALESCE(OriginalDeviceVFTable.SetPixelShader, vftd.SetPixelShader), (LPVOID)&SetPixelShader_hook, (LPVOID*)&SetPixelShader_real);
	MH_EnableHook(MH_ALL_HOOKS);

	PostCreateDevice(temp_device, pPresentationParameters);

	return hr;
}

void OnD3DCreate()
{
	if (!RealD3D9Module)
	{
		TCHAR path[MAX_PATH];

		GetSystemDirectory(path, MAX_PATH);
		_tcscat_s(path, TEXT("\\d3d9.dll"));

		RealD3D9Module = LoadLibrary(path);
	}

	if (!ChainD3D9Module)
	{
		TCHAR path[MAX_PATH];

		GetCurrentDirectory(MAX_PATH, path);
		_tcscat_s(path, TEXT("\\d3d9_mchain.dll"));

		if (!FileExists(path))
		{
			GetCurrentDirectory(MAX_PATH, path);
			_tcscat_s(path, TEXT("\\bin64\\d3d9_mchain.dll"));
		}

		if (!FileExists(path))
		{
			GetCurrentDirectory(MAX_PATH, path);
			_tcscat_s(path, TEXT("\\ReShade64.dll"));
		}

		if (!FileExists(path))
		{
			GetCurrentDirectory(MAX_PATH, path);
			_tcscat_s(path, TEXT("\\bin64\\ReShade64.dll"));
		}

		if (FileExists(path))
			ChainD3D9Module = LoadLibrary(path);
	}
}

D3DPRESENT_PARAMETERS SetupHookDevice(HWND &hWnd)
{
	WNDCLASSEXA wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = DefWindowProc;
	wc.hInstance = GetModuleHandleA(NULL);
	wc.lpszClassName = "DXTMP";
	RegisterClassExA(&wc);

	hWnd = CreateWindowA("DXTMP", 0, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), 0, wc.hInstance, 0);

	D3DPRESENT_PARAMETERS d3dPar = { 0 };
	d3dPar.Windowed = TRUE;
	d3dPar.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dPar.hDeviceWindow = hWnd;
	d3dPar.BackBufferCount = 1;
	d3dPar.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dPar.BackBufferHeight = 300;
	d3dPar.BackBufferHeight = 300;

	return d3dPar;
}

void DeleteHookDevice(IDirect3DDevice9* pDev, HWND hWnd)
{
	COM_RELEASE(pDev);

	DestroyWindow(hWnd);
	UnregisterClassA("DXTMP", GetModuleHandleA(NULL));
}

void LoadOriginalDevicePointers(IDirect3D9* d3d)
{
	HWND hWnd;
	auto d3dpar = SetupHookDevice(hWnd);
	IDirect3DDevice9* pDev;
	CreateDevice_real(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpar, &pDev);

	OriginalDeviceVFTable = GetVirtualFunctionTableD3DDevice9(pDev);

	DeleteHookDevice(pDev, hWnd);
}

void LoadOriginalDevicePointers(IDirect3D9Ex* d3d)
{
	HWND hWnd;
	auto d3dpar = SetupHookDevice(hWnd);
	IDirect3DDevice9Ex* pDev;
	CreateDeviceEx_real(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpar, nullptr, &pDev);

	OriginalDeviceVFTable = GetVirtualFunctionTableD3DDevice9Ex(pDev);

	DeleteHookDevice(pDev, hWnd);
}

bool HookedD3D = false;

IDirect3D9 *WINAPI Direct3DCreate9(UINT SDKVersion)
{
	OnD3DCreate();

	auto fDirect3DCreate9 = (Direct3DCreate9_t)GetProcAddress(RealD3D9Module, "Direct3DCreate9");
	auto d3d = fDirect3DCreate9(SDKVersion);

	if (!HookedD3D)
	{
		auto vft = GetVirtualFunctionTableD3D9(d3d);

		MH_CreateHook(vft.CreateDevice, (LPVOID)&CreateDevice_hook, (LPVOID*)&CreateDevice_real);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	if (ChainD3D9Module)
	{
		LoadOriginalDevicePointers(d3d);
		d3d->Release();

		fDirect3DCreate9 = (Direct3DCreate9_t)GetProcAddress(ChainD3D9Module, "Direct3DCreate9");
		d3d = fDirect3DCreate9(SDKVersion);
	}

	HookedD3D = true;

	return d3d;
}

IDirect3D9Ex *WINAPI Direct3DCreate9Ex(UINT SDKVersion)
{
	OnD3DCreate();

	auto fDirect3DCreate9 = (Direct3DCreate9Ex_t)GetProcAddress(RealD3D9Module, "Direct3DCreate9Ex");
	auto d3d = fDirect3DCreate9(SDKVersion);

	if (!HookedD3D)
	{
		auto vft = GetVirtualFunctionTableD3D9Ex(d3d);

		MH_CreateHook(vft.CreateDevice, (LPVOID)&CreateDevice_hook, (LPVOID*)&CreateDevice_real);
		MH_CreateHook(vft.CreateDeviceEx, (LPVOID)&CreateDeviceEx_hook, (LPVOID*)&CreateDeviceEx_real);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	if (ChainD3D9Module)
	{
		d3d->Release();

		fDirect3DCreate9 = (Direct3DCreate9Ex_t)GetProcAddress(ChainD3D9Module, "Direct3DCreate9Ex");
		d3d = fDirect3DCreate9(SDKVersion);
	}

	return d3d;
}
#include <Direct3D11Loader.h>
#include <Utility.h>
#include <tchar.h>

#include <gw2al_d3d9_wrapper.h>

typedef struct com_vtable
{
	void* methods[1024];
} com_vtable;
typedef struct wrapped_com_obj
{
	com_vtable* vtable;
	union
	{
		IDirect3D9* orig_obj;
		IDirect3DDevice9* orig_dev;
		ID3D11Device5* orig_dev11;
		IDXGISwapChain4* orig_swc;
		IDXGIFactory5* orig_dxgi;
	};
} wrapped_com_obj;

typedef struct wrap_event_data
{
	void* ret;
	wrapped_com_obj** stackPtr;
} wrap_event_data;

namespace GW2Radial
{

void Direct3D11Loader::DevPostRelease(IDirect3DDevice9 * sThis, ULONG refs)
{
	if (refs == 1)
	{
		preResetCallback();
		sThis->Release();
	}
}

void Direct3D11Loader::DevPrePresent(IDirect3DDevice9 *sThis)
{
	drawOverCallback(sThis, isFrameDrawn_, true);
	isFrameDrawn_ = false;	
}

void Direct3D11Loader::DevPreReset()
{
	preResetCallback();
}

void Direct3D11Loader::DevPostReset(IDirect3DDevice9 *sThis, D3DPRESENT_PARAMETERS *pPresentationParameters, HRESULT hr)
{
	if (FAILED(hr))
		return;

	postResetCallback(sThis, pPresentationParameters);	
}

void Direct3D11Loader::DevPostCreateVertexShader(const DWORD *pFunction, IDirect3DVertexShader9 **ppShader)
{
	if (!preUiVertexShader_)
	{
		int l = GetShaderFuncLength(pFunction);
		XXH64_hash_t hash = XXH64(pFunction, l, 0);
		if (hash == preUiVertexShaderHash_)
			preUiVertexShader_ = *ppShader;
	}	
}

void Direct3D11Loader::DevPostSetVertexShader(IDirect3DDevice9 *sThis, IDirect3DVertexShader9 *pShader)
{	
	if (!isInShaderHook_ && pShader && !isFrameDrawn_ && pShader == preUiVertexShader_)
	{
		isInShaderHook_ = true;
		drawUnderCallback(sThis, isFrameDrawn_, false);
		isInShaderHook_ = false;
		isFrameDrawn_ = true;
	}	
}
void Direct3D11Loader::DevPostCreatePixelShader(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader)
{	
	if (!preUiPixelShader_)
	{
		int l = GetShaderFuncLength(pFunction);
		XXH64_hash_t hash = XXH64(pFunction, l, 0);
		if (hash == preUiPixelShaderHash_)
			preUiPixelShader_ = *ppShader;
	}	
}

void Direct3D11Loader::DevPostSetPixelShader(IDirect3DDevice9 *sThis, IDirect3DPixelShader9 *pShader)
{
	if (!isInShaderHook_ && pShader && !isFrameDrawn_ && pShader == preUiPixelShader_)
	{
		isInShaderHook_ = true;
		drawUnderCallback(sThis, isFrameDrawn_, false);
		isInShaderHook_ = false;
		isFrameDrawn_ = true;
	}
}

void Direct3D11Loader::ObjPreCreateDevice(HWND hFocusWindow)
{
	preCreateDeviceCallback(hFocusWindow);
}

void Direct3D11Loader::ObjPostCreateDevice(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	postCreateDeviceCallback(pDevice, pPresentationParameters);	
	pDevice->AddRef();
}

void OnDevPostRelease(wrap_event_data* evd)
{
	GetD3D11Loader()->DevPostRelease((*evd->stackPtr)->orig_dev11, *((ULONG*)evd->ret));
}

void OnSwapChainPrePresent(wrap_event_data* evd)
{
	GetD3D11Loader()->SwapChainPrePresent((*evd->stackPtr)->orig_swc);
}

void OnSwapChainPrePresent1(wrap_event_data* evd)
{
	GetD3D11Loader()->SwapChainPrePresent((*evd->stackPtr)->orig_swc);
}

void OnObjPreCreateDevice(wrap_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GetD3D11Loader()->ObjPreCreateDevice(dx_api_cp->CreateDevice.v3);
}

void OnObjPostCreateDevice(wrap_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GetD3D11Loader()->ObjPostCreateDevice(*dx_api_cp->CreateDevice.ret, dx_api_cp->CreateDevice.v5);
}

void OnDXGIPostCreateSwapChain(wrap_event_data* evd)
{
	GetD3D11Loader()->DXGICreateSwapChain((*evd->stackPtr)->orig_swc);
}

void Direct3D11Loader::Init(gw2al_core_vtable* gAPI)
{
	D3D9_wrapper d3d9_wrap;
	d3d9_wrap.enable_event = static_cast<pD3D9_wrapper_enable_event>(gAPI->query_function(
        gAPI->hash_name(const_cast<wchar_t*>(D3D9_WRAPPER_ENABLE_EVENT_FNAME))));

	d3d9_wrap.enable_event(METH_OBJ_CreateDevice, WRAP_CB_PRE_POST);
	d3d9_wrap.enable_event(METH_DEV11_Release, WRAP_CB_POST);
	d3d9_wrap.enable_event(METH_SWC_Present, WRAP_CB_PRE);
	d3d9_wrap.enable_event(METH_SWC_Present1, WRAP_CB_PRE);
	d3d9_wrap.enable_event(METH_DXGI_CreateSwapChain, WRAP_CB_POST);
	d3d9_wrap.enable_event(METH_DXGI_CreateSwapChainForComposition, WRAP_CB_POST);
	d3d9_wrap.enable_event(METH_DXGI_CreateSwapChainForCoreWindow, WRAP_CB_POST);
	d3d9_wrap.enable_event(METH_DXGI_CreateSwapChainForHwnd, WRAP_CB_POST);

	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DEV11_Release", OnDevPostRelease, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_PRE_SWC_Present", OnSwapChainPrePresent, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_PRE_SWC_Present1", OnSwapChainPrePresent1, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_PRE_OBJ_CreateDevice", OnObjPreCreateDevice, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_OBJ_CreateDevice", OnObjPostCreateDevice, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DXGI_CreateSwapChain", OnDXGIPostCreateSwapChain, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DXGI_CreateSwapChainForComposition", OnDXGIPostCreateSwapChain, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DXGI_CreateSwapChainForCoreWindow", OnDXGIPostCreateSwapChain, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DXGI_CreateSwapChainForHwnd", OnDXGIPostCreateSwapChain, 0);
}

}


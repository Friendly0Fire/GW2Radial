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
	union
	{
		struct
		{
			UINT SyncInterval;
			UINT Flags;
		} Present;
		struct
		{
			UINT SyncInterval;
			UINT PresentFlags;
			const DXGI_PRESENT_PARAMETERS* pPresentParameters;
		} Present1;
		struct
		{
			IDXGIAdapter*			 pAdapter;
			D3D_DRIVER_TYPE          DriverType;
			HMODULE                  Software;
			UINT                     Flags;
			const D3D_FEATURE_LEVEL* pFeatureLevels;
			UINT                     FeatureLevels;
			UINT                     SDKVersion;
			ID3D11Device**			 ppDevice;
			D3D_FEATURE_LEVEL*		 pFeatureLevel;
			ID3D11DeviceContext**	 ppImmediateContext;
		} CreateDevice;
		struct
		{
			IUnknown* pDevice;
			DXGI_SWAP_CHAIN_DESC* pDesc;
			IDXGISwapChain** ppSwapChain;
		} CreateSwapChain;
		struct
		{
			IUnknown* pDevice;
			HWND hWnd;
			const DXGI_SWAP_CHAIN_DESC1* pDesc;
			const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc;
			IDXGIOutput* pRestrictToOutput;
			IDXGISwapChain1** ppSwapChain;
		} CreateSwapChainForHwnd;
	};
} wrapped_com_obj;

typedef struct wrap_event_data
{
	void* ret;
	wrapped_com_obj** stackPtr;
} wrap_event_data;

namespace GW2Radial
{

void Direct3D11Loader::PrePresentSwapChain()
{
	prePresentSwapChainCallback();
}

void Direct3D11Loader::PreCreateSwapChain(HWND hwnd)
{
	preCreateSwapChainCallback(hwnd);
}

void Direct3D11Loader::PostCreateSwapChain(IDXGISwapChain* swc)
{
	postCreateSwapChainCallback(swc);
}

void Direct3D11Loader::PostCreateDevice(ID3D11Device* pDevice)
{
	postCreateDeviceCallback(pDevice);
}

void OnSwapChainPrePresent(wrap_event_data* evd)
{
	GetD3D11Loader()->PrePresentSwapChain();
}

void OnSwapChainPrePresent1(wrap_event_data* evd)
{
	GetD3D11Loader()->PrePresentSwapChain();
}

void OnPostCreateDevice(wrap_event_data* evd)
{
	GetD3D11Loader()->PostCreateDevice(*(*evd->stackPtr)->CreateDevice.ppDevice);
}

void OnDXGIPostCreateSwapChain(wrap_event_data* evd)
{
	GetD3D11Loader()->PostCreateSwapChain(*(*evd->stackPtr)->CreateSwapChain.ppSwapChain);
}

void OnDXGIPostCreateSwapChainForHwnd(wrap_event_data* evd)
{
	GetD3D11Loader()->PostCreateSwapChain(*(*evd->stackPtr)->CreateSwapChainForHwnd.ppSwapChain);
}

void OnDXGIPreCreateSwapChain(wrap_event_data* evd)
{
	GetD3D11Loader()->PreCreateSwapChain((*evd->stackPtr)->CreateSwapChain.pDesc->OutputWindow);
}

void OnDXGIPreCreateSwapChainForHwnd(wrap_event_data* evd)
{
	GetD3D11Loader()->PreCreateSwapChain((*evd->stackPtr)->CreateSwapChainForHwnd.hWnd);
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

	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_PRE_SWC_Present", OnSwapChainPrePresent, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_PRE_SWC_Present1", OnSwapChainPrePresent1, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_OBJ_CreateDevice", OnPostCreateDevice, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_PRE_DXGI_CreateSwapChain", OnDXGIPreCreateSwapChain, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_PRE_DXGI_CreateSwapChainForHwnd", OnDXGIPreCreateSwapChainForHwnd, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DXGI_CreateSwapChain", OnDXGIPostCreateSwapChain, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DXGI_CreateSwapChainForHwnd", OnDXGIPostCreateSwapChainForHwnd, 0);
}

}

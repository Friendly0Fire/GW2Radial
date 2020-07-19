#include <Direct3D9Loader.h>
#include <Utility.h>
#include <tchar.h>
#include "gw2al_d3d9_wrapper.h"

namespace GW2Radial
{
DEFINE_SINGLETON(Direct3D9Loader);

Direct3D9Loader::Direct3D9Loader()
{
}

void Direct3D9Loader::DevPostRelease(IDirect3DDevice9 * sThis, ULONG refs)
{
	if (refs == 1)
	{
		preResetCallback_();
		sThis->Release();
	}
}

void Direct3D9Loader::DevPrePresent(IDirect3DDevice9 *sThis)
{
	drawOverCallback_(sThis, isFrameDrawn_, true);
	isFrameDrawn_ = false;	
}

void Direct3D9Loader::DevPreReset()
{
	preResetCallback_();
}

void Direct3D9Loader::DevPostReset(IDirect3DDevice9 *sThis, D3DPRESENT_PARAMETERS *pPresentationParameters, HRESULT hr)
{
	if (FAILED(hr))
		return;

	postResetCallback_(sThis, pPresentationParameters);	
}

void Direct3D9Loader::DevPostCreateVertexShader(const DWORD *pFunction, IDirect3DVertexShader9 **ppShader)
{
	if (!preUiVertexShader_)
	{
		int l = GetShaderFuncLength(pFunction);
		XXH64_hash_t hash = XXH64(pFunction, l, 0);
		if (hash == preUiVertexShaderHash_)
			preUiVertexShader_ = *ppShader;
	}	
}

void Direct3D9Loader::DevPostSetVertexShader(IDirect3DDevice9 *sThis, IDirect3DVertexShader9 *pShader)
{	
	if (!isInShaderHook_ && pShader && !isFrameDrawn_ && pShader == preUiVertexShader_)
	{
		isInShaderHook_ = true;
		drawUnderCallback_(sThis, isFrameDrawn_, false);
		isInShaderHook_ = false;
		isFrameDrawn_ = true;
	}	
}
void Direct3D9Loader::DevPostCreatePixelShader(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader)
{	
	if (!preUiPixelShader_)
	{
		int l = GetShaderFuncLength(pFunction);
		XXH64_hash_t hash = XXH64(pFunction, l, 0);
		if (hash == preUiPixelShaderHash_)
			preUiPixelShader_ = *ppShader;
	}	
}

void Direct3D9Loader::DevPostSetPixelShader(IDirect3DDevice9 *sThis, IDirect3DPixelShader9 *pShader)
{
	if (!isInShaderHook_ && pShader && !isFrameDrawn_ && pShader == preUiPixelShader_)
	{
		isInShaderHook_ = true;
		drawUnderCallback_(sThis, isFrameDrawn_, false);
		isInShaderHook_ = false;
		isFrameDrawn_ = true;
	}
}

void Direct3D9Loader::ObjPreCreateDevice(HWND hFocusWindow)
{
	preCreateDeviceCallback_(hFocusWindow);
}

void Direct3D9Loader::ObjPostCreateDevice(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	postCreateDeviceCallback_(pDevice, pPresentationParameters);	
	pDevice->AddRef();
}

#pragma pack(push, 1)
typedef struct d3d9_api_call {
	union {
		IDirect3D9* obj;
		IDirect3DDevice9* dev;
	};
	union {
		struct {
			UINT pad0;
			UINT v1;
			UINT pad1;
			D3DDEVTYPE v2;
			HWND v3;
			UINT pad2;
			DWORD v4;
			D3DPRESENT_PARAMETERS* v5;
			IDirect3DDevice9** ret;
		} CreateDevice;
		struct {
			DWORD            *pFunction;
			union {
				IDirect3DPixelShader9 **ppShader;
				IDirect3DVertexShader9 **pvShader;
			};
		} CreateShader;
		struct {
			union {
				IDirect3DVertexShader9 *vs;
				IDirect3DPixelShader9 *ps;
			};
		} SetShader;
		struct {
			D3DPRESENT_PARAMETERS* pPresentationParameters;
		} Reset;
	};
} d3d9_api_call;
#pragma pack(pop)

void OnDevPostRelease(D3D9_wrapper_event_data* evd)
{
	GW2Radial::Direct3D9Loader::i()->DevPostRelease((IDirect3DDevice9*)*evd->stackPtr, *((ULONG*)evd->ret));
}

void OnDevPrePresent(D3D9_wrapper_event_data* evd)
{
	GW2Radial::Direct3D9Loader::i()->DevPrePresent((IDirect3DDevice9*)*evd->stackPtr);
}

void OnDevPreReset(D3D9_wrapper_event_data* evd)
{
	GW2Radial::Direct3D9Loader::i()->DevPreReset();
}

void OnDevPostReset(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Loader::i()->DevPostReset(dx_api_cp->dev, dx_api_cp->Reset.pPresentationParameters, *((HRESULT*)evd->ret));
}

void OnDevPostCreateVertexShader(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Loader::i()->DevPostCreateVertexShader(dx_api_cp->CreateShader.pFunction, dx_api_cp->CreateShader.pvShader);
}

void OnDevPostSetVertexShader(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Loader::i()->DevPostSetVertexShader(dx_api_cp->dev, dx_api_cp->SetShader.vs);
}

void OnDevPostCreatePixelShader(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Loader::i()->DevPostCreatePixelShader(dx_api_cp->CreateShader.pFunction, dx_api_cp->CreateShader.ppShader);
}

void OnDevPostSetPixelShader(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Loader::i()->DevPostSetVertexShader(dx_api_cp->dev, dx_api_cp->SetShader.vs);
}

void OnObjPreCreateDevice(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Loader::i()->ObjPreCreateDevice(dx_api_cp->CreateDevice.v3);
}

void OnObjPostCreateDevice(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Loader::i()->ObjPostCreateDevice(*dx_api_cp->CreateDevice.ret, dx_api_cp->CreateDevice.v5);
}

void Direct3D9Loader::InitHooks(gw2al_core_vtable* gAPI)
{
	D3D9_wrapper d3d9_wrap;
	d3d9_wrap.enable_event = (pD3D9_wrapper_enable_event)gAPI->query_function(gAPI->hash_name((wchar_t*)D3D9_WRAPPER_ENABLE_EVENT_FNAME));

	d3d9_wrap.enable_event(METH_OBJ_CreateDevice, WRAP_CB_PRE_POST);
	d3d9_wrap.enable_event(METH_DEV_Release, WRAP_CB_POST);
	d3d9_wrap.enable_event(METH_DEV_Present, WRAP_CB_PRE);
	d3d9_wrap.enable_event(METH_DEV_Reset, WRAP_CB_PRE_POST);
	d3d9_wrap.enable_event(METH_DEV_CreateVertexShader, WRAP_CB_POST);
	d3d9_wrap.enable_event(METH_DEV_CreatePixelShader, WRAP_CB_POST);
	d3d9_wrap.enable_event(METH_DEV_SetVertexShader, WRAP_CB_POST);
	d3d9_wrap.enable_event(METH_DEV_SetPixelShader, WRAP_CB_POST);

	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DEV_Release", OnDevPostRelease, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_PRE_DEV_Present", OnDevPrePresent, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_PRE_DEV_Reset", OnDevPreReset, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DEV_Reset", OnDevPostReset, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DEV_CreateVertexShader", OnDevPostCreateVertexShader, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DEV_CreatePixelShader", OnDevPostCreatePixelShader, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DEV_SetVertexShader", OnDevPostSetVertexShader, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DEV_SetPixelShader", OnDevPostSetPixelShader, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_PRE_OBJ_CreateDevice", OnObjPreCreateDevice, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_OBJ_CreateDevice", OnObjPostCreateDevice, 0);
}

}


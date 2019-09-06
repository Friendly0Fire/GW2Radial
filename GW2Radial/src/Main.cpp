#include <Main.h>
#include <Core.h>
#include <Direct3D9Hooks.h>
#include "gw2al_api.h"
#include "gw2al_d3d9_wrapper.h"

gw2al_core_vtable* gAPI;

gw2al_addon_dsc gAddonDeps[] = {
	{
		L"loader_core",
		L"whatever",
		0,
		1,
		1,
		0
	},
	{0,0,0,0,0,0}
};

gw2al_addon_dsc gAddonDsc = {
	L"gw2radial",
	L"Radial menu for GW2",
	0,
	0,
	0,
	gAddonDeps
};

HMODULE custom_d3d9_module;

gw2al_addon_dsc* gw2addon_get_description()
{
	return &gAddonDsc;
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

void OnDevPrePresent(D3D9_wrapper_event_data* evd)
{
	GW2Radial::Direct3D9Hooks::i()->DevPrePresent((IDirect3DDevice9*)*evd->stackPtr);
}

void OnDevPreReset(D3D9_wrapper_event_data* evd)
{
	GW2Radial::Direct3D9Hooks::i()->DevPreReset();
}

void OnDevPostReset(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Hooks::i()->DevPostReset(dx_api_cp->dev, dx_api_cp->Reset.pPresentationParameters, *((HRESULT*)evd->ret));
}

void OnDevPostCreateVertexShader(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Hooks::i()->DevPostCreateVertexShader(dx_api_cp->CreateShader.pFunction, dx_api_cp->CreateShader.pvShader);
}

void OnDevPostSetVertexShader(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Hooks::i()->DevPostSetVertexShader(dx_api_cp->dev, dx_api_cp->SetShader.vs);
}

void OnDevPostCreatePixelShader(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Hooks::i()->DevPostCreatePixelShader(dx_api_cp->CreateShader.pFunction, dx_api_cp->CreateShader.ppShader);
}

void OnDevPostSetPixelShader(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Hooks::i()->DevPostSetVertexShader(dx_api_cp->dev, dx_api_cp->SetShader.vs);
}

void OnObjPreCreateDevice(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Hooks::i()->ObjPreCreateDevice(dx_api_cp->CreateDevice.v3);
}

void OnObjPostCreateDevice(D3D9_wrapper_event_data* evd)
{
	d3d9_api_call* dx_api_cp = (d3d9_api_call*)evd->stackPtr;
	GW2Radial::Direct3D9Hooks::i()->ObjPostCreateDevice(*dx_api_cp->CreateDevice.ret, dx_api_cp->CreateDevice.v5);
}

gw2al_api_ret gw2addon_load(gw2al_core_vtable* core_api)
{
	gAPI = core_api;

	D3D9_wrapper d3d9_wrap;
	d3d9_wrap.enable_event = (pD3D9_wrapper_enable_event)gAPI->query_function(gAPI->hash_name((wchar_t*)D3D9_WRAPPER_ENABLE_EVENT_FNAME));

	d3d9_wrap.enable_event(METH_OBJ_CreateDevice, WRAP_CB_PRE_POST);
	d3d9_wrap.enable_event(METH_DEV_Present, WRAP_CB_POST);
	d3d9_wrap.enable_event(METH_DEV_Reset, WRAP_CB_PRE_POST);
	d3d9_wrap.enable_event(METH_DEV_CreateVertexShader, WRAP_CB_POST);
	d3d9_wrap.enable_event(METH_DEV_CreatePixelShader, WRAP_CB_POST);
	d3d9_wrap.enable_event(METH_DEV_SetVertexShader, WRAP_CB_POST);
	d3d9_wrap.enable_event(METH_DEV_SetPixelShader, WRAP_CB_POST);

	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_PRE_DEV_Present", OnDevPrePresent, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_PRE_DEV_Reset", OnDevPreReset, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DEV_Reset", OnDevPostReset, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DEV_CreateVertexShader", OnDevPostCreateVertexShader, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DEV_CreatePixelShader", OnDevPostCreatePixelShader, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DEV_SetVertexShader", OnDevPostSetVertexShader, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_DEV_SetPixelShader", OnDevPostSetPixelShader, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_PRE_OBJ_CreateDevice", OnObjPreCreateDevice, 0);
	D3D9_WRAPPER_WATCH_EVENT(L"gw2radial", L"D3D9_POST_OBJ_CreateDevice", OnObjPostCreateDevice, 0);

	return GW2AL_OK;
}

gw2al_api_ret gw2addon_unload(int gameExiting)
{
	//TODO cleanup
	return GW2AL_OK;
}

bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
#ifdef _DEBUG
		while (!IsDebuggerPresent());
#endif
		GW2Radial::Core::Init(hModule);
		break;
	case DLL_PROCESS_DETACH:
		GW2Radial::Core::Shutdown();
		break;
	}

	return true;
}
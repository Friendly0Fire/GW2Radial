#pragma once

#include <Main.h>
#include <d3d9.h>
#include <Direct3D9Inject.h>
#include "gw2al_api.h"

namespace GW2Radial
{

class Direct3D9Loader : public Direct3D9Inject
{
public:
	Direct3D9Loader() = default;
	
	void DevPostRelease(IDirect3DDevice9 *sThis, ULONG refs);
	void DevPrePresent(IDirect3DDevice9 *sThis);
	void DevPreReset();
	void DevPostReset(IDirect3DDevice9 *sThis, D3DPRESENT_PARAMETERS *pPresentationParameters, HRESULT hr);
	void DevPostCreateVertexShader(const DWORD *pFunction, IDirect3DVertexShader9 **ppShader);
	void DevPostSetVertexShader(IDirect3DDevice9 *sThis, IDirect3DVertexShader9 *pShader);
	void DevPostCreatePixelShader(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader);
	void DevPostSetPixelShader(IDirect3DDevice9 *sThis, IDirect3DPixelShader9 *pShader);
	void ObjPreCreateDevice(HWND hFocusWindow);
	void ObjPostCreateDevice(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters);

	void Init(gw2al_core_vtable* gAPI);
};

inline Direct3D9Loader* GetD3D9Loader() { return static_cast<Direct3D9Loader*>(&Direct3D9Inject::i()); }

}

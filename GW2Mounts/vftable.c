#include <d3d9.h>
#include "vftable.h"

D3D9_vftable GetVirtualFunctionTableD3D9(IDirect3D9* obj)
{
	D3D9_vftable vft = { 0 };
	vft.CreateDevice = obj->lpVtbl->CreateDevice;

	return vft;
}

D3D9_vftable GetVirtualFunctionTableD3D9Ex(IDirect3D9Ex* obj)
{
	D3D9_vftable vft = { 0 };
	vft.CreateDevice = obj->lpVtbl->CreateDevice;
	vft.CreateDeviceEx = obj->lpVtbl->CreateDeviceEx;

	return vft;
}

D3DDevice9_vftable GetVirtualFunctionTableD3DDevice9(IDirect3DDevice9 * obj)
{
	D3DDevice9_vftable vft = { 0 };
	vft.Present = obj->lpVtbl->Present;
	vft.Reset = obj->lpVtbl->Reset;
	vft.Release = obj->lpVtbl->Release;
	vft.AddRef = obj->lpVtbl->AddRef;

	return vft;
}

D3DDevice9_vftable GetVirtualFunctionTableD3DDevice9Ex(IDirect3DDevice9Ex * obj)
{
	D3DDevice9_vftable vft = { 0 };
	vft.Present = obj->lpVtbl->Present;
	vft.PresentEx = obj->lpVtbl->PresentEx;
	vft.Reset = obj->lpVtbl->Reset;
	vft.ResetEx = obj->lpVtbl->ResetEx;
	vft.Release = obj->lpVtbl->Release;
	vft.AddRef = obj->lpVtbl->AddRef;

	return vft;
}

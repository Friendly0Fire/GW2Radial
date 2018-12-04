#include <d3d9.h>
#include "Direct3DVirtualFunctionTable.h"

Direct3D9VirtualFunctionTable_t GetVirtualFunctionTableD3D9(IDirect3D9* obj)
{
	Direct3D9VirtualFunctionTable_t vft = { 0 };
	vft.CreateDevice = obj->lpVtbl->CreateDevice;

	return vft;
}

Direct3D9VirtualFunctionTable_t GetVirtualFunctionTableD3D9Ex(IDirect3D9Ex* obj)
{
	Direct3D9VirtualFunctionTable_t vft = { 0 };
	vft.CreateDevice = obj->lpVtbl->CreateDevice;
	vft.CreateDeviceEx = obj->lpVtbl->CreateDeviceEx;

	return vft;
}

Direct3DDevice9VirtualFunctionTable_t GetVirtualFunctionTableD3DDevice9(IDirect3DDevice9 * obj)
{
	Direct3DDevice9VirtualFunctionTable_t vft = { 0 };
	vft.Present = obj->lpVtbl->Present;
	vft.Reset = obj->lpVtbl->Reset;
	vft.Release = obj->lpVtbl->Release;
	vft.AddRef = obj->lpVtbl->AddRef;
	vft.CreateVertexShader = obj->lpVtbl->CreateVertexShader;
	vft.CreatePixelShader = obj->lpVtbl->CreatePixelShader;
	vft.SetVertexShader = obj->lpVtbl->SetVertexShader;
	vft.SetPixelShader = obj->lpVtbl->SetPixelShader;

	return vft;
}

Direct3DDevice9VirtualFunctionTable_t GetVirtualFunctionTableD3DDevice9Ex(IDirect3DDevice9Ex * obj)
{
	Direct3DDevice9VirtualFunctionTable_t vft = { 0 };
	vft.Present = obj->lpVtbl->Present;
	vft.PresentEx = obj->lpVtbl->PresentEx;
	vft.Reset = obj->lpVtbl->Reset;
	vft.ResetEx = obj->lpVtbl->ResetEx;
	vft.Release = obj->lpVtbl->Release;
	vft.AddRef = obj->lpVtbl->AddRef;
	vft.CreateVertexShader = obj->lpVtbl->CreateVertexShader;
	vft.CreatePixelShader = obj->lpVtbl->CreatePixelShader;
	vft.SetVertexShader = obj->lpVtbl->SetVertexShader;
	vft.SetPixelShader = obj->lpVtbl->SetPixelShader;

	return vft;
}

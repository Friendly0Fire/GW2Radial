#include <Direct3D9Hooks.h>
#include <Utility.h>
#include <tchar.h>

namespace GW2Radial
{
DEFINE_SINGLETON(Direct3D9Hooks);

Direct3D9Hooks::Direct3D9Hooks()
{
}

void Direct3D9Hooks::DevPrePresent(IDirect3DDevice9 *sThis)
{
	drawOverCallback_(sThis, isFrameDrawn_, true);
	isFrameDrawn_ = false;	
}

void Direct3D9Hooks::DevPreReset()
{
	preResetCallback_();
}

void Direct3D9Hooks::DevPostReset(IDirect3DDevice9 *sThis, D3DPRESENT_PARAMETERS *pPresentationParameters, HRESULT hr)
{
	if (FAILED(hr))
		return;

	postResetCallback_(sThis, pPresentationParameters);	
}

void Direct3D9Hooks::DevPostCreateVertexShader(const DWORD *pFunction, IDirect3DVertexShader9 **ppShader)
{
	if (!preUiVertexShader_)
	{
		int l = GetShaderFuncLength(pFunction);
		XXH64_hash_t hash = XXH64(pFunction, l, 0);
		if (hash == preUiVertexShaderHash_)
			preUiVertexShader_ = *ppShader;
	}	
}

void Direct3D9Hooks::DevPostSetVertexShader(IDirect3DDevice9 *sThis, IDirect3DVertexShader9 *pShader)
{	
	if (!isInShaderHook_ && pShader && !isFrameDrawn_ && pShader == preUiVertexShader_)
	{
		isInShaderHook_ = true;
		drawUnderCallback_(sThis, isFrameDrawn_, false);
		isInShaderHook_ = false;
		isFrameDrawn_ = true;
	}	
}
void Direct3D9Hooks::DevPostCreatePixelShader(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader)
{	
	if (!preUiPixelShader_)
	{
		int l = GetShaderFuncLength(pFunction);
		XXH64_hash_t hash = XXH64(pFunction, l, 0);
		if (hash == preUiPixelShaderHash_)
			preUiPixelShader_ = *ppShader;
	}	
}

void Direct3D9Hooks::DevPostSetPixelShader(IDirect3DDevice9 *sThis, IDirect3DPixelShader9 *pShader)
{
	if (!isInShaderHook_ && pShader && !isFrameDrawn_ && pShader == preUiPixelShader_)
	{
		isInShaderHook_ = true;
		drawUnderCallback_(sThis, isFrameDrawn_, false);
		isInShaderHook_ = false;
		isFrameDrawn_ = true;
	}
}

void Direct3D9Hooks::ObjPreCreateDevice(HWND hFocusWindow)
{
	preCreateDeviceCallback_(hFocusWindow);
}

void Direct3D9Hooks::ObjPostCreateDevice(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	postCreateDeviceCallback_(pDevice, pPresentationParameters);	
}

}


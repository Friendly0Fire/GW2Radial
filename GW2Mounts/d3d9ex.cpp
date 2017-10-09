#include "d3d9ex.h"

D3DC9Ex orig_Direct3DCreate9Ex;

/*************************
Bare D3DEx Callbacks
*************************/

ULONG f_iD3D9Ex::AddRef()
{
	return f_pD3DEx->AddRef();
}

ULONG f_iD3D9Ex::Release()
{
	return f_pD3DEx->Release();
}

HRESULT f_iD3D9Ex::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	return f_pD3DEx->QueryInterface(riid, ppvObj);
}

HRESULT f_iD3D9Ex::EnumAdapterModes(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	return f_pD3DEx->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

UINT f_iD3D9Ex::GetAdapterCount()
{
	return f_pD3DEx->GetAdapterCount();
}

HRESULT f_iD3D9Ex::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
	return f_pD3DEx->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT f_iD3D9Ex::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
	return f_pD3DEx->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT f_iD3D9Ex::GetAdapterModeCount(THIS_ UINT Adapter, D3DFORMAT Format)
{
	return f_pD3DEx->GetAdapterModeCount(Adapter, Format);
}

HMONITOR f_iD3D9Ex::GetAdapterMonitor(UINT Adapter)
{
	return f_pD3DEx->GetAdapterMonitor(Adapter);
}

HRESULT f_iD3D9Ex::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps)
{
	return f_pD3DEx->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HRESULT f_iD3D9Ex::RegisterSoftwareDevice(void *pInitializeFunction)
{
	return f_pD3DEx->RegisterSoftwareDevice(pInitializeFunction);
}

HRESULT f_iD3D9Ex::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	return f_pD3DEx->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT f_iD3D9Ex::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	return f_pD3DEx->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT f_iD3D9Ex::CheckDeviceMultiSampleType(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
	return f_pD3DEx->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT f_iD3D9Ex::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed)
{
	return f_pD3DEx->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}


HRESULT f_iD3D9Ex::CheckDeviceFormatConversion(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
	return f_pD3DEx->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

STDMETHODIMP_(UINT) f_iD3D9Ex::GetAdapterModeCountEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER * pFilter)
{
	return f_pD3DEx->GetAdapterModeCountEx(Adapter, pFilter);
}

STDMETHODIMP f_iD3D9Ex::EnumAdapterModesEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER * pFilter, UINT Mode, D3DDISPLAYMODEEX * pMode)
{
	return f_pD3DEx->EnumAdapterModesEx(Adapter, pFilter, Mode, pMode);
}

STDMETHODIMP f_iD3D9Ex::GetAdapterDisplayModeEx(UINT Adapter, D3DDISPLAYMODEEX * pMode, D3DDISPLAYROTATION * pRotation)
{
	return f_pD3DEx->GetAdapterDisplayModeEx(Adapter, pMode, pRotation);
}

STDMETHODIMP f_iD3D9Ex::GetAdapterLUID(UINT Adapter, LUID * pLUID)
{
	return f_pD3DEx->GetAdapterLUID(Adapter, pLUID);
}

/*************************
Bare Device Callbacks
*************************/

f_IDirect3DDevice9Ex::f_IDirect3DDevice9Ex(LPDIRECT3DDEVICE9EX pDevice)
{
	f_pD3DDeviceEx = pDevice;
}

ULONG f_IDirect3DDevice9Ex::AddRef()
{
	return f_pD3DDeviceEx->AddRef();
}

ULONG f_IDirect3DDevice9Ex::Release()
{
	return f_pD3DDeviceEx->Release();
}

HRESULT f_IDirect3DDevice9Ex::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	return f_pD3DDeviceEx->QueryInterface(riid, ppvObj);
}

void f_IDirect3DDevice9Ex::SetCursorPosition(int X, int Y, DWORD Flags)
{
	f_pD3DDeviceEx->SetCursorPosition(X, Y, Flags);
}

HRESULT f_IDirect3DDevice9Ex::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9 *pCursorBitmap)
{
	return f_pD3DDeviceEx->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

BOOL f_IDirect3DDevice9Ex::ShowCursor(BOOL bShow)
{
	return f_pD3DDeviceEx->ShowCursor(bShow);
}

HRESULT f_IDirect3DDevice9Ex::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain)
{
	return f_pD3DDeviceEx->CreateAdditionalSwapChain(pPresentationParameters, ppSwapChain);
}

HRESULT f_IDirect3DDevice9Ex::CreateCubeTexture(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	return f_pD3DDeviceEx->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT f_IDirect3DDevice9Ex::CreateDepthStencilSurface(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return f_pD3DDeviceEx->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
}

HRESULT f_IDirect3DDevice9Ex::CreateIndexBuffer(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	return f_pD3DDeviceEx->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT f_IDirect3DDevice9Ex::CreateRenderTarget(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return f_pD3DDeviceEx->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
}

HRESULT f_IDirect3DDevice9Ex::CreateTexture(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	return f_pD3DDeviceEx->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT f_IDirect3DDevice9Ex::CreateVertexBuffer(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	return f_pD3DDeviceEx->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT f_IDirect3DDevice9Ex::CreateVolumeTexture(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	return f_pD3DDeviceEx->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
}

HRESULT f_IDirect3DDevice9Ex::BeginStateBlock()
{
	return f_pD3DDeviceEx->BeginStateBlock();
}

HRESULT f_IDirect3DDevice9Ex::CreateStateBlock(THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	return f_pD3DDeviceEx->CreateStateBlock(Type, ppSB);
}

HRESULT f_IDirect3DDevice9Ex::EndStateBlock(THIS_ IDirect3DStateBlock9** ppSB)
{
	return f_pD3DDeviceEx->EndStateBlock(ppSB);
}

HRESULT f_IDirect3DDevice9Ex::GetClipStatus(D3DCLIPSTATUS9 *pClipStatus)
{
	return f_pD3DDeviceEx->GetClipStatus(pClipStatus);
}

HRESULT f_IDirect3DDevice9Ex::GetDisplayMode(THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
	return f_pD3DDeviceEx->GetDisplayMode(iSwapChain, pMode);
}

HRESULT f_IDirect3DDevice9Ex::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue)
{
	return f_pD3DDeviceEx->GetRenderState(State, pValue);
}

HRESULT f_IDirect3DDevice9Ex::GetRenderTarget(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	return f_pD3DDeviceEx->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT f_IDirect3DDevice9Ex::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix)
{
	return f_pD3DDeviceEx->GetTransform(State, pMatrix);
}

HRESULT f_IDirect3DDevice9Ex::SetClipStatus(CONST D3DCLIPSTATUS9 *pClipStatus)
{
	return f_pD3DDeviceEx->SetClipStatus(pClipStatus);
}

HRESULT f_IDirect3DDevice9Ex::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	return f_pD3DDeviceEx->SetRenderState(State, Value);
}

HRESULT f_IDirect3DDevice9Ex::SetRenderTarget(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	return f_pD3DDeviceEx->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT f_IDirect3DDevice9Ex::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	return f_pD3DDeviceEx->SetTransform(State, pMatrix);
}

void f_IDirect3DDevice9Ex::GetGammaRamp(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
	f_pD3DDeviceEx->GetGammaRamp(iSwapChain, pRamp);
}

void f_IDirect3DDevice9Ex::SetGammaRamp(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	f_pD3DDeviceEx->SetGammaRamp(iSwapChain, Flags, pRamp);
}

HRESULT f_IDirect3DDevice9Ex::DeletePatch(UINT Handle)
{
	return f_pD3DDeviceEx->DeletePatch(Handle);
}

HRESULT f_IDirect3DDevice9Ex::DrawRectPatch(UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo)
{
	return f_pD3DDeviceEx->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT f_IDirect3DDevice9Ex::DrawTriPatch(UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo)
{
	return f_pD3DDeviceEx->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT f_IDirect3DDevice9Ex::GetIndices(THIS_ IDirect3DIndexBuffer9** ppIndexData)
{
	return f_pD3DDeviceEx->GetIndices(ppIndexData);
}

HRESULT f_IDirect3DDevice9Ex::SetIndices(THIS_ IDirect3DIndexBuffer9* pIndexData)
{
	return f_pD3DDeviceEx->SetIndices(pIndexData);
}

UINT f_IDirect3DDevice9Ex::GetAvailableTextureMem()
{
	return f_pD3DDeviceEx->GetAvailableTextureMem();
}

HRESULT f_IDirect3DDevice9Ex::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	return f_pD3DDeviceEx->GetCreationParameters(pParameters);
}

HRESULT f_IDirect3DDevice9Ex::GetDeviceCaps(D3DCAPS9 *pCaps)
{
	return f_pD3DDeviceEx->GetDeviceCaps(pCaps);
}

HRESULT f_IDirect3DDevice9Ex::GetDirect3D(IDirect3D9 **ppD3D9)
{
	return f_pD3DDeviceEx->GetDirect3D(ppD3D9);
}

HRESULT f_IDirect3DDevice9Ex::GetRasterStatus(THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	return f_pD3DDeviceEx->GetRasterStatus(iSwapChain, pRasterStatus);
}

HRESULT f_IDirect3DDevice9Ex::GetLight(DWORD Index, D3DLIGHT9 *pLight)
{
	return f_pD3DDeviceEx->GetLight(Index, pLight);
}

HRESULT f_IDirect3DDevice9Ex::GetLightEnable(DWORD Index, BOOL *pEnable)
{
	return f_pD3DDeviceEx->GetLightEnable(Index, pEnable);
}

HRESULT f_IDirect3DDevice9Ex::GetMaterial(D3DMATERIAL9 *pMaterial)
{
	return f_pD3DDeviceEx->GetMaterial(pMaterial);
}

HRESULT f_IDirect3DDevice9Ex::LightEnable(DWORD LightIndex, BOOL bEnable)
{
	return f_pD3DDeviceEx->LightEnable(LightIndex, bEnable);
}

HRESULT f_IDirect3DDevice9Ex::SetLight(DWORD Index, CONST D3DLIGHT9 *pLight)
{

	return f_pD3DDeviceEx->SetLight(Index, pLight);
}

HRESULT f_IDirect3DDevice9Ex::SetMaterial(CONST D3DMATERIAL9 *pMaterial)
{
	return f_pD3DDeviceEx->SetMaterial(pMaterial);
}

HRESULT f_IDirect3DDevice9Ex::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	return f_pD3DDeviceEx->MultiplyTransform(State, pMatrix);
}

HRESULT f_IDirect3DDevice9Ex::ProcessVertices(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
	return f_pD3DDeviceEx->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT f_IDirect3DDevice9Ex::TestCooperativeLevel()
{
	return f_pD3DDeviceEx->TestCooperativeLevel();
}

HRESULT f_IDirect3DDevice9Ex::GetCurrentTexturePalette(UINT *pPaletteNumber)
{
	return f_pD3DDeviceEx->GetCurrentTexturePalette(pPaletteNumber);
}

HRESULT f_IDirect3DDevice9Ex::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries)
{
	return f_pD3DDeviceEx->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT f_IDirect3DDevice9Ex::SetCurrentTexturePalette(UINT PaletteNumber)
{
	return f_pD3DDeviceEx->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT f_IDirect3DDevice9Ex::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY *pEntries)
{
	return f_pD3DDeviceEx->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT f_IDirect3DDevice9Ex::CreatePixelShader(THIS_ CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	return f_pD3DDeviceEx->CreatePixelShader(pFunction, ppShader);
}

HRESULT f_IDirect3DDevice9Ex::GetPixelShader(THIS_ IDirect3DPixelShader9** ppShader)
{
	return f_pD3DDeviceEx->GetPixelShader(ppShader);
}

HRESULT f_IDirect3DDevice9Ex::SetPixelShader(THIS_ IDirect3DPixelShader9* pShader)
{
	return f_pD3DDeviceEx->SetPixelShader(pShader);
}

HRESULT f_IDirect3DDevice9Ex::DrawIndexedPrimitive(THIS_ D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	return f_pD3DDeviceEx->DrawIndexedPrimitive(Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT f_IDirect3DDevice9Ex::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return f_pD3DDeviceEx->DrawIndexedPrimitiveUP(PrimitiveType, MinIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT f_IDirect3DDevice9Ex::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	return f_pD3DDeviceEx->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT f_IDirect3DDevice9Ex::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return f_pD3DDeviceEx->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT f_IDirect3DDevice9Ex::BeginScene()
{
	return f_pD3DDeviceEx->BeginScene();
}

HRESULT f_IDirect3DDevice9Ex::EndScene()
{
	return f_pD3DDeviceEx->EndScene();
}

HRESULT f_IDirect3DDevice9Ex::GetStreamSource(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride)
{
	return f_pD3DDeviceEx->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride);
}

HRESULT f_IDirect3DDevice9Ex::SetStreamSource(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	return f_pD3DDeviceEx->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT f_IDirect3DDevice9Ex::GetBackBuffer(THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	return f_pD3DDeviceEx->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT f_IDirect3DDevice9Ex::GetDepthStencilSurface(IDirect3DSurface9 **ppZStencilSurface)
{
	return f_pD3DDeviceEx->GetDepthStencilSurface(ppZStencilSurface);
}

HRESULT f_IDirect3DDevice9Ex::GetTexture(DWORD Stage, IDirect3DBaseTexture9 **ppTexture)
{
	return f_pD3DDeviceEx->GetTexture(Stage, ppTexture);
}

HRESULT f_IDirect3DDevice9Ex::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue)
{
	return f_pD3DDeviceEx->GetTextureStageState(Stage, Type, pValue);
}

HRESULT f_IDirect3DDevice9Ex::SetTexture(DWORD Stage, IDirect3DBaseTexture9 *pTexture)
{
	return f_pD3DDeviceEx->SetTexture(Stage, pTexture);
}

HRESULT f_IDirect3DDevice9Ex::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	return f_pD3DDeviceEx->SetTextureStageState(Stage, Type, Value);
}

HRESULT f_IDirect3DDevice9Ex::UpdateTexture(IDirect3DBaseTexture9 *pSourceTexture, IDirect3DBaseTexture9 *pDestinationTexture)
{
	return f_pD3DDeviceEx->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT f_IDirect3DDevice9Ex::ValidateDevice(DWORD *pNumPasses)
{
	return f_pD3DDeviceEx->ValidateDevice(pNumPasses);
}

HRESULT f_IDirect3DDevice9Ex::GetClipPlane(DWORD Index, float *pPlane)
{
	return f_pD3DDeviceEx->GetClipPlane(Index, pPlane);
}

HRESULT f_IDirect3DDevice9Ex::SetClipPlane(DWORD Index, CONST float *pPlane)
{
	return f_pD3DDeviceEx->SetClipPlane(Index, pPlane);
}

HRESULT f_IDirect3DDevice9Ex::Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	return f_pD3DDeviceEx->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT f_IDirect3DDevice9Ex::GetViewport(D3DVIEWPORT9 *pViewport)
{
	return f_pD3DDeviceEx->GetViewport(pViewport);
}

HRESULT f_IDirect3DDevice9Ex::SetViewport(CONST D3DVIEWPORT9 *pViewport)
{
	return f_pD3DDeviceEx->SetViewport(pViewport);
}

HRESULT f_IDirect3DDevice9Ex::CreateVertexShader(THIS_ CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
	return f_pD3DDeviceEx->CreateVertexShader(pFunction, ppShader);
}

HRESULT f_IDirect3DDevice9Ex::GetVertexShader(THIS_ IDirect3DVertexShader9** ppShader)
{
	return f_pD3DDeviceEx->GetVertexShader(ppShader);
}

HRESULT f_IDirect3DDevice9Ex::SetVertexShader(THIS_ IDirect3DVertexShader9* pShader)
{
	return f_pD3DDeviceEx->SetVertexShader(pShader);
}

HRESULT f_IDirect3DDevice9Ex::CreateQuery(THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	return f_pD3DDeviceEx->CreateQuery(Type, ppQuery);
}

HRESULT f_IDirect3DDevice9Ex::SetPixelShaderConstantB(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return f_pD3DDeviceEx->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT f_IDirect3DDevice9Ex::GetPixelShaderConstantB(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return f_pD3DDeviceEx->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT f_IDirect3DDevice9Ex::SetPixelShaderConstantI(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return f_pD3DDeviceEx->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT f_IDirect3DDevice9Ex::GetPixelShaderConstantI(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return f_pD3DDeviceEx->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT f_IDirect3DDevice9Ex::SetPixelShaderConstantF(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return f_pD3DDeviceEx->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT f_IDirect3DDevice9Ex::GetPixelShaderConstantF(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return f_pD3DDeviceEx->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT f_IDirect3DDevice9Ex::SetStreamSourceFreq(THIS_ UINT StreamNumber, UINT Divider)
{
	return f_pD3DDeviceEx->SetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT f_IDirect3DDevice9Ex::GetStreamSourceFreq(THIS_ UINT StreamNumber, UINT* Divider)
{
	return f_pD3DDeviceEx->GetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT f_IDirect3DDevice9Ex::SetVertexShaderConstantB(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return f_pD3DDeviceEx->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT f_IDirect3DDevice9Ex::GetVertexShaderConstantB(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return f_pD3DDeviceEx->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT f_IDirect3DDevice9Ex::SetVertexShaderConstantF(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return f_pD3DDeviceEx->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT f_IDirect3DDevice9Ex::GetVertexShaderConstantF(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return f_pD3DDeviceEx->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT f_IDirect3DDevice9Ex::SetVertexShaderConstantI(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return f_pD3DDeviceEx->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT f_IDirect3DDevice9Ex::GetVertexShaderConstantI(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return f_pD3DDeviceEx->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT f_IDirect3DDevice9Ex::SetFVF(THIS_ DWORD FVF)
{
	return f_pD3DDeviceEx->SetFVF(FVF);
}

HRESULT f_IDirect3DDevice9Ex::GetFVF(THIS_ DWORD* pFVF)
{
	return f_pD3DDeviceEx->GetFVF(pFVF);
}

HRESULT f_IDirect3DDevice9Ex::CreateVertexDeclaration(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
	return f_pD3DDeviceEx->CreateVertexDeclaration(pVertexElements, ppDecl);
}

HRESULT f_IDirect3DDevice9Ex::SetVertexDeclaration(THIS_ IDirect3DVertexDeclaration9* pDecl)
{
	return f_pD3DDeviceEx->SetVertexDeclaration(pDecl);
}

HRESULT f_IDirect3DDevice9Ex::GetVertexDeclaration(THIS_ IDirect3DVertexDeclaration9** ppDecl)
{
	return f_pD3DDeviceEx->GetVertexDeclaration(ppDecl);
}

HRESULT f_IDirect3DDevice9Ex::SetNPatchMode(THIS_ float nSegments)
{
	return f_pD3DDeviceEx->SetNPatchMode(nSegments);
}

float f_IDirect3DDevice9Ex::GetNPatchMode(THIS)
{
	return f_pD3DDeviceEx->GetNPatchMode();
}

int f_IDirect3DDevice9Ex::GetSoftwareVertexProcessing(THIS)
{
	return f_pD3DDeviceEx->GetSoftwareVertexProcessing();
}

unsigned int f_IDirect3DDevice9Ex::GetNumberOfSwapChains(THIS)
{
	return f_pD3DDeviceEx->GetNumberOfSwapChains();
}

HRESULT f_IDirect3DDevice9Ex::EvictManagedResources(THIS)
{
	return f_pD3DDeviceEx->EvictManagedResources();
}

HRESULT f_IDirect3DDevice9Ex::SetSoftwareVertexProcessing(THIS_ BOOL bSoftware)
{
	return f_pD3DDeviceEx->SetSoftwareVertexProcessing(bSoftware);
}

HRESULT f_IDirect3DDevice9Ex::SetScissorRect(THIS_ CONST RECT* pRect)
{
	return f_pD3DDeviceEx->SetScissorRect(pRect);
}

HRESULT f_IDirect3DDevice9Ex::GetScissorRect(THIS_ RECT* pRect)
{
	return f_pD3DDeviceEx->GetScissorRect(pRect);
}

HRESULT f_IDirect3DDevice9Ex::GetSamplerState(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	return f_pD3DDeviceEx->GetSamplerState(Sampler, Type, pValue);
}

HRESULT f_IDirect3DDevice9Ex::SetSamplerState(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	return f_pD3DDeviceEx->SetSamplerState(Sampler, Type, Value);
}

HRESULT f_IDirect3DDevice9Ex::SetDepthStencilSurface(THIS_ IDirect3DSurface9* pNewZStencil)
{
	return f_pD3DDeviceEx->SetDepthStencilSurface(pNewZStencil);
}

HRESULT f_IDirect3DDevice9Ex::CreateOffscreenPlainSurface(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return f_pD3DDeviceEx->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT f_IDirect3DDevice9Ex::ColorFill(THIS_ IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
{
	return f_pD3DDeviceEx->ColorFill(pSurface, pRect, color);
}

HRESULT f_IDirect3DDevice9Ex::StretchRect(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	return f_pD3DDeviceEx->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT f_IDirect3DDevice9Ex::GetFrontBufferData(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	return f_pD3DDeviceEx->GetFrontBufferData(iSwapChain, pDestSurface);
}

HRESULT f_IDirect3DDevice9Ex::GetRenderTargetData(THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	return f_pD3DDeviceEx->GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT f_IDirect3DDevice9Ex::UpdateSurface(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
{
	return f_pD3DDeviceEx->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT f_IDirect3DDevice9Ex::SetDialogBoxMode(THIS_ BOOL bEnableDialogs)
{
	return f_pD3DDeviceEx->SetDialogBoxMode(bEnableDialogs);
}

HRESULT f_IDirect3DDevice9Ex::GetSwapChain(THIS_ UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{
	return f_pD3DDeviceEx->GetSwapChain(iSwapChain, pSwapChain);
}

HRESULT f_IDirect3DDevice9Ex::SetConvolutionMonoKernel(UINT width, UINT height, float * rows, float * columns)
{
	return f_pD3DDeviceEx->SetConvolutionMonoKernel(width, height, rows, columns);
}

HRESULT f_IDirect3DDevice9Ex::ComposeRects(IDirect3DSurface9 * pSrc, IDirect3DSurface9 * pDst, IDirect3DVertexBuffer9 * pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9 * pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset)
{
	return f_pD3DDeviceEx->ComposeRects(pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset);
}

HRESULT f_IDirect3DDevice9Ex::GetGPUThreadPriority(INT * pPriority)
{
	return f_pD3DDeviceEx->GetGPUThreadPriority(pPriority);
}

HRESULT f_IDirect3DDevice9Ex::SetGPUThreadPriority(INT Priority)
{
	return f_pD3DDeviceEx->SetGPUThreadPriority(Priority);
}

HRESULT f_IDirect3DDevice9Ex::WaitForVBlank(UINT iSwapChain)
{
	return f_pD3DDeviceEx->WaitForVBlank(iSwapChain);
}

HRESULT f_IDirect3DDevice9Ex::CheckResourceResidency(IDirect3DResource9 ** pResourceArray, UINT32 NumResources)
{
	return f_pD3DDeviceEx->CheckResourceResidency(pResourceArray, NumResources);
}

HRESULT f_IDirect3DDevice9Ex::SetMaximumFrameLatency(UINT MaxLatency)
{
	return f_pD3DDeviceEx->SetMaximumFrameLatency(MaxLatency);
}

HRESULT f_IDirect3DDevice9Ex::GetMaximumFrameLatency(UINT * pMaxLatency)
{
	return f_pD3DDeviceEx->GetMaximumFrameLatency(pMaxLatency);
}

HRESULT f_IDirect3DDevice9Ex::CheckDeviceState(HWND hDestinationWindow)
{
	return f_pD3DDeviceEx->CheckDeviceState(hDestinationWindow);
}

HRESULT f_IDirect3DDevice9Ex::CreateRenderTargetEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9 ** ppSurface, HANDLE * pSharedHandle, DWORD Usage)
{
	return f_pD3DDeviceEx->CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);
}

HRESULT f_IDirect3DDevice9Ex::CreateOffscreenPlainSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9 ** ppSurface, HANDLE * pSharedHandle, DWORD Usage)
{
	return f_pD3DDeviceEx->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);
}

HRESULT f_IDirect3DDevice9Ex::CreateDepthStencilSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9 ** ppSurface, HANDLE * pSharedHandle, DWORD Usage)
{
	return f_pD3DDeviceEx->CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage);
}

HRESULT f_IDirect3DDevice9Ex::GetDisplayModeEx(UINT iSwapChain, D3DDISPLAYMODEEX * pMode, D3DDISPLAYROTATION * pRotation)
{
	return f_pD3DDeviceEx->GetDisplayModeEx(iSwapChain, pMode, pRotation);
}
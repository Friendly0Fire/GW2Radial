
typedef enum D3D9_wrapper_mode {
	WRAP_PASSTHRU = 0,
	WRAP_CB_PRE = 1,
	WRAP_CB_POST = 2,
	WRAP_CB_PRE_POST = 3
} D3D9_wrapper_mode;

typedef enum D3D9_wrapper_method {
	METH_DEV_QueryInterface,   
	METH_DEV_AddRef,   
	METH_DEV_Release,    
	METH_DEV_TestCooperativeLevel,   
	METH_DEV_GetAvailableTextureMem,   
	METH_DEV_EvictManagedResources,   
	METH_DEV_GetDirect3D,   
	METH_DEV_GetDeviceCaps,   
	METH_DEV_GetDisplayMode,   
	METH_DEV_GetCreationParameters,   
	METH_DEV_SetCursorProperties,   
	METH_DEV_SetCursorPosition,   
	METH_DEV_ShowCursor,   
	METH_DEV_CreateAdditionalSwapChain,   
	METH_DEV_GetSwapChain,   
	METH_DEV_GetNumberOfSwapChains,   
	METH_DEV_Reset,   
	METH_DEV_Present,   
	METH_DEV_GetBackBuffer,   
	METH_DEV_GetRasterStatus,   
	METH_DEV_SetDialogBoxMode,   
	METH_DEV_SetGammaRamp,   
	METH_DEV_GetGammaRamp,   
	METH_DEV_CreateTexture,   
	METH_DEV_CreateVolumeTexture,   
	METH_DEV_CreateCubeTexture,   
	METH_DEV_CreateVertexBuffer,   
	METH_DEV_CreateIndexBuffer,   
	METH_DEV_CreateRenderTarget,   
	METH_DEV_CreateDepthStencilSurface,   
	METH_DEV_UpdateSurface,   
	METH_DEV_UpdateTexture,   
	METH_DEV_GetRenderTargetData,   
	METH_DEV_GetFrontBufferData,   
	METH_DEV_StretchRect,   
	METH_DEV_ColorFill,   
	METH_DEV_CreateOffscreenPlainSurface,   
	METH_DEV_SetRenderTarget,   
	METH_DEV_GetRenderTarget,   
	METH_DEV_SetDepthStencilSurface,   
	METH_DEV_GetDepthStencilSurface,   
	METH_DEV_BeginScene,   
	METH_DEV_EndScene,   
	METH_DEV_Clear,   
	METH_DEV_SetTransform,   
	METH_DEV_GetTransform,   
	METH_DEV_MultiplyTransform,   
	METH_DEV_SetViewport,   
	METH_DEV_GetViewport,   
	METH_DEV_SetMaterial,   
	METH_DEV_GetMaterial,   
	METH_DEV_SetLight,   
	METH_DEV_GetLight,   
	METH_DEV_LightEnable,   
	METH_DEV_GetLightEnable,   
	METH_DEV_SetClipPlane,   
	METH_DEV_GetClipPlane,   
	METH_DEV_SetRenderState,   
	METH_DEV_GetRenderState,   
	METH_DEV_CreateStateBlock,   
	METH_DEV_BeginStateBlock,   
	METH_DEV_EndStateBlock,   
	METH_DEV_SetClipStatus,   
	METH_DEV_GetClipStatus,   
	METH_DEV_GetTexture,   
	METH_DEV_SetTexture,   
	METH_DEV_GetTextureStageState,   
	METH_DEV_SetTextureStageState,   
	METH_DEV_GetSamplerState,   
	METH_DEV_SetSamplerState,   
	METH_DEV_ValidateDevice,   
	METH_DEV_SetPaletteEntries,   
	METH_DEV_GetPaletteEntries,   
	METH_DEV_SetCurrentTexturePalette,   
	METH_DEV_GetCurrentTexturePalette,   
	METH_DEV_SetScissorRect,   
	METH_DEV_GetScissorRect,   
	METH_DEV_SetSoftwareVertexProcessing,   
	METH_DEV_GetSoftwareVertexProcessing,   
	METH_DEV_SetNPatchMode,   
	METH_DEV_GetNPatchMode,   
	METH_DEV_DrawPrimitive,   
	METH_DEV_DrawIndexedPrimitive,   
	METH_DEV_DrawPrimitiveUP,   
	METH_DEV_DrawIndexedPrimitiveUP,   
	METH_DEV_ProcessVertices,   
	METH_DEV_CreateVertexDeclaration,   
	METH_DEV_SetVertexDeclaration,   
	METH_DEV_GetVertexDeclaration,   
	METH_DEV_SetFVF,   
	METH_DEV_GetFVF,   
	METH_DEV_CreateVertexShader,   
	METH_DEV_SetVertexShader,   
	METH_DEV_GetVertexShader,   
	METH_DEV_SetVertexShaderConstantF,   
	METH_DEV_GetVertexShaderConstantF,   
	METH_DEV_SetVertexShaderConstantI,   
	METH_DEV_GetVertexShaderConstantI,   
	METH_DEV_SetVertexShaderConstantB,   
	METH_DEV_GetVertexShaderConstantB,   
	METH_DEV_SetStreamSource,   
	METH_DEV_GetStreamSource,   
	METH_DEV_SetStreamSourceFreq,   
	METH_DEV_GetStreamSourceFreq,   
	METH_DEV_SetIndices,   
	METH_DEV_GetIndices,   
	METH_DEV_CreatePixelShader,   
	METH_DEV_SetPixelShader,   
	METH_DEV_GetPixelShader,   
	METH_DEV_SetPixelShaderConstantF,   
	METH_DEV_GetPixelShaderConstantF,   
	METH_DEV_SetPixelShaderConstantI,   
	METH_DEV_GetPixelShaderConstantI,   
	METH_DEV_SetPixelShaderConstantB,   
	METH_DEV_GetPixelShaderConstantB,   
	METH_DEV_DrawRectPatch,   
	METH_DEV_DrawTriPatch,   
	METH_DEV_DeletePatch,   
	METH_DEV_CreateQuery,   
	METH_OBJ_QueryInterface,   
	METH_OBJ_AddRef,   
	METH_OBJ_Release,   
	METH_OBJ_RegisterSoftwareDevice,   
	METH_OBJ_GetAdapterCount,   
	METH_OBJ_GetAdapterIdentifier,   
	METH_OBJ_GetAdapterModeCount,   
	METH_OBJ_EnumAdapterModes,   
	METH_OBJ_GetAdapterDisplayMode,   
	METH_OBJ_CheckDeviceType,   
	METH_OBJ_CheckDeviceFormat,   
	METH_OBJ_CheckDeviceMultiSampleType,   
	METH_OBJ_CheckDepthStencilMatch,   
	METH_OBJ_CheckDeviceFormatConversion,   
	METH_OBJ_GetDeviceCaps,   
	METH_OBJ_GetAdapterMonitor,   
	METH_OBJ_CreateDevice,   
	METH_WRAP_COUNT
} D3D9_wrapper_method;

typedef struct D3D9_wrapper_event_data {
	void* ret;
	void** stackPtr;
} D3D9_wrapper_event_data;

typedef void (*pD3D9_wrapper_enable_event)(D3D9_wrapper_method method, D3D9_wrapper_mode mode);
typedef wchar_t* (*pD3D9_wrapper_custom_d3d9_lib_query)();

#define D3D9_WRAPPER_ENABLE_EVENT_FNAME L"D3D_wrapper_enable_event"
#define D3D9_WRAPPER_CUSTOM_D3D9_QUERY_FNAME L"D3D_wrapper_custom_d3d9_lib_query"

/*
	Event name format

	prefix        mode         vtable     method 
	D3D9_         POST_/PRE_   DEV_/OBJ_  SetTexture/AddRef
	
	Example: 
		D3D9_POST_DEV_Release 
		
		Triggers after Device->Release() API call execution
	
*/
#define D3D9_WRAPPER_WATCH_EVENT(subscriber,name,proc,priority) gAPI->watch_event(gAPI->query_event(gAPI->hash_name((wchar_t*)name)), gAPI->hash_name((wchar_t*)subscriber),(gw2al_api_event_handler)&proc, priority)

typedef struct D3D9_wrapper {
	pD3D9_wrapper_enable_event enable_event;
} D3D9_wrapper;

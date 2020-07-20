#include <Direct3D9Hooks.h>
#include <MinHook.h>
#include <Utility.h>
#include <tchar.h>
#include <filesystem>

namespace GW2Radial
{
template<typename Function>
struct Trampoline;

template<typename Ret, typename ...Args>
struct Trampoline<Ret(Direct3D9Hooks::*)(Args...)>
{
	using Function = Ret(Direct3D9Hooks::*)(Args...);

	template<Function f>
	static auto Eval(Args ...args)
	{
		return std::invoke(f, GetD3D9Hooks(), args...);
	}
};

#define D3DTRAMPOLINE(m) LPVOID(&Trampoline<decltype(&Direct3D9Hooks::m)>::Eval<&Direct3D9Hooks::m>)

Direct3D9Hooks::Direct3D9Hooks()
{
	MH_Initialize();
}

HRESULT Direct3D9Hooks::Present_hook(IDirect3DDevice9 *sThis, const RECT *pSourceRect, const RECT *pDestRect,
                                            HWND hDestWindowOverride, const RGNDATA *pDirtyRegion)
{
	drawOverCallback(sThis, isFrameDrawn_, true);
	isFrameDrawn_ = false;

	return Present_real(sThis, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT Direct3D9Hooks::PresentEx_hook(IDirect3DDevice9Ex *sThis, const RECT *pSourceRect,
                                                  const RECT *pDestRect, HWND hDestWindowOverride,
                                                  const RGNDATA *pDirtyRegion, DWORD dwFlags)
{
	drawOverCallback(sThis, isFrameDrawn_, true);
	isFrameDrawn_ = false;

	return PresentEx_real(sThis, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}

HRESULT Direct3D9Hooks::Reset_hook(IDirect3DDevice9 *sThis, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	preResetCallback();

	if (HRESULT hr = Reset_real(sThis, pPresentationParameters); FAILED(hr))
		return hr;

	postResetCallback(sThis, pPresentationParameters);

	return D3D_OK;
}

HRESULT Direct3D9Hooks::ResetEx_hook(IDirect3DDevice9Ex *sThis,
                                                D3DPRESENT_PARAMETERS *pPresentationParameters,
                                                D3DDISPLAYMODEEX *pFullscreenDisplayMode)
{
	preResetCallback();

	if (HRESULT hr = ResetEx_real(sThis, pPresentationParameters, pFullscreenDisplayMode); FAILED(hr))
		return hr;

	postResetCallback(sThis, pPresentationParameters);

	return D3D_OK;
}

ULONG Direct3D9Hooks::Release_hook(IDirect3DDevice9 *sThis)
{
	ULONG refcount = Release_real(sThis);

	if (refcount == 1)
	{
		preResetCallback();
		Release_real(sThis);
	}

	return refcount-1;
}

ULONG Direct3D9Hooks::AddRef_hook(IDirect3DDevice9 *sThis) 
{ 
	return AddRef_real(sThis)-1; 
}

HRESULT Direct3D9Hooks::CreateVertexShader_hook(IDirect3DDevice9 *sThis, const DWORD *pFunction,
                                                           IDirect3DVertexShader9 **ppShader)
{
	HRESULT hr = CreateVertexShader_real(sThis, pFunction, ppShader);

	if (!preUiVertexShader_)
	{
		int l = GetShaderFuncLength(pFunction);
		XXH64_hash_t hash = XXH64(pFunction, l, 0);
		if (hash == preUiVertexShaderHash_)
			preUiVertexShader_ = *ppShader;
	}

	return hr;
}

HRESULT Direct3D9Hooks::SetVertexShader_hook(IDirect3DDevice9 *sThis, IDirect3DVertexShader9 *pShader)
{
	HRESULT hr = SetVertexShader_real(sThis, pShader);

	if (!isInShaderHook_ && pShader && !isFrameDrawn_ && pShader == preUiVertexShader_)
	{
		isInShaderHook_ = true;
		drawUnderCallback(sThis, isFrameDrawn_, false);
		isInShaderHook_ = false;
		isFrameDrawn_ = true;
	}

	return hr;
}

HRESULT Direct3D9Hooks::CreatePixelShader_hook(IDirect3DDevice9 *sThis, const DWORD *pFunction,
                                                          IDirect3DPixelShader9 **ppShader)
{
	HRESULT hr = CreatePixelShader_real(sThis, pFunction, ppShader);

	if (!preUiPixelShader_)
	{
		int l = GetShaderFuncLength(pFunction);
		XXH64_hash_t hash = XXH64(pFunction, l, 0);
		if (hash == preUiPixelShaderHash_)
			preUiPixelShader_ = *ppShader;
	}

	return hr;
}

HRESULT Direct3D9Hooks::SetPixelShader_hook(IDirect3DDevice9 *sThis, IDirect3DPixelShader9 *pShader)
{
	HRESULT hr = SetPixelShader_real(sThis, pShader);

	if (!isInShaderHook_ && pShader && !isFrameDrawn_ && pShader == preUiPixelShader_)
	{
		isInShaderHook_ = true;
		drawUnderCallback(sThis, isFrameDrawn_, false);
		isInShaderHook_ = false;
		isFrameDrawn_ = true;
	}

	return hr;
}

HRESULT Direct3D9Hooks::CreateDevice_hook(IDirect3D9 *sThis, UINT Adapter, D3DDEVTYPE DeviceType,
                                                     HWND hFocusWindow, DWORD BehaviorFlags,
                                                     D3DPRESENT_PARAMETERS *pPresentationParameters,
                                                     IDirect3DDevice9 **ppReturnedDeviceInterface)
{
	preCreateDeviceCallback(hFocusWindow);

	//#define TEST_AMD
	#ifdef TEST_AMD
		for(UINT i = 0; i < sThis->GetAdapterCount(); i++)
		{
			D3DADAPTER_IDENTIFIER9 id;
			sThis->GetAdapterIdentifier(i, 0, &id);
			if(strstr(id.Description, "AMD"))
			{
				Adapter = i;
				break;
			}
		}
	#endif

	IDirect3DDevice9 *tempDevice = nullptr;
	if (const auto hr = CreateDevice_real(sThis, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters,
	                               &tempDevice); FAILED(hr))
		return hr;

	*ppReturnedDeviceInterface = tempDevice;

	auto vftd = GetVirtualFunctionTableD3DDevice9(tempDevice);

	MH_CreateHook(NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.Reset, vftd.Reset), D3DTRAMPOLINE(Reset_hook), (LPVOID*)&Reset_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.Present, vftd.Present), D3DTRAMPOLINE(Present_hook), (LPVOID*)&Present_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.CreateVertexShader, vftd.CreateVertexShader),
		D3DTRAMPOLINE(CreateVertexShader_hook), (LPVOID*)&CreateVertexShader_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.SetVertexShader, vftd.SetVertexShader), D3DTRAMPOLINE(SetVertexShader_hook),
		(LPVOID*)&SetVertexShader_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.CreatePixelShader, vftd.CreatePixelShader), D3DTRAMPOLINE(CreatePixelShader_hook),
		(LPVOID*)&CreatePixelShader_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.SetPixelShader, vftd.SetPixelShader), D3DTRAMPOLINE(SetPixelShader_hook),
		(LPVOID*)&SetPixelShader_real);

	tempDevice->AddRef();

	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.Release, vftd.Release), D3DTRAMPOLINE(Release_hook),
		(LPVOID*)&Release_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.AddRef, vftd.AddRef), D3DTRAMPOLINE(AddRef_hook),
		(LPVOID*)&AddRef_real);

	MH_EnableHook(MH_ALL_HOOKS);

	postCreateDeviceCallback(tempDevice, pPresentationParameters);

	return D3D_OK;
}

HRESULT Direct3D9Hooks::CreateDeviceEx_hook(IDirect3D9Ex *sThis, UINT Adapter, D3DDEVTYPE DeviceType,
                                                       HWND hFocusWindow, DWORD BehaviorFlags,
                                                       D3DPRESENT_PARAMETERS *pPresentationParameters,
                                                       D3DDISPLAYMODEEX *pFullscreenDisplayMode,
                                                       IDirect3DDevice9 **ppReturnedDeviceInterface)
{
	preCreateDeviceCallback(hFocusWindow);

	IDirect3DDevice9Ex *tempDevice = nullptr;
	
	if (const auto hr = CreateDeviceEx_real(sThis, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters,
	                                 pFullscreenDisplayMode, &tempDevice); FAILED(hr))
		return hr;

	*ppReturnedDeviceInterface = tempDevice;

	auto vftd = GetVirtualFunctionTableD3DDevice9Ex(tempDevice);

	MH_CreateHook(NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.Reset, vftd.Reset), D3DTRAMPOLINE(Reset_hook), (LPVOID*)&Reset_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.Present, vftd.Present), D3DTRAMPOLINE(Present_hook), (LPVOID*)&Present_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.ResetEx, vftd.ResetEx), D3DTRAMPOLINE(ResetEx_hook), (LPVOID*)&ResetEx_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.PresentEx, vftd.PresentEx), D3DTRAMPOLINE(PresentEx_hook),
		(LPVOID*)&PresentEx_real);
	//MH_CreateHook(NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.Release, vftd.Release), (LPVOID)&Release_hook, (LPVOID*)&Release_real);
	//MH_CreateHook(NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.AddRef, vftd.AddRef), (LPVOID)&AddRef_hook, (LPVOID*)&AddRef_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.CreateVertexShader, vftd.CreateVertexShader),
		D3DTRAMPOLINE(CreateVertexShader_hook), (LPVOID*)&CreateVertexShader_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.SetVertexShader, vftd.SetVertexShader), D3DTRAMPOLINE(SetVertexShader_hook),
		(LPVOID*)&SetVertexShader_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.CreatePixelShader, vftd.CreatePixelShader), D3DTRAMPOLINE(CreatePixelShader_hook),
		(LPVOID*)&CreatePixelShader_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.SetPixelShader, vftd.SetPixelShader), D3DTRAMPOLINE(SetPixelShader_hook),
		(LPVOID*)&SetPixelShader_real);

	tempDevice->AddRef();

	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.Release, vftd.Release), D3DTRAMPOLINE(Release_hook),
		(LPVOID*)&Release_real);
	MH_CreateHook(
		NULL_COALESCE(direct3DDevice9VirtualFunctionTable_.AddRef, vftd.AddRef), D3DTRAMPOLINE(AddRef_hook),
		(LPVOID*)&AddRef_real);

	MH_EnableHook(MH_ALL_HOOKS);

	postCreateDeviceCallback(tempDevice, pPresentationParameters);

	return D3D_OK;
}

void Direct3D9Hooks::OnD3DCreate()
{
	namespace fs = std::filesystem;

	cref basePath = fs::current_path();

	if (!realD3D9Module_)
	{
		auto path = basePath / L"bin64" / "d912pxy.dll";
		if (fs::exists(path))
			realD3D9Module_ = LoadLibrary(path.c_str());
		else {
			wchar_t sysPath[MAX_PATH];
			GetSystemDirectoryW(sysPath, MAX_PATH);

			path = fs::path(sysPath) / "d3d9.dll";

			realD3D9Module_ = LoadLibrary(path.c_str());
		}
	}

	if (!chainD3D9Module_)
	{
		for(cref path : {
		        basePath / "d3d9_mchain.dll",
			    basePath / "bin64" / "d3d9_mchain.dll",
			    basePath / "ReShade64.dll",
			    basePath / "bin64" / "ReShade64.dll"
		    }) {
			if(fs::exists(path)) {
		        chainD3D9Module_ = LoadLibraryW(path.c_str());
				break;
			}
		}
	}
}

D3DPRESENT_PARAMETERS Direct3D9Hooks::SetupHookDevice(HWND &hWnd)
{
	WNDCLASSEXA wc = { };
	wc.cbSize = sizeof(wc);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = DefWindowProc;
	wc.hInstance = GetModuleHandleA(nullptr);
	wc.lpszClassName = "DXTMP";
	RegisterClassExA(&wc);

	hWnd = CreateWindowA("DXTMP", 0, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), 0, wc.hInstance, 0);

	D3DPRESENT_PARAMETERS d3dPar = { };
	d3dPar.Windowed = TRUE;
	d3dPar.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dPar.hDeviceWindow = hWnd;
	d3dPar.BackBufferCount = 1;
	d3dPar.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dPar.BackBufferHeight = 300;
	d3dPar.BackBufferHeight = 300;

	return d3dPar;
}

void Direct3D9Hooks::DeleteHookDevice(IDirect3DDevice9 *pDev, HWND hWnd)
{
	COM_RELEASE(pDev);

	DestroyWindow(hWnd);
	UnregisterClassA("DXTMP", GetModuleHandleA(NULL));
}

void Direct3D9Hooks::LoadOriginalDevicePointers(IDirect3D9 *d3d)
{
	HWND hWnd;
	auto d3dpar = SetupHookDevice(hWnd);
	IDirect3DDevice9 *pDev;
	CreateDevice_real(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpar,
	                  &pDev);

	direct3DDevice9VirtualFunctionTable_ = GetVirtualFunctionTableD3DDevice9(pDev);

	DeleteHookDevice(pDev, hWnd);
}

void Direct3D9Hooks::LoadOriginalDevicePointers(IDirect3D9Ex *d3d)
{
	HWND hWnd;
	auto d3dpar = SetupHookDevice(hWnd);
	IDirect3DDevice9Ex *pDev;
	CreateDeviceEx_real(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpar,
	                    nullptr, &pDev);

	direct3DDevice9VirtualFunctionTable_ = GetVirtualFunctionTableD3DDevice9Ex(pDev);

	DeleteHookDevice(pDev, hWnd);
}

IDirect3D9* Direct3D9Hooks::Direct3DCreate9(UINT SDKVersion)
{
	OnD3DCreate();

	auto fDirect3DCreate9 = reinterpret_cast<Direct3DCreate9_t>(GetProcAddress(realD3D9Module_, "Direct3DCreate9"));
	auto d3d = fDirect3DCreate9(SDKVersion);

	if (!isDirect3DHooked_)
	{
		auto vft = GetVirtualFunctionTableD3D9(d3d);

		MH_CreateHook(vft.CreateDevice, D3DTRAMPOLINE(CreateDevice_hook), (LPVOID*)&CreateDevice_real);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	if (chainD3D9Module_)
	{
		LoadOriginalDevicePointers(d3d);
		d3d->Release();

		fDirect3DCreate9 = reinterpret_cast<Direct3DCreate9_t>(GetProcAddress(chainD3D9Module_, "Direct3DCreate9"));
		d3d = fDirect3DCreate9(SDKVersion);
	}

	isDirect3DHooked_ = true;

	return d3d;
}

HRESULT Direct3D9Hooks::Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** output)
{
	OnD3DCreate();

	auto fDirect3DCreate9 = reinterpret_cast<Direct3DCreate9Ex_t>(GetProcAddress(realD3D9Module_, "Direct3DCreate9Ex"));
	IDirect3D9Ex* d3d = nullptr;
	if(HRESULT hr = fDirect3DCreate9(SDKVersion, &d3d); FAILED(hr))
		return hr;

	if (!isDirect3DHooked_)
	{
		auto vft = GetVirtualFunctionTableD3D9Ex(d3d);

		MH_CreateHook(vft.CreateDevice, D3DTRAMPOLINE(CreateDevice_hook), (LPVOID*)&CreateDevice_real);
		MH_CreateHook(vft.CreateDeviceEx, D3DTRAMPOLINE(CreateDeviceEx_hook), (LPVOID*)&CreateDeviceEx_real);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	if (chainD3D9Module_)
	{
		d3d->Release();

		fDirect3DCreate9 = reinterpret_cast<Direct3DCreate9Ex_t>(GetProcAddress(chainD3D9Module_, "Direct3DCreate9Ex"));
		if(HRESULT hr = fDirect3DCreate9(SDKVersion, &d3d); FAILED(hr))
			return hr;
	}

	*output = d3d;

	return D3D_OK;
}
}


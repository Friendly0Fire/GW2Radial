#pragma once

#include <Main.h>
#include <Singleton.h>
#include <d3dx9.h>

namespace GW2Addons
{

class UnitQuad;

class Core : public Singleton<Core>
{
public:
	static void Init(HMODULE dll);
	static void Shutdown();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	~Core();

	HWND gameWindow() const { return gameWindow_; }
	HMODULE dllModule() const { return dllModule_; }
	WNDPROC baseWndProc() const { return baseWndProc_; }
	uint screenWidth() const { return screenWidth_; }
	uint screenHeight() const { return screenHeight_; }
	const std::unique_ptr<UnitQuad>& quad() const { return quad_; }
	ID3DXEffect* mainEffect() const { return mainEffect_; }

protected:
	void InternalInit();
	void OnFocusLost();

	void OnDeviceSet(IDirect3DDevice9 *device, D3DPRESENT_PARAMETERS *presentationParameters);
	void OnDeviceUnset();

	void PreCreateDevice(HWND hFocusWindow);
	void PostCreateDevice(IDirect3DDevice9 *device, D3DPRESENT_PARAMETERS *presentationParameters);

	void PreReset();
	void PostReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS *presentationParameters);

	void DrawOver(IDirect3DDevice9* device, bool frameDrawn, bool sceneEnded);

	HWND gameWindow_ = nullptr;
	HMODULE dllModule_ = nullptr;
	WNDPROC baseWndProc_ = nullptr;

	uint screenWidth_ = 0, screenHeight_ = 0;
	bool firstFrame_ = true;

	std::unique_ptr<UnitQuad> quad_;
	ID3DXEffect* mainEffect_ = nullptr;

	std::unique_ptr<class Wheel> wheelMounts_, wheelNovelties_;
};
}

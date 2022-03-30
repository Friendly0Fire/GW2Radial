#pragma once

#include <Main.h>
#include <Singleton.h>
#include <Wheel.h>
#include <CustomWheel.h>
#include <Defs.h>
#include <d3d11_1.h>
#include <dxgi.h>

class ShaderManager;

namespace GW2Radial
{

class Core : public BaseCore, public Singleton<Core>
{
public:
	static void Init(HMODULE dll);
	static void Shutdown();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	Core() = default;
	~Core();

	void ForceReloadWheels() { forceReloadWheels_ = true; }

	const auto& wheels() const { return wheels_; }

	void OnInjectorCreated();

	void OnInputLanguageChange();

	UINT GetDpiForWindow(HWND hwnd);

	void Draw();

protected:
	void InternalInit();
	void OnFocusLost();
	void OnFocus();
	void OnUpdate();

	void PostCreateSwapChain(HWND hwnd, ID3D11Device* device, IDXGISwapChain* swc);
	void PreResizeSwapChain();
	void PostResizeSwapChain(uint w, uint h);

	bool firstFrame_ = true;
	bool forceReloadWheels_ = false;
	uint mapId_ = 0;
	std::wstring characterName_;
	uint tickSkip_ = 0;
	const uint TickSkipCount = 10;
	uint longTickSkip_ = 0;
	const uint LongTickSkipCount = 600;

	std::vector<std::unique_ptr<Wheel>> wheels_;
	std::unique_ptr<CustomWheelsManager> customWheels_;

	std::unique_ptr<ConfigurationOption<bool>> firstMessageShown_;

	ImGuiContext* imguiContext_ = nullptr;

	using GetDpiForWindow_t = UINT (WINAPI *)(HWND hwnd);
	HMODULE user32_ = 0;
	GetDpiForWindow_t getDpiForWindow_ = nullptr;

	ComPtr<ID3DUserDefinedAnnotation> annotations_;

	ComPtr<ID3D11RenderTargetView> backBufferRTV_;
};
}

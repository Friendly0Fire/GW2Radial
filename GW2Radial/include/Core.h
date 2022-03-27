#pragma once

#include <Main.h>
#include <Singleton.h>
#include <Wheel.h>
#include <CustomWheel.h>
#include <Defs.h>
#include <d3d11_1.h>
#include <dxgi.h>

struct RENDERDOC_API_1_5_0;

class ShaderManager;

namespace GW2Radial
{

class Core : public Singleton<Core>
{
public:
	static void Init(HMODULE dll);
	static void Shutdown();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	Core() = default;
	~Core();

	void ForceReloadWheels() { forceReloadWheels_ = true; }

	auto gameWindow() const { return gameWindow_; }
	auto dllModule() const { return dllModule_; }
	auto baseWndProc() const { return baseWndProc_; }
	auto screenWidth() const { return screenWidth_; }
	auto screenHeight() const { return screenHeight_; }
	auto font() const { return font_; }
	auto fontBlack() const { return fontBlack_; }
	auto fontItalic() const { return fontItalic_; }
	auto fontIcon() const { return fontIcon_; }
	auto fontMono() const { return fontMono_; }

	auto rdoc() const { return rdoc_; }
	auto device() const { return device_; }

	const auto& wheels() const { return wheels_; }

	void OnInjectorCreated();

	void OnInputLanguageChange();

	void RegisterOnInputLanguageChange(InputLanguageChangeListener* ilcl) { ilcListeners_.push_back(ilcl); }
	void UnregisterOnInputLanguageChange(InputLanguageChangeListener* ilcl) { auto it = std::find(ilcListeners_.begin(), ilcListeners_.end(), ilcl); if (it != ilcListeners_.end()) ilcListeners_.erase(it); }

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


	HWND gameWindow_ = nullptr;
	HMODULE dllModule_ = nullptr;
	WNDPROC baseWndProc_ = nullptr;
	ComPtr<ID3D11Device> device_;
	ComPtr<ID3D11DeviceContext> context_;
	ComPtr<IDXGISwapChain> swc_;

	uint screenWidth_ = 0, screenHeight_ = 0;
	bool firstFrame_ = true;
	bool forceReloadWheels_ = false;
	uint mapId_ = 0;
	std::wstring characterName_;
	uint tickSkip_ = 0;
	const uint TickSkipCount = 10;
	uint longTickSkip_ = 0;
	const uint LongTickSkipCount = 600;
	std::list<InputLanguageChangeListener*> ilcListeners_;

	ImFont *font_ = nullptr, *fontBlack_ = nullptr, *fontItalic_ = nullptr, *fontDraw_ = nullptr, *fontIcon_ = nullptr, *fontMono_ = nullptr;

	std::vector<std::unique_ptr<Wheel>> wheels_;
	std::unique_ptr<CustomWheelsManager> customWheels_;

	std::unique_ptr<ConfigurationOption<bool>> firstMessageShown_;

	ImGuiContext* imguiContext_ = nullptr;

	using GetDpiForWindow_t = UINT (WINAPI *)(HWND hwnd);
	HMODULE user32_ = 0;
	GetDpiForWindow_t getDpiForWindow_ = nullptr;

	ComPtr<ID3DUserDefinedAnnotation> annotations_;

	ComPtr<ID3D11RenderTargetView> backBufferRTV_;

	RENDERDOC_API_1_5_0* rdoc_ = nullptr;
};
}

#pragma once

#include <Main.h>
#include <Singleton.h>
#include <Wheel.h>
#include <CustomWheel.h>
#include <Defs.h>
#include <d3d11.h>
#include <dxgi.h>

namespace GW2Radial
{

class ShaderManager;

class Core : public Singleton<Core>
{
public:
	static void Init(HMODULE dll);
	static void Shutdown();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	Core() = default;
	~Core();

	void ForceReloadWheels() { forceReloadWheels_ = true; }

	HWND gameWindow() const { return gameWindow_; }
	HMODULE dllModule() const { return dllModule_; }
	WNDPROC baseWndProc() const { return baseWndProc_; }
	uint screenWidth() const { return screenWidth_; }
	uint screenHeight() const { return screenHeight_; }
	ImFont* font() const { return font_; }
	ImFont* fontBlack() const { return fontBlack_; }
	ImFont* fontItalic() const { return fontItalic_; }
	ImFont* fontIcon() const { return fontIcon_; }
	ImFont* fontMono() const { return fontMono_; }

	const std::vector<std::unique_ptr<Wheel>>& wheels() const { return wheels_; }

	void OnInjectorCreated();

	void OnInputLanguageChange();

	void RegisterOnInputLanguageChange(InputLanguageChangeListener* ilcl) { ilcListeners_.push_back(ilcl); }
	void UnregisterOnInputLanguageChange(InputLanguageChangeListener* ilcl) { auto it = std::find(ilcListeners_.begin(), ilcListeners_.end(), ilcl); if (it != ilcListeners_.end()) ilcListeners_.erase(it); }

	UINT GetDpiForWindow(HWND hwnd);

protected:
	void InternalInit();
	void OnFocusLost();
	void OnFocus();
	void OnUpdate();

	void Draw();

	void PreCreateSwapChain(HWND hwnd);
	void PostCreateSwapChain(ID3D11Device* device, IDXGISwapChain* swc);
	

	HWND gameWindow_ = nullptr;
	HMODULE dllModule_ = nullptr;
	WNDPROC baseWndProc_ = nullptr;
	ID3D11Device* device_ = nullptr;
	ID3D11DeviceContext* context_ = nullptr;

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

	using GetDpiForWindow_t = decltype(::GetDpiForWindow)*;
	HMODULE user32_ = 0;
	GetDpiForWindow_t getDpiForWindow_ = nullptr;
};
}

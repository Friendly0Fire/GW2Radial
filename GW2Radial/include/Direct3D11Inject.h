#pragma once

#include <Main.h>
#include <xxhash.h>
#include <d3d11.h>
#include <functional>
#include <Singleton.h>

namespace GW2Radial
{

class Direct3D11Inject : public Singleton<Direct3D11Inject>
{
public:
	using PrePresentSwapChainCallback = std::function<void()>;
	using PreResizeSwapChainCallback = std::function<void()>;
	using PostResizeSwapChainCallback = std::function<void(uint, uint)>;
	using PostCreateSwapChainCallback = std::function<void(HWND, ID3D11Device*, IDXGISwapChain*)>;

	PrePresentSwapChainCallback prePresentSwapChainCallback;
	PostCreateSwapChainCallback postCreateSwapChainCallback;
	PreResizeSwapChainCallback preResizeSwapChainCallback;
	PostResizeSwapChainCallback postResizeSwapChainCallback;

protected:
	Direct3D11Inject() = default;

	bool isFrameDrawn_ = false;
	bool isDirect3DHooked_ = false;
};

}

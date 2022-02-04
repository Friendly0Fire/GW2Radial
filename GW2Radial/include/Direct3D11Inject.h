#pragma once

#include <Main.h>
#include <xxhash/xxhash.h>
#include <d3d9.h>
#include <functional>
#include <Singleton.h>

namespace GW2Radial
{

class Direct3D11Inject : public Singleton<Direct3D11Inject, false>
{
public:
	using DrawCallback = std::function<void(ID3D11Device*, bool, bool)>;
	using PreCreateDeviceCallback = std::function<void(HWND)>;
	using PostCreateDeviceCallback = std::function<void(ID3D11Device*)>;
	using PostCreateSwapChainCallback = std::function<void(IDXGISwapChain*)>;

	DrawCallback drawCallback;
	PreCreateDeviceCallback preCreateDeviceCallback;
	PostCreateDeviceCallback postCreateDeviceCallback;

protected:
	Direct3D11Inject() = default;

	bool isFrameDrawn_ = false;
	bool isDirect3DHooked_ = false;
};

}

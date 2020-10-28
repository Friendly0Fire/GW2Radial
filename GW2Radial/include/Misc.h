#pragma once
#include <d3d9.h>

inline bool IsD912Pxy(IDirect3DDevice9* device) {
    return device->SetRenderState((D3DRENDERSTATETYPE)220, 1) == 343434;
}

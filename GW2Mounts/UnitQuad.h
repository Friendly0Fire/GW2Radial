#pragma once
#include "main.h"
#include <d3d9.h>
#include <d3dx9.h>

struct ScreenVertex
{
	D3DXVECTOR2 uv;
};

struct UnitQuad
{
	UnitQuad(IDirect3DDevice9* device);
	~UnitQuad();
	ScreenVertex points[4];
	static uint size() { return sizeof(ScreenVertex) * 4; }
	static uint stride() { return sizeof(ScreenVertex); }

	static const D3DVERTEXELEMENT9* def();

	void Bind(uint stream = 0, uint offset = 0);
	void Draw(uint triCount = 2, uint startVert = 0);

private:
	IDirect3DDevice9* _device = nullptr;
	IDirect3DVertexDeclaration9* _vd = nullptr;
	IDirect3DVertexBuffer9* _buffer = nullptr;
};
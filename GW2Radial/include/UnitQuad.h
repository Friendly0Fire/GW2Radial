#pragma once
#include <Main.h>
#include <d3d9.h>
#include <d3dx9.h>

namespace GW2Radial
{

struct ScreenVertex
{
	D3DXVECTOR2 uv;
};

class UnitQuad
{
public:
	explicit UnitQuad(IDirect3DDevice9* device);
	UnitQuad(const UnitQuad& uq) = delete;
	UnitQuad& operator=(UnitQuad uq) = delete;
	UnitQuad(UnitQuad&& uq) = delete;
	UnitQuad& operator=(UnitQuad&& uq) = delete;
	~UnitQuad();

	ScreenVertex points[4];
	static uint size() { return sizeof(ScreenVertex) * 4; }
	static uint stride() { return sizeof(ScreenVertex); }

	static const D3DVERTEXELEMENT9* def();

	void Bind(uint stream = 0, uint offset = 0) const;
	void Draw(uint triCount = 2, uint startVert = 0) const;

private:
	IDirect3DDevice9* device_ = nullptr;
	IDirect3DVertexDeclaration9* vertexDeclaration_ = nullptr;
	IDirect3DVertexBuffer9* buffer_ = nullptr;
};

}
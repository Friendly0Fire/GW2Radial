#include "UnitQuad.h"

const D3DVERTEXELEMENT9 ScreenVertexDefinition[2] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};

UnitQuad::UnitQuad(IDirect3DDevice9* device)
	: _device(device)
{
	points[0].uv = D3DXVECTOR2(0.f, 0.f);
	points[1].uv = D3DXVECTOR2(1.f, 0.f);
	points[2].uv = D3DXVECTOR2(0.f, 1.f);
	points[3].uv = D3DXVECTOR2(1.f, 1.f);

	_device->CreateVertexDeclaration(UnitQuad::def(), &_vd);

	_device->CreateVertexBuffer(size(), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_buffer, nullptr);

	LPVOID ptr = nullptr;
	_buffer->Lock(0, 0, &ptr, 0);
	CopyMemory(ptr, points, size());
	_buffer->Unlock();
}

const D3DVERTEXELEMENT9 * UnitQuad::def()
{
	return ScreenVertexDefinition;
}

void UnitQuad::Bind(uint stream, uint offset)
{
	_device->SetVertexDeclaration(_vd);

	_device->SetStreamSource(stream, _buffer, offset, stride());
}

void UnitQuad::Draw()
{
	_device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}

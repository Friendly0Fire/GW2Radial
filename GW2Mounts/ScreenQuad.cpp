#include "ScreenQuad.h"

const D3DVERTEXELEMENT9 ScreenVertexDefinition[3] =
{
	{ 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
	{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 },
	D3DDECL_END()
};

ScreenQuad::ScreenQuad(IDirect3DDevice9* device, uint width, uint height)
	: _device(device)
{
	float fwidth = width - .5f;
	float fheight = height - .5f;

	points[0].position = D3DXVECTOR4(-.5f, -.5f, .5f, 1.f);
	points[1].position = D3DXVECTOR4(fwidth, -.5f, .5f, 1.f);
	points[2].position = D3DXVECTOR4(-.5f, fheight, .5f, 1.f);
	points[3].position = D3DXVECTOR4(fwidth, fheight, .5f, 1.f);

	points[0].uv = D3DXVECTOR2(0.f, 0.f);
	points[1].uv = D3DXVECTOR2(1.f, 0.f);
	points[2].uv = D3DXVECTOR2(0.f, 1.f);
	points[3].uv = D3DXVECTOR2(1.f, 1.f);

	_device->CreateVertexDeclaration(ScreenQuad::def(), &_vd);

	_device->CreateVertexBuffer(size(), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_buffer, nullptr);

	LPVOID ptr = nullptr;
	_buffer->Lock(0, 0, &ptr, 0);
	CopyMemory(ptr, points, size());
	_buffer->Unlock();
}

const D3DVERTEXELEMENT9 * ScreenQuad::def()
{
	return ScreenVertexDefinition;
}

void ScreenQuad::Bind(uint stream, uint offset)
{
	_device->SetVertexDeclaration(_vd);

	_device->SetStreamSource(stream, _buffer, offset, stride());
}
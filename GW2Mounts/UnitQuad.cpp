#include "UnitQuad.h"

const D3DVERTEXELEMENT9 ScreenVertexDefinition[2] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};

UnitQuad::UnitQuad(IDirect3DDevice9* device)
	: _device(device)
{
	if (!_device)
		throw std::exception();

	_device->AddRef();

	points[0].uv = D3DXVECTOR2(0.f, 0.f);
	points[1].uv = D3DXVECTOR2(1.f, 0.f);
	points[2].uv = D3DXVECTOR2(0.f, 1.f);
	points[3].uv = D3DXVECTOR2(1.f, 1.f);

	HRESULT hr = _device->CreateVertexDeclaration(UnitQuad::def(), &_vd);
	if (FAILED(hr))
		throw std::exception();

	hr = _device->CreateVertexBuffer(size(), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_buffer, nullptr);
	if (FAILED(hr))
		throw std::exception();

	LPVOID ptr = nullptr;
	hr = _buffer->Lock(0, 0, &ptr, 0);

	if (FAILED(hr))
		throw std::exception();

	CopyMemory(ptr, points, size());

	hr = _buffer->Unlock();

	if (FAILED(hr))
		throw std::exception();
}

UnitQuad::~UnitQuad()
{
	COM_RELEASE(_device);
	COM_RELEASE(_vd);
	COM_RELEASE(_buffer);
}

const D3DVERTEXELEMENT9 * UnitQuad::def()
{
	return ScreenVertexDefinition;
}

void UnitQuad::Bind(uint stream, uint offset)
{
	if (!_device || !_vd || !_buffer)
		return;

	_device->SetVertexDeclaration(_vd);

	_device->SetStreamSource(stream, _buffer, offset, stride());
}

void UnitQuad::Draw(uint triCount, uint startVert)
{
	if (!_device)
		return;

	_device->DrawPrimitive(D3DPT_TRIANGLESTRIP, startVert, triCount);
}

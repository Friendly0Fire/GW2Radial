#include <UnitQuad.h>

namespace GW2Radial
{

const D3DVERTEXELEMENT9 ScreenVertexDefinition[2] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};

UnitQuad::UnitQuad(IDirect3DDevice9* device)
	: device_(device)
{
	if (!device_)
		throw std::exception();

	device_->AddRef();

	points[0].uv = D3DXVECTOR2(0.f, 0.f);
	points[1].uv = D3DXVECTOR2(1.f, 0.f);
	points[2].uv = D3DXVECTOR2(0.f, 1.f);
	points[3].uv = D3DXVECTOR2(1.f, 1.f);

	HRESULT hr = device_->CreateVertexDeclaration(def(), &vertexDeclaration_);
	if (FAILED(hr))
		throw std::exception();

	hr = device_->CreateVertexBuffer(size(), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &buffer_, nullptr);
	if (FAILED(hr))
		throw std::exception();

	LPVOID ptr = nullptr;
	hr = buffer_->Lock(0, 0, &ptr, 0);

	if (FAILED(hr))
		throw std::exception();

	CopyMemory(ptr, points, size());

	hr = buffer_->Unlock();

	if (FAILED(hr))
		throw std::exception();
}

UnitQuad::~UnitQuad()
{
	COM_RELEASE(device_);
	COM_RELEASE(vertexDeclaration_);
	COM_RELEASE(buffer_);
}

const D3DVERTEXELEMENT9 * UnitQuad::def()
{
	return ScreenVertexDefinition;
}

void UnitQuad::Bind(uint stream, uint offset) const
{
	if (!device_ || !vertexDeclaration_ || !buffer_)
		return;

	device_->SetVertexDeclaration(vertexDeclaration_);

	device_->SetStreamSource(stream, buffer_, offset, stride());
}

void UnitQuad::Draw(uint triCount, uint startVert) const
{
	if (!device_)
		return;

	device_->DrawPrimitive(D3DPT_TRIANGLESTRIP, startVert, triCount);
}

}
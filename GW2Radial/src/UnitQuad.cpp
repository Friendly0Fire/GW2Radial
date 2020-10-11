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

	points[0].uv = { 0.f, 0.f };
	points[1].uv = { 1.f, 0.f };
	points[2].uv = { 0.f, 1.f };
	points[3].uv = { 1.f, 1.f };

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

	hr = device_->CreateIndexBuffer(size(), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &ind_buffer_, nullptr);
	if (FAILED(hr))
		throw std::exception();

	ptr = nullptr;
	hr = ind_buffer_->Lock(0, 0, &ptr, 0);

	if (FAILED(hr))
		throw std::exception();

	WORD indexes[6] = { 0, 1, 2, 2, 1, 3 };
	CopyMemory(ptr, indexes, 6*2);

	hr = ind_buffer_->Unlock();

	if (FAILED(hr))
		throw std::exception();
}

UnitQuad::~UnitQuad()
{
}

const D3DVERTEXELEMENT9 * UnitQuad::def()
{
	return ScreenVertexDefinition;
}

void UnitQuad::Bind(Effect* fx, uint stream, uint offset) const
{
	if (!device_ || !vertexDeclaration_ || !buffer_)
		return;

	device_->SetStreamSource(stream, buffer_.Get(), offset, stride());
	device_->SetIndices(ind_buffer_.Get());

	fx->OnBind(vertexDeclaration_.Get());
}

void UnitQuad::Draw(uint triCount, uint startVert) const
{
	if (!device_)
		return;

	device_->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, startVert, 0, 4, 0, triCount);
}

}
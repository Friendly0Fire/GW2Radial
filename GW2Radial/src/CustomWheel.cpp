#include <CustomWheel.h>
#include <Wheel.h>
#include "DDSTextureLoader.h"

namespace GW2Radial
{

CustomWheelsManager::CustomWheelsManager(std::vector<std::unique_ptr<Wheel>>& wheels, IDirect3DDevice9* dev)
{
}

IDirect3DTexture9* LoadCustomTexture(IDirect3DDevice9* dev, const std::wstring& path)
{
	IDirect3DTexture9* tex = nullptr;
    GW2_ASSERT(SUCCEEDED(CreateDDSTextureFromFile(dev, path.c_str(), &tex)));
	return tex;
}

CustomElement::CustomElement(const CustomElementSettings& ces, IDirect3DDevice9* dev)
	: WheelElement(ces.id, ces.nickname, ces.category, ces.name, dev, LoadCustomTexture(dev, ces.texturePath)), color_(ces.color)
{ }

template<>
void Wheel::Setup<CustomElement>(IDirect3DDevice9* dev)
{
	CustomElementSettings ces;


	AddElement(std::make_unique<CustomElement>(ces, dev));
}

fVector4 CustomElement::color()
{
	return color_;
}

}
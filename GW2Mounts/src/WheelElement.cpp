#include <WheelElement.h>
#include <Core.h>

namespace GW2Addons
{

WheelElement::WheelElement(uint id, std::string nickname, std::string displayName, IDirect3DDevice9* dev)
	: elementId_(id), keybind_(nickname, displayName)
{
	D3DXCreateTextureFromResource(dev, Core::i()->dllModule(), MAKEINTRESOURCE(elementId_), &appearance_);
}

WheelElement::~WheelElement()
{
	COM_RELEASE(appearance_);
}

}
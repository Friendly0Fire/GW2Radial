#include <Wheel.h>
#include <Core.h>

namespace GW2Addons
{

Wheel::Wheel(uint resourceId, std::string nickname, std::string displayName, IDirect3DDevice9 * dev)
	: keybind_(displayName, nickname), centralKeybind_(displayName + " (Center Locked)", nickname + "_cl")
{
	D3DXCreateTextureFromResource(dev, Core::i()->dllModule(), MAKEINTRESOURCE(resourceId), &appearance_);
}

Wheel::~Wheel()
{
	COM_RELEASE(appearance_);
}
}
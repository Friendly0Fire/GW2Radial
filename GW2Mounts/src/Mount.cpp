#include <Mount.h>
#include <Wheel.h>

namespace GW2Addons
{

Mount::Mount(MountType m, IDirect3DDevice9* dev)
	: WheelElement(uint(m), std::string("mount") + GetMountNameFromType(m), GetMountNameFromType(m), dev)
{ }

void Mount::AddAllMounts(Wheel* w, IDirect3DDevice9* dev)
{
	for(auto i = MountType::FIRST; i <= MountType::LAST; i = MountType(uint(i) + 1))
		w->AddElement(std::make_unique<Mount>(i, dev));
}

}
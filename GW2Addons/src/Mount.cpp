#include <Mount.h>
#include <Wheel.h>

namespace GW2Addons
{

Mount::Mount(MountType m, IDirect3DDevice9* dev)
	: WheelElement(uint(m), std::string("mount_") + GetMountNicknameFromType(m), GetMountNameFromType(m), dev)
{ }

void Mount::AddAllMounts(Wheel* w, IDirect3DDevice9* dev)
{
	for(auto i = MountType::FIRST; i <= MountType::LAST; i = MountType(uint(i) + 1))
		w->AddElement(std::make_unique<Mount>(i, dev));
}

std::array<float, 4> Mount::color()
{
	auto mt = MountType(elementId_);
	switch(mt)
	{
	case MountType::RAPTOR:
		return { 213 / 255.f, 100 / 255.f, 89 / 255.f, 1 };
	case MountType::SPRINGER:
		return { 212 / 255.f, 198 / 255.f, 94 / 255.f, 1 };
	case MountType::SKIMMER:
		return { 108 / 255.f, 128 / 255.f, 213 / 255.f, 1 };
	case MountType::JACKAL:
		return { 120 / 255.f, 183 / 255.f, 197 / 255.f, 1 };
	case MountType::BEETLE:
		return { 199 / 255.f, 131 / 255.f, 68 / 255.f, 1 };
	case MountType::GRIFFON:
		return { 136 / 255.f, 123 / 255.f, 195 / 255.f, 1 };
	default:
		return { 1, 1, 1, 1 };
	}
}

}
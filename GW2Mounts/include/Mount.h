#pragma once
#include <Main.h>
#include <WheelElement.h>

namespace GW2Addons
{

enum class MountType : uint
{
	NONE = 0xFFFFFFFF,
	RAPTOR = IDR_MOUNT1,
	SPRINGER = IDR_MOUNT2,
	SKIMMER = IDR_MOUNT3,
	JACKAL = IDR_MOUNT4,
	BEETLE = IDR_MOUNT5,
	GRIFFON = IDR_MOUNT6,

	FIRST = RAPTOR,
	LAST = GRIFFON
};
const unsigned int MountTypeCount = 6;

class Mount : public WheelElement
{
public:
	Mount(MountType m, IDirect3DDevice9* dev);

	static void AddAllMounts(class Wheel* w, IDirect3DDevice9* dev);

protected:
	static const char* GetMountNameFromType(MountType m)
	{
		switch (m)
		{
		case MountType::RAPTOR:
			return "Raptor";
		case MountType::SPRINGER:
			return "Springer";
		case MountType::SKIMMER:
			return "Skimmer";
		case MountType::JACKAL:
			return "Jackal";
		case MountType::BEETLE:
			return "Beetle";
		case MountType::GRIFFON:
			return "Griffon";
		default:
			return "[Unknown]";
		}
	}

	std::array<float, 4> color() override;
};

}
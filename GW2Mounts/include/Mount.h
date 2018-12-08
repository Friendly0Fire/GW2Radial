#pragma once
#include <Main.h>
#include <WheelElement.h>

namespace GW2Addons
{

enum class MountType : uint
{
	NONE = (uint)-1,
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

const std::array<float, 4> MountColors[MountTypeCount] = {
	{ 213 / 255.f, 100 / 255.f, 89 / 255.f, 1 },
	{ 212 / 255.f, 198 / 255.f, 94 / 255.f, 1 },
	{ 108 / 255.f, 128 / 255.f, 213 / 255.f, 1 },
	{ 120 / 255.f, 183 / 255.f, 197 / 255.f, 1 },
	{ 199 / 255.f, 131 / 255.f, 68 / 255.f, 1 },
	{ 136 / 255.f, 123 / 255.f, 195 / 255.f, 1 }
};

class Mount : public WheelElement
{
public:
	const char* name() const override { return GetMountNameFromType(static_cast<MountType>(elementId_)); }
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
};

}
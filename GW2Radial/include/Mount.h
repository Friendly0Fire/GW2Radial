#pragma once
#include <Main.h>
#include <WheelElement.h>

namespace GW2Radial
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
	WARCLAW = IDR_MOUNT7,
	SKYSCALE = IDR_MOUNT8,

	FIRST = RAPTOR,
	LAST = SKYSCALE
};
const unsigned int MountTypeCount = 6;

class Mount : public WheelElement
{
public:
	Mount(MountType m, IDirect3DDevice9* dev);

	bool isActive() const override;

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
			return "Roller Beetle";
		case MountType::GRIFFON:
			return "Griffon";
		case MountType::WARCLAW:
			return "Warclaw";
		case MountType::SKYSCALE:
			return "Warclaw";
		default:
			return "[Unknown]";
		}
	}
	static const char* GetMountNicknameFromType(MountType m)
	{
		switch (m)
		{
		case MountType::RAPTOR:
			return "raptor";
		case MountType::SPRINGER:
			return "springer";
		case MountType::SKIMMER:
			return "skimmer";
		case MountType::JACKAL:
			return "jackal";
		case MountType::BEETLE:
			return "beetle";
		case MountType::GRIFFON:
			return "griffon";
		case MountType::WARCLAW:
			return "warclaw";
		case MountType::SKYSCALE:
			return "skyscale";
		default:
			return "unknown";
		}
	}

	std::array<float, 4> color() override;
};

}

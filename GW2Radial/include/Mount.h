#pragma once
#include <Main.h>
#include <WheelElement.h>

namespace GW2Radial
{

class Mount : public WheelElement
{
public:
	Mount(MountType m, ID3D11Device* dev);

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
		case MountType::GRIFFON:
			return "Griffon";
		case MountType::BEETLE:
			return "Roller Beetle";
        case MountType::WARCLAW:
            return "Warclaw";
		case MountType::SKYSCALE:
			return "Skyscale";
		case MountType::SKIFF:
			return "Skiff";
		case MountType::TURTLE:
			return "Turtle";
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
		case MountType::GRIFFON:
			return "griffon";
		case MountType::BEETLE:
			return "beetle";
		case MountType::WARCLAW:
			return "warclaw";
		case MountType::SKYSCALE:
			return "skyscale";
		case MountType::SKIFF:
			return "skiff";
		case MountType::TURTLE:
			return "turtle";
		default:
			return "unknown";
		}
	}

	fVector4 color() override;
};

}
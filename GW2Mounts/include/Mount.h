#pragma once
#include <Main.h>
#include <WheelElement.h>

namespace GW2Addons
{

enum class MountType : uint
{
	NONE = (uint)-1,
	RAPTOR = 0,
	SPRINGER = 1,
	SKIMMER = 2,
	JACKAL = 3,
	BEETLE = 4,
	GRIFFON = 5
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
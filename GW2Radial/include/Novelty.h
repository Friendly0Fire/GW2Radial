#pragma once
#include <Main.h>
#include <WheelElement.h>

namespace GW2Radial
{

enum class NoveltyType : uint
{
	NONE = 0xFFFFFFFF,
	CHAIR = IDR_NOVELTY1,
	MUSICAL_INSTRUMENT = IDR_NOVELTY2,
	HELD_ITEM = IDR_NOVELTY3,
	TRAVEL_TOY = IDR_NOVELTY4,
	TONIC = IDR_NOVELTY5,

	FIRST = CHAIR,
	LAST = TONIC
};
const unsigned int NoveltyTypeCount = 6;

class Novelty : public WheelElement
{
public:
	Novelty(NoveltyType m, IDirect3DDevice9* dev);

protected:
	static const char* GetNoveltyNameFromType(NoveltyType m)
	{
		switch (m)
		{
		case NoveltyType::CHAIR:
			return "Chair";
		case NoveltyType::MUSICAL_INSTRUMENT:
			return "Musical instrument";
		case NoveltyType::HELD_ITEM:
			return "Held item";
		case NoveltyType::TRAVEL_TOY:
			return "Travel toy";
		case NoveltyType::TONIC:
			return "Tonic";
		default:
			return "[Unknown]";
		}
	}
	static const char* GetNoveltyNicknameFromType(NoveltyType m)
	{
		switch (m)
		{
		case NoveltyType::CHAIR:
			return "chair";
		case NoveltyType::MUSICAL_INSTRUMENT:
			return "musical_instrument";
		case NoveltyType::HELD_ITEM:
			return "held_item";
		case NoveltyType::TRAVEL_TOY:
			return "travel_toy";
		case NoveltyType::TONIC:
			return "tonic";
		default:
			return "unknown";
		}
	}

	fVector4 color() override;
};

}
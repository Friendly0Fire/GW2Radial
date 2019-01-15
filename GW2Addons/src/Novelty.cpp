#include <Novelty.h>
#include <Wheel.h>

namespace GW2Addons
{

Novelty::Novelty(NoveltyType m, IDirect3DDevice9* dev)
	: WheelElement(uint(m), std::string("novelty_") + GetNoveltyNicknameFromType(m), "Mounts", GetNoveltyNameFromType(m), dev)
{ }

void Novelty::AddAllNovelties(Wheel* w, IDirect3DDevice9* dev)
{
	for(auto i = NoveltyType::FIRST; i <= NoveltyType::LAST; i = NoveltyType(uint(i) + 1))
		w->AddElement(std::make_unique<Novelty>(i, dev));
}

std::array<float, 4> Novelty::color()
{
	auto mt = NoveltyType(elementId_);
	switch(mt)
	{
	case NoveltyType::CHAIR:
		return { 213 / 255.f, 100 / 255.f, 89 / 255.f, 1 };
	case NoveltyType::MUSICAL_INSTRUMENT:
		return { 212 / 255.f, 198 / 255.f, 94 / 255.f, 1 };
	case NoveltyType::HELD_ITEM:
		return { 108 / 255.f, 128 / 255.f, 213 / 255.f, 1 };
	case NoveltyType::TRAVEL_TOY:
		return { 120 / 255.f, 183 / 255.f, 197 / 255.f, 1 };
	case NoveltyType::TONIC:
		return { 199 / 255.f, 131 / 255.f, 68 / 255.f, 1 };
	default:
		return { 1, 1, 1, 1 };
	}
}

}
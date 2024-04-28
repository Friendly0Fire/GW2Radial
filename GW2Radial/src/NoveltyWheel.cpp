#include <NoveltyWheel.h>
#include <Wheel.h>

namespace GW2Radial
{

NoveltyWheel::NoveltyWheel(std::shared_ptr<Texture2D> bgTexture)
    : Wheel(std::move(bgTexture), "novelties", "Novelties")
{
    // Novelties are variable in their restrictions, so show and allow at all times
    auto props = ConditionalProperties::UsableAll | ConditionalProperties::VisibleAll;
    IterateEnum<NoveltyType>(
        [&](auto i)
        {
            AddElement(std::make_unique<WheelElement>(static_cast<u32>(i), std::string("novelty_") + GetNoveltyNicknameFromType(i), "Novelties & Masteries",
                                                      GetNoveltyNameFromType(i), GetNoveltyColorFromType(i), props));
        });
}

glm::vec4 NoveltyWheel::GetNoveltyColorFromType(NoveltyType n)
{
    switch (n)
    {
        case NoveltyType::Chair:
            return { 213 / 255.f, 100 / 255.f, 89 / 255.f, 1 };
        case NoveltyType::MusicalInstrument:
            return { 212 / 255.f, 198 / 255.f, 94 / 255.f, 1 };
        case NoveltyType::HeldItem:
            return { 108 / 255.f, 128 / 255.f, 213 / 255.f, 1 };
        case NoveltyType::TravelToy:
            return { 120 / 255.f, 183 / 255.f, 197 / 255.f, 1 };
        case NoveltyType::Tonic:
            return { 199 / 255.f, 131 / 255.f, 68 / 255.f, 1 };
        case NoveltyType::JadeWaypoint:
            return { 22 / 255.f, 227 / 255.f, 0 / 255.f, 1 };
        case NoveltyType::Fishing:
            return { 2 / 255.f, 154 / 255.f, 255 / 255.f, 1 };
        default:
            return { 1, 1, 1, 1 };
    }
}

} // namespace GW2Radial

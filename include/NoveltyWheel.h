#pragma once
#include <Main.h>
#include <Wheel.h>

namespace GW2Radial
{

class NoveltyWheel : public Wheel
{
public:
    NoveltyWheel(std::shared_ptr<Texture2D> bgTexture);

protected:
    static const char* GetNoveltyNameFromType(NoveltyType m)
    {
        switch (m)
        {
            case NoveltyType::Chair:
                return "Chair";
            case NoveltyType::MusicalInstrument:
                return "Musical instrument";
            case NoveltyType::HeldItem:
                return "Held item";
            case NoveltyType::TravelToy:
                return "Travel toy";
            case NoveltyType::Tonic:
                return "Tonic";
            case NoveltyType::JadeWaypoint:
                return "Jade waypoint";
            case NoveltyType::Fishing:
                return "Start fishing";
            default:
                return "[Unknown]";
        }
    }
    static const char* GetNoveltyNicknameFromType(NoveltyType m)
    {
        switch (m)
        {
            case NoveltyType::Chair:
                return "chair";
            case NoveltyType::MusicalInstrument:
                return "musical_instrument";
            case NoveltyType::HeldItem:
                return "held_item";
            case NoveltyType::TravelToy:
                return "travel_toy";
            case NoveltyType::Tonic:
                return "tonic";
            case NoveltyType::JadeWaypoint:
                return "jade_waypoint";
            case NoveltyType::Fishing:
                return "fishing";
            default:
                return "unknown";
        }
    }

    static glm::vec4 GetNoveltyColorFromType(NoveltyType n);
};

} // namespace GW2Radial
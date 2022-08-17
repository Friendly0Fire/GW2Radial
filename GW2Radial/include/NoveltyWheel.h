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
            case NoveltyType::JADE_WAYPOINT:
                return "Jade waypoint";
            case NoveltyType::FISHING:
                return "Start fishing";
            case NoveltyType::SKIFF:
                return "Skiff";
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
            case NoveltyType::JADE_WAYPOINT:
                return "jade_waypoint";
            case NoveltyType::FISHING:
                return "fishing";
            case NoveltyType::SKIFF:
                return "skiff";
            default:
                return "unknown";
        }
    }

    static glm::vec4 GetNoveltyColorFromType(NoveltyType n);
};

} // namespace GW2Radial
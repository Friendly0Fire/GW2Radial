#include <MarkerWheel.h>
#include <Wheel.h>

namespace GW2Radial
{

std::map<MarkerType, std::tuple<std::string, std::string, glm::vec4>> data{
    {MarkerType::Arrow,     { "arrow", "Arrow", { 186 / 255.f, 237 / 255.f, 126 / 255.f, 1 } }        },
    { MarkerType::Circle,   { "circle", "Circle", { 107 / 255.f, 24 / 255.f, 181 / 255.f, 1 } }       },
    { MarkerType::Heart,    { "heart", "Heart", { 222 / 255.f, 40 / 255.f, 41 / 255.f, 1 } }          },
    { MarkerType::Square,   { "square", "Square", { 57 / 255.f, 134 / 255.f, 231 / 255.f, 1 } }       },
    { MarkerType::Star,     { "star", "Star", { 27 / 255.f, 181 / 255.f, 68 / 255.f, 1 } }            },
    { MarkerType::Spiral,   { "spiral", "Spiral", { 140 / 255.f, 239 / 255.f, 236 / 255.f, 1 } }      },
    { MarkerType::Triangle, { "triangle", "Triangle", { 239 / 255.f, 121 / 255.f, 214 / 255.f, 1 } }  },
    { MarkerType::X,        { "x", "X", { 222 / 255.f, 182 / 255.f, 33 / 255.f, 1 } }                 },
    { MarkerType::Clear,    { "clear_all", "Clear All", { 128 / 255.f, 128 / 255.f, 128 / 255.f, 1 } }},
};

MarkerWheel::MarkerWheel(std::shared_ptr<Texture2D> bgTexture)
    : Wheel(std::move(bgTexture), "markers", "Markers")
{
    // Markers are usable and visible at all times
    auto props = ConditionalProperties::UsableAll | ConditionalProperties::VisibleAll;
    SetAlwaysResetCursorPositionBeforeKeyPress(true);
    for (const auto& [id, val] : data)
    {
        const auto& [nick, name, color] = val;
        AddElement(std::make_unique<WheelElement>(u32(id), "marker_" + nick, "Markers", name, GetMarkerColorFromType(id), props));
    }
}

glm::vec4 MarkerWheel::GetMarkerColorFromType(MarkerType m)
{
    return std::get<2>(data.at(m));
}

ObjectMarkerWheel::ObjectMarkerWheel(std::shared_ptr<Texture2D> bgTexture)
    : Wheel(std::move(bgTexture), "object_markers", "Object Markers")
{
    // Markers are usable and visible at all times
    auto props = ConditionalProperties::UsableAll | ConditionalProperties::VisibleAll;
    for (const auto& [id, val] : data)
    {
        const auto& [nick, name, color] = val;
        AddElement(std::make_unique<WheelElement>(u32(id), "object_marker_" + nick, "ObjectMarkers", name, GetObjectMarkerColorFromType(id), props));
    }
}

glm::vec4 ObjectMarkerWheel::GetObjectMarkerColorFromType(MarkerType m)
{
    return std::get<2>(data.at(m));
}

} // namespace GW2Radial
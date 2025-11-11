#pragma once
#include <Main.h>
#include <Resource.h>

namespace GW2Radial
{

enum class MountType : u32
{
    None     = 0xFFFFFFFF,
    Raptor   = IDR_MOUNT1,
    Springer = IDR_MOUNT2,
    Skimmer  = IDR_MOUNT3,
    Jackal   = IDR_MOUNT4,
    Griffon  = IDR_MOUNT5,
    Beetle   = IDR_MOUNT6,
    Warclaw  = IDR_MOUNT7,
    Skyscale = IDR_MOUNT8,
    Turtle   = IDR_MOUNT9,
    Skiff    = IDR_MOUNT10,

    First    = Raptor,
    Last     = Skiff
};
constexpr u32 MountTypeCount = std::underlying_type_t<MountType>(MountType::Last) + 1;
constexpr u32 MountIndex(MountType m)
{
    return u32(m) - u32(MountType::First);
}

enum class MountSpecial : u32
{
    Cancel = ToUnderlying(MountType::Last) + 1,
    Force  = ToUnderlying(MountType::Last) + 2,
};

constexpr u32 MountIndex(MountSpecial m)
{
    return u32(m) - u32(MountType::First);
}

enum class MarkerType : u32
{
    Arrow    = IDR_MARKER1,
    Circle   = IDR_MARKER2,
    Heart    = IDR_MARKER3,
    Square   = IDR_MARKER4,
    Star     = IDR_MARKER5,
    Spiral   = IDR_MARKER6,
    Triangle = IDR_MARKER7,
    X        = IDR_MARKER8,
    Clear    = IDR_MARKER9,

    First    = Arrow,
    Last     = Clear
};

enum class NoveltyType : u32
{
    None              = 0xFFFFFFFF,
    Chair             = IDR_NOVELTY1,
    MusicalInstrument = IDR_NOVELTY2,
    HeldItem          = IDR_NOVELTY3,
    TravelToy         = IDR_NOVELTY4,
    Tonic             = IDR_NOVELTY5,
    JadeWaypoint      = IDR_NOVELTY6,
    Fishing           = IDR_NOVELTY7,
    ScanForRift       = IDR_NOVELTY8,
    SummonDoorway     = IDR_NOVELTY9,

    First             = Chair,
    Last              = SummonDoorway
};
constexpr unsigned int NoveltyTypeCount = std::underlying_type_t<NoveltyType>(NoveltyType::Last) + 1;

constexpr u32          NoveltyIndex(NoveltyType m)
{
    return u32(m) - u32(NoveltyType::First);
}

} // namespace GW2Radial
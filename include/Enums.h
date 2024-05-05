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
constexpr unsigned int MountTypeCount = std::underlying_type_t<MountType>(MountType::Last) + 1;
constexpr unsigned int MountIndex(MountType m)
{
    return unsigned int(m) - unsigned int(MountType::First);
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

    First             = Chair,
    Last              = Fishing
};
constexpr unsigned int NoveltyTypeCount = std::underlying_type_t<NoveltyType>(NoveltyType::Last) + 1;

} // namespace GW2Radial
#pragma once
#include <Main.h>

namespace GW2Radial {

enum class MountType : uint
{
	NONE = 0xFFFFFFFF,
	RAPTOR = IDR_MOUNT1,
	SPRINGER = IDR_MOUNT2,
	SKIMMER = IDR_MOUNT3,
	JACKAL = IDR_MOUNT4,
	GRIFFON = IDR_MOUNT5,
	BEETLE = IDR_MOUNT6,
	WARCLAW = IDR_MOUNT7,
    SKYSCALE = IDR_MOUNT8,

	FIRST = RAPTOR,
	LAST = SKYSCALE
};
const unsigned int MountTypeCount = 7;

enum class MarkerType : uint
{
	ARROW       = IDR_MARKER1,
	CIRCLE      = IDR_MARKER2,
	HEART       = IDR_MARKER3,
	SQUARE      = IDR_MARKER4,
	STAR        = IDR_MARKER5,
	SPIRAL      = IDR_MARKER6,
	TRIANGLE    = IDR_MARKER7,
	X           = IDR_MARKER8,
	CLEAR       = IDR_MARKER9,
};

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

template<typename E>
bool Is(E e) {
	return e != E::NONE;
}

}
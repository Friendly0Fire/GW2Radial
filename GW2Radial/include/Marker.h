#pragma once
#include <Main.h>
#include <WheelElement.h>

namespace GW2Radial
{

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

class Marker : public WheelElement
{
public:
	Marker(MarkerType m, IDirect3DDevice9* dev);

	fVector4 color() override;
};

class ObjectMarker : public WheelElement
{
public:
	ObjectMarker(MarkerType m, IDirect3DDevice9* dev);

	fVector4 color() override;
};

}
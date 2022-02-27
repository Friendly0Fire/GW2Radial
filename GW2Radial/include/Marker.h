#pragma once
#include <Main.h>
#include <WheelElement.h>

namespace GW2Radial
{

class Marker : public WheelElement
{
public:
	Marker(MarkerType m);

	fVector4 color() override;
};

class ObjectMarker : public WheelElement
{
public:
	ObjectMarker(MarkerType m);

	fVector4 color() override;
};

}
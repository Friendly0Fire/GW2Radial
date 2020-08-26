#pragma once
#include <Main.h>
#include <WheelElement.h>

namespace GW2Radial
{

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
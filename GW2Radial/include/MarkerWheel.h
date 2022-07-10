#pragma once
#include <Main.h>
#include <Wheel.h>

namespace GW2Radial
{

class MarkerWheel : public Wheel
{
public:
	MarkerWheel(std::shared_ptr<Texture2D> bgTexture);
	
	static glm::vec4 GetMarkerColorFromType(MarkerType m);
};

class ObjectMarkerWheel : public Wheel
{
public:
	ObjectMarkerWheel(std::shared_ptr<Texture2D> bgTexture);
	
	static glm::vec4 GetObjectMarkerColorFromType(MarkerType m);
};

}
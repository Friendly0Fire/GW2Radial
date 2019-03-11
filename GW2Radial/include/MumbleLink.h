#pragma once
#include <Main.h>
#include <Singleton.h>

namespace GW2Radial
{

class MumbleLink : public Singleton<MumbleLink>
{
public:
	MumbleLink();
	~MumbleLink();

	bool isWvW() const;

protected:
	const struct MumbleContext* context() const;
	HANDLE fileMapping_ = nullptr;
	struct LinkedMem* linkedMemory_ = nullptr;
};

}
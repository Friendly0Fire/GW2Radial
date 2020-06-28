#pragma once
#include <Main.h>
#include <Singleton.h>

#define PARSE_FLAG_BOOL(name, offset) inline bool name() const { return (uiState() & (1 << offset)) != 0; }

namespace GW2Radial
{

class MumbleLink : public Singleton<MumbleLink> {
public:
	MumbleLink();
	~MumbleLink();

	bool isInWvW() const;

	// uiState flags
	PARSE_FLAG_BOOL(isMapOpen, 0);
	PARSE_FLAG_BOOL(isCompassTopRight, 1);
	PARSE_FLAG_BOOL(doesCompassHaveRotationEnabled, 2);
	PARSE_FLAG_BOOL(gameHasFocus, 3);
	PARSE_FLAG_BOOL(isInCompetitiveMode, 4);
	PARSE_FLAG_BOOL(textboxHasFocus, 5);
	PARSE_FLAG_BOOL(isInCombat, 6);

protected:
	uint32_t uiState() const;

	const struct MumbleContext* context() const;
	HANDLE fileMapping_ = nullptr;
	struct LinkedMem* linkedMemory_ = nullptr;
};

}
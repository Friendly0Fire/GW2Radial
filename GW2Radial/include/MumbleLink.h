#pragma once
#include <Main.h>
#include <Singleton.h>
#include <Mount.h>

#define PARSE_FLAG_BOOL(name, offset) [[nodiscard]] inline bool name() const { return (uiState() & (1 << offset)) != 0; }

namespace GW2Radial
{

class MumbleLink : public Singleton<MumbleLink> {
public:
	MumbleLink();
	~MumbleLink();

	void OnUpdate();

	[[nodiscard]] bool isInWvW() const;

	// uiState flags
	PARSE_FLAG_BOOL(isMapOpen, 0);
	PARSE_FLAG_BOOL(isCompassTopRight, 1);
	PARSE_FLAG_BOOL(doesCompassHaveRotationEnabled, 2);
	PARSE_FLAG_BOOL(gameHasFocus, 3);
	PARSE_FLAG_BOOL(isInCompetitiveMode, 4);
	PARSE_FLAG_BOOL(textboxHasFocus, 5);
	PARSE_FLAG_BOOL(isInCombat, 6);

	[[nodiscard]] fVector3 position() const;

	[[nodiscard]] MountType currentMount() const;
	[[nodiscard]] bool isMounted() const;
	[[nodiscard]] bool isInMap() const;
	[[nodiscard]] uint32_t mapId() const;
	[[nodiscard]] const wchar_t* characterName() const;
	[[nodiscard]] bool isSwimmingOnSurface() const;
	[[nodiscard]] bool isUnderwater() const;
	
	[[nodiscard]] uint8_t characterProfession() const {
	    return identity_.profession;
    }
	
	[[nodiscard]] uint8_t characterSpecialization() const {
	    return identity_.specialization;
    }

    [[nodiscard]] uint8_t characterRace() const {
	    return identity_.race;
    }

	[[nodiscard]] bool isCommander() const {
		return identity_.commander;
	}

	[[nodiscard]] float fov() const {
		return identity_.fov;
	}

	[[nodiscard]] uint8_t uiScale() const {
		return identity_.uiScale;
	}

protected:
	[[nodiscard]] uint32_t uiState() const;

	struct Identity
	{
	    uint8_t profession = 0;
		uint8_t specialization = 0;
		uint8_t race = 0;
		bool commander = false;
		float fov = 0.f;
		uint8_t uiScale = 0;
	};

	[[nodiscard]] const struct MumbleContext* context() const;
	HANDLE fileMapping_ = nullptr;
	struct LinkedMem* linkedMemory_ = nullptr;

	Identity identity_;
};

}
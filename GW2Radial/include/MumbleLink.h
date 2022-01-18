#pragma once
#include <Main.h>
#include <Singleton.h>

#define PARSE_FLAG_BOOL(name, offset) [[nodiscard]] inline bool name() const { return (uiState() & (1 << offset)) != 0; }

namespace GW2Radial
{

struct LinkedMem;
struct MumbleContext;

class MumbleLink : public Singleton<MumbleLink> {
public:
	enum class Profession : uint8_t {
		NONE = 0,
	    GUARDIAN = 1,
		WARRIOR = 2,
		ENGINEER = 3,
		RANGER = 4,
		THIEF = 5,
		ELEMENTALIST = 6,
		MESMER = 7,
		NECROMANCER = 8,
		REVENANT = 9
	};

	enum class EliteSpec : uint8_t {
		NONE = 0,
		BERSERKER = 1,
		BLADESWORN = 2,
		CATALYST = 3,
		CHRONOMANCER = 4,
		DAREDEVIL = 5,
		DEADEYE = 6,
		DRAGONHUNTER = 7,
		DRUID = 8,
		FIREBRAND = 9,
		HARBINGER = 10,
		HERALD = 11,
		HOLOSMITH = 12,
		MECHANIST = 13,
		MIRAGE = 14,
		REAPER = 15,
		RENEGADE = 16,
		SCOURGE = 17,
		SCRAPPER = 18,
		SOULBEAST = 19,
		SPECTER = 20,
		SPELLBREAKER = 21,
		TEMPEST = 22,
		UNTAMED = 23,
		VINDICATOR = 24,
		VIRTUOSO = 25,
		WEAVER = 26,
		WILLBENDER = 27
	};

	enum class Race : uint8_t {
	    ASURA = 0,
		CHARR = 1,
		HUMAN = 2,
		NORN = 3,
		SYLVARI = 4
	};

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
	[[nodiscard]] std::wstring characterName() const;
	[[nodiscard]] bool isSwimmingOnSurface() const;
	[[nodiscard]] bool isUnderwater() const;

	[[nodiscard]] Profession characterProfession() const {
	    return identity_.profession;
    }

	[[nodiscard]] EliteSpec characterSpecialization() const;

    [[nodiscard]] Race characterRace() const {
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
	std::wstring fileMappingName_ = L"MumbleLink";

	[[nodiscard]] uint32_t uiState() const;

	struct Identity
	{
	    Profession profession = Profession::NONE;
		EliteSpec specialization = EliteSpec::NONE;
		Race race = Race::ASURA;
		bool commander = false;
		float fov = 0.f;
		uint8_t uiScale = 0;
		std::string name;
	};

	[[nodiscard]] const MumbleContext* context() const;
	HANDLE fileMapping_ = nullptr;
	LinkedMem* linkedMemory_ = nullptr;

	Identity identity_;
};

}
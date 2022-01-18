#include <MumbleLink.h>
#include <nlohmann/json.hpp>
#include <Utility.h>

namespace GW2Radial
{
struct LinkedMem
{
#ifdef WIN32
	UINT32	uiVersion;
	DWORD	uiTick;
#else
	uint32_t uiVersion;
	uint32_t uiTick;
#endif
	fVector3	fAvatarPosition;
	fVector3	fAvatarFront;
	fVector3	fAvatarTop;
	wchar_t		name[256];
	fVector3	fCameraPosition;
	fVector3	fCameraFront;
	fVector3	fCameraTop;
	wchar_t		identity[256];
#ifdef WIN32
	UINT32	context_len;
#else
	uint32_t context_len;
#endif
	unsigned char context[256];
	wchar_t description[2048];
};

struct MumbleContext {
	std::byte serverAddress[28]; // contains sockaddr_in or sockaddr_in6
	uint32_t mapId;
	uint32_t mapType;
	uint32_t shardId;
	uint32_t instance;
	uint32_t buildId;
	// Additional data beyond the 48 bytes Mumble uses for identification
	uint32_t uiState; // Bitmask: Bit 1 = IsMapOpen, Bit 2 = IsCompassTopRight, Bit 3 = DoesCompassHaveRotationEnabled, Bit 4 = Game has focus, Bit 5 = Is in Competitive game mode, Bit 6 = Textbox has focus, Bit 7 = Is in Combat
	uint16_t compassWidth; // pixels
	uint16_t compassHeight; // pixels
	float compassRotation; // radians
	float playerX; // continentCoords
	float playerY; // continentCoords
	float mapCenterX; // continentCoords
	float mapCenterY; // continentCoords
	float mapScale;
	uint32_t processId;
	uint8_t mountIndex;
};

MumbleLink::MumbleLink() {
	if (auto* m = GetCommandLineArg(L"mumble"); m)
		fileMappingName_ = m;

	fileMapping_ = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(LinkedMem), fileMappingName_.c_str());
	if (!fileMapping_) {
		Log::i().Print(Severity::Error, L"Could not find MumbleLink map named '{}'!", fileMappingName_.c_str());
		return;
	}

	linkedMemory_ = static_cast<LinkedMem*>(MapViewOfFile(fileMapping_, FILE_MAP_READ, 0, 0, sizeof(LinkedMem)));
	if(!linkedMemory_)
	{
		Log::i().Print(Severity::Error, L"Could not map to MumbleLink map named '{}'!", fileMappingName_.c_str());

		CloseHandle(fileMapping_);
		fileMapping_ = nullptr;
	}
}

MumbleLink::~MumbleLink() {
	if(linkedMemory_)
	{
		UnmapViewOfFile(linkedMemory_);
		linkedMemory_ = nullptr;
	}
	if(fileMapping_)
	{
		CloseHandle(fileMapping_);
		fileMapping_ = nullptr;
	}
}

void MumbleLink::OnUpdate()
{
	identity_ = {};
	auto identityUtf8 = utf8_encode(linkedMemory_->identity);
	auto json = nlohmann::json::parse(identityUtf8, nullptr, false);

	auto updateIfExists = [&json](auto& value, const char* key)
	{
	    auto f = json.find(key);
		if(f != json.end())
			value = *f;
	};

	updateIfExists(identity_.commander, "commander");
	updateIfExists(identity_.fov, "fov");
	updateIfExists(identity_.uiScale, "uisz");
	updateIfExists(identity_.race, "race");
	updateIfExists(identity_.specialization, "spec");
	updateIfExists(identity_.profession, "profession");
	updateIfExists(identity_.name, "name");
}

bool MumbleLink::isInMap() const {
	if (!linkedMemory_)
		return false;

	return context()->mapId != 0;
}

uint32_t MumbleLink::mapId() const {
    if (!linkedMemory_)
		return 0;

	return context()->mapId;
}

std::wstring MumbleLink::characterName() const
{
	if (!linkedMemory_)
		return L"";

	return utf8_decode(identity_.name);
}

const float MinSurfaceThreshold = -1.15f;

bool MumbleLink::isSwimmingOnSurface() const {
    if (!linkedMemory_)
		return false;

    return linkedMemory_->fAvatarPosition.y <= -1.f && linkedMemory_->fAvatarPosition.y >= MinSurfaceThreshold;
}

bool MumbleLink::isUnderwater() const {
    if (!linkedMemory_)
		return false;

    return linkedMemory_->fAvatarPosition.y < MinSurfaceThreshold;
}

MumbleLink::EliteSpec MumbleLink::characterSpecialization() const
{
	enum class AnetEliteSpec : uint8_t {
		NONE = 0,
		DRUID = 5,
		DAREDEVIL = 7,
		BERSERKER = 18,
		DRAGONHUNTER = 27,
		REAPER = 34,
		CHRONOMANCER = 40,
		SCRAPPER = 43,
		TEMPEST = 48,
		HERALD = 52,
		SOULBEAST = 55,
		WEAVER = 56,
		HOLOSMITH = 57,
		DEADEYE = 58,
		MIRAGE = 59,
		SCOURGE = 60,
		SPELLBREAKER = 61,
		FIREBRAND = 62,
		RENEGADE = 63,
		HARBINGER = 64,
		WILLBENDER = 65,
		VIRTUOSO = 66,
		CATALYST = 67,
		BLADESWORN = 68,
		VINDICATOR = 69,
		MECHANIST = 70,
		SPECTER = 71,
		UNTAMED = 72
	};

	switch (AnetEliteSpec(identity_.specialization))
	{
	default: return EliteSpec::NONE;
	case AnetEliteSpec::DRUID : return EliteSpec::DRUID;
	case AnetEliteSpec::DAREDEVIL : return EliteSpec::DAREDEVIL;
	case AnetEliteSpec::BERSERKER : return EliteSpec::BERSERKER;
	case AnetEliteSpec::DRAGONHUNTER : return EliteSpec::DRAGONHUNTER;
	case AnetEliteSpec::REAPER : return EliteSpec::REAPER;
	case AnetEliteSpec::CHRONOMANCER : return EliteSpec::CHRONOMANCER;
	case AnetEliteSpec::SCRAPPER : return EliteSpec::SCRAPPER;
	case AnetEliteSpec::TEMPEST : return EliteSpec::TEMPEST;
	case AnetEliteSpec::HERALD : return EliteSpec::HERALD;
	case AnetEliteSpec::SOULBEAST : return EliteSpec::SOULBEAST;
	case AnetEliteSpec::WEAVER : return EliteSpec::WEAVER;
	case AnetEliteSpec::HOLOSMITH : return EliteSpec::HOLOSMITH;
	case AnetEliteSpec::DEADEYE : return EliteSpec::DEADEYE;
	case AnetEliteSpec::MIRAGE : return EliteSpec::MIRAGE;
	case AnetEliteSpec::SCOURGE : return EliteSpec::SCOURGE;
	case AnetEliteSpec::SPELLBREAKER : return EliteSpec::SPELLBREAKER;
	case AnetEliteSpec::FIREBRAND : return EliteSpec::FIREBRAND;
	case AnetEliteSpec::RENEGADE : return EliteSpec::RENEGADE;
	case AnetEliteSpec::HARBINGER : return EliteSpec::HARBINGER;
	case AnetEliteSpec::WILLBENDER : return EliteSpec::WILLBENDER;
	case AnetEliteSpec::VIRTUOSO : return EliteSpec::VIRTUOSO;
	case AnetEliteSpec::CATALYST : return EliteSpec::CATALYST;
	case AnetEliteSpec::BLADESWORN : return EliteSpec::BLADESWORN;
	case AnetEliteSpec::VINDICATOR : return EliteSpec::VINDICATOR;
	case AnetEliteSpec::MECHANIST : return EliteSpec::MECHANIST;
	case AnetEliteSpec::SPECTER : return EliteSpec::SPECTER;
	case AnetEliteSpec::UNTAMED : return EliteSpec::UNTAMED;
	}
}


bool MumbleLink::isInWvW() const {
	if (!linkedMemory_)
		return false;

	auto mt = context()->mapType;

	return mt == 18 || (mt >= 9 && mt <= 15 && mt != 13);
}

uint32_t MumbleLink::uiState() const {
	if (!linkedMemory_)
		return 0;

	return context()->uiState;
}

fVector3 MumbleLink::position() const {
	if (!linkedMemory_)
		return { 0, 0, 0 };

    return linkedMemory_->fAvatarPosition;
}

MountType MumbleLink::currentMount() const {
	if (!linkedMemory_)
		return MountType::NONE;

	enum class AnetMountType : uint32_t {
		None,
		Jackal,
		Griffon,
		Springer,
		Skimmer,
		Raptor,
		RollerBeetle,
		Warclaw,
		Dragon,
	};

	switch ((AnetMountType)context()->mountIndex) {
	default:
	case AnetMountType::None:
		return MountType::NONE;
	case AnetMountType::Jackal:
		return MountType::JACKAL;
	case AnetMountType::Griffon:
		return MountType::GRIFFON;
	case AnetMountType::Springer:
		return MountType::SPRINGER;
	case AnetMountType::Raptor:
		return MountType::RAPTOR;
	case AnetMountType::RollerBeetle:
		return MountType::BEETLE;
	case AnetMountType::Warclaw:
		return MountType::WARCLAW;
	case AnetMountType::Dragon:
		return MountType::SKYSCALE;
	}
}

bool MumbleLink::isMounted() const {
	if (!linkedMemory_)
		return false;

	return context()->mountIndex != 0;
}

const MumbleContext* MumbleLink::context() const {
	if(!linkedMemory_)
		return nullptr;

	return reinterpret_cast<const MumbleContext*>(&linkedMemory_->context);
}

}
#include <MumbleLink.h>
#include <nlohmann/json.hpp>

namespace GW2Radial
{
DEFINE_SINGLETON(MumbleLink);

struct LinkedMem
{
#ifdef WIN32
	UINT32	uiVersion;
	DWORD	uiTick;
#else
	uint32_t uiVersion;
	uint32_t uiTick;
#endif
	float	fAvatarPosition[3];
	float	fAvatarFront[3];
	float	fAvatarTop[3];
	wchar_t	name[256];
	float	fCameraPosition[3];
	float	fCameraFront[3];
	float	fCameraTop[3];
	wchar_t	identity[256];
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
	fileMapping_ = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(LinkedMem), L"MumbleLink");
	if(!fileMapping_)
		return;

	linkedMemory_ = static_cast<LinkedMem*>(MapViewOfFile(fileMapping_, FILE_MAP_READ, 0, 0, sizeof(LinkedMem)));
	if(!linkedMemory_)
	{
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
	auto json = nlohmann::json::parse(linkedMemory_->identity, linkedMemory_->identity + 256, nullptr, false);

	identity_.commander = json["commander"];
	identity_.fov = json["fov"];
	identity_.uiScale = json["uisz"];
	identity_.race = json["race"];
	identity_.specialization = json["spec"];
	identity_.profession = json["profession"];
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

const wchar_t* MumbleLink::characterName() const
{
	if (!linkedMemory_)
		return nullptr;

	return linkedMemory_->name;
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
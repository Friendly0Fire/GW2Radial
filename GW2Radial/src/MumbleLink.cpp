#include <MumbleLink.h>

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

struct MumbleContext
{
	std::byte serverAddress[28]; // contains sockaddr_in or sockaddr_in6
    unsigned mapId;
    unsigned mapType;
    unsigned shardId;
    unsigned instance;
    unsigned buildId;
};

MumbleLink::MumbleLink()
{
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

MumbleLink::~MumbleLink()
{
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

bool MumbleLink::isWvW() const
{
	if(!linkedMemory_)
		return false;

	auto mt = context()->mapType;

	return mt >= 9 && mt <= 15 && mt != 13;
}

const MumbleContext* MumbleLink::context() const
{
	if(!linkedMemory_)
		return nullptr;

	return reinterpret_cast<const MumbleContext*>(&linkedMemory_->context);
}

}
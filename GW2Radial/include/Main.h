#pragma once
/// These defines are universally accepted as helping to trim down windows.h size
#define WIN32_LEAN_AND_MEAN
#define STRICT

/// These defines all come from windows.h itself and were set to minimize size as much as possible
/// If new usages of the Windows APIs arise, some of these may have to be commented out
#define NOGDICAPMASKS // - CC_*, LC_*, PC_*, CP_*, TC_*, RC_
//#define NOVIRTUALKEYCODES // - VK_*
//#define NOWINMESSAGES // - WM_*, EM_*, LB_*, CB_*
//#define NOWINSTYLES // - WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
//#define NOSYSMETRICS // - SM_*
#define NOMENUS // - MF_*
#define NOICONS // - IDI_*
//#define NOKEYSTATES // - MK_*
#define NOSYSCOMMANDS // - SC_*
#define NORASTEROPS // - Binary and Tertiary raster ops
//#define NOSHOWWINDOW // - SW_*
#define OEMRESOURCE // - OEM Resource values
#define NOATOM // - Atom Manager routines
//#define NOCLIPBOARD // - Clipboard routines
//#define NOCOLOR // - Screen colors
//#define NOCTLMGR // - Control and Dialog routines
#define NODRAWTEXT // - DrawText() and DT_*
//#define NOGDI // - All GDI defines and routines
#define NOKERNEL // - All KERNEL defines and routines
//#define NOUSER // - All USER defines and routines
//#define NONLS // - All NLS defines and routines
//#define NOMB // - MB_* and MessageBox()
#define NOMEMMGR // - GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE // - typedef METAFILEPICT
#define NOMINMAX // - Macros min(a, b) and max(a, b)
//#define NOMSG // - typedef MSG and associated routines
#define NOOPENFILE // - OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL // - SB_* and scrolling routines
#define NOSERVICE // - All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND // - Sound driver routines
//#define NOTEXTMETRIC // - typedef TEXTMETRIC and associated routines
//#define NOWH // - SetWindowsHook and WH_*
//#define NOWINOFFSETS // - GWL_*, GCL_*, associated routines
#define NOCOMM // - COMM driver routines
#define NOKANJI // - Kanji support stuff.
#define NOHELP // - Help engine interface.
#define NOPROFILER // - Profiler interface.
#define NODEFERWINDOWPOS // - DeferWindowPos routines
#define NOMCX // - Modem Configuration Extensions
#include <windows.h>

#include <vector>
#include <string>
#include <memory>

#include <Resource.h>

#define COM_RELEASE(x) { if((x)) { (x)->Release(); (x) = nullptr; } }
#define NULL_COALESCE(a, b) ((a) != nullptr ? (a) : (b))
#define SQUARE(x) ((x) * (x))

typedef unsigned char uchar;
typedef unsigned int uint;
typedef std::basic_string<TCHAR> tstring;
typedef unsigned __int64 mstime;

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

#ifndef M_PI
#define M_PI 3.14159265359
#endif

typedef struct fVector4 {
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT w;
} fVector4;

typedef struct fVector3 {
	FLOAT x;
	FLOAT y;
	FLOAT z;	
} fVector3;

typedef struct fVector2 {
	FLOAT x;
	FLOAT y;
} fVector2;

#include "d3d9.h"

//D3D9 API extenders =======================

#define D3DRS_ENABLE_D912PXY_API_HACKS (D3DRENDERSTATETYPE)220
#define D3DRS_D912PXY_ENQUEUE_PSO_COMPILE (D3DRENDERSTATETYPE)221
#define D3DRS_D912PXY_SETUP_PSO (D3DRENDERSTATETYPE)222
#define D3DRS_D912PXY_GPU_WRITE (D3DRENDERSTATETYPE)223
#define D3DRS_D912PXY_DRAW (D3DRENDERSTATETYPE)224
#define D3DRS_D912PXY_SAMPLER_ID (D3DRENDERSTATETYPE)225

#define D3DDECLMETHOD_PER_VERTEX_CONSTANT 8
#define D3DUSAGE_D912PXY_FORCE_RT 0x0F000000L

#define D912PXY_ENCODE_GPU_WRITE_DSC(sz, offset) ((sz & 0xFFFF) | ((offset & 0xFFFF) << 16))

#define D912PXY_GPU_WRITE_OFFSET_TEXBIND 0
#define D912PXY_GPU_WRITE_OFFSET_SAMPLER 8 
#define D912PXY_GPU_WRITE_OFFSET_VS_VARS 16
#define D912PXY_GPU_WRITE_OFFSET_PS_VARS 16 + 256

//========

#include "Effect.h"
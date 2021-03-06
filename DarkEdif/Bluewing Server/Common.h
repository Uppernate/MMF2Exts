#pragma once

// #define TGFEXT	// TGF2, MMF2, MMF2 Dev
#define MMFEXT		// MMF2, MMF2 Dev
// #define PROEXT	// MMF2 Dev only

#ifdef RUN_ONLY
	#define CurLang (*::SDK->json.u.object.values[::SDK->json.u.object.length - 1].value)
#else
	const extern struct _json_value & CurrentLanguage();
	#define CurLang CurrentLanguage()
#endif

#define JSON_COMMENT_MACRO lacewing::relayserver::buildnum, STRIFY(CONFIG)

// Lacewing-required imports for accessing Windows sockets
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "mpr.lib")
#pragma comment(lib, "secur32.lib")
#pragma comment(lib, "crypt32.lib")

// #define lw_import
// #define _lacewing_static
// These two are now defined in project settings; they're absolutely required.

#include "Edif.h"
#include "Resource.h"
#include "DarkEdif.h"

#include "Lacewing.h"
#include "LacewingFunctions.h"

// Enable DarkEdif's utility
#define MULTI_THREADING
#include "MultiThreading.h"

#include <sstream>
#include <algorithm>
#include "zlib.h"
#pragma comment(lib, "..\\Lib\\Windows\\zlib.lib")

#ifdef _DEBUG
	extern std::stringstream CriticalSection;
#define EnterCriticalSectionDebug(x) { \
		EnterCriticalSection(x); \
		::CriticalSection << "Thread " << GetCurrentThreadId() << " : Entered on " \
		<< __FILE__ << ", line " << __LINE__ << ".\r\n"; \
	}

#define LeaveCriticalSectionDebug(x) { \
		::CriticalSection << "Thread " << GetCurrentThreadId() << " : Left on " \
			<< __FILE__ << ", line " << __LINE__ << ".\r\n"; \
		LeaveCriticalSection(x); \
	}
#else
#define EnterCriticalSectionDebug(x)  EnterCriticalSection(x)
#define LeaveCriticalSectionDebug(x)  LeaveCriticalSection(x)
#endif


// edPtr : Used at edittime and saved in the MFA/CCN/EXE files
struct EDITDATA
{
	// Header - required
	extHeader			eHeader;

	// Because Lacewing Blue can be interchanged with Lacewing Relay by replacing the MFXes,
	// we don't use DarkEdif's automatic property system (except for the menus), instead
	// matching the Relay's memory layout, and changing Edittime.cpp.

	// Any random edittime stuff? Place here.
	char pad0[5];				// For matching old extension
	bool automaticClear,
		 multiThreading;
	bool isGlobal;
	char edGlobalID[255];
	bool timeoutWarningEnabled,
		 fullDeleteEnabled,
		 enableInactivityTimer;
	char pad1[243];

	// Keep as last or risk overwriting by functions accessing this address
	size_t DarkEdif_Prop_Size;
	char DarkEdif_Props[];

#ifndef NOPROPS
	// DarkEdif functions, use within Extension ctor.
	bool IsPropChecked(int propID);
	std::tstring GetPropertyStr(const char * propName);
	std::tstring GetPropertyStr(int propID);
#endif
};

class Extension;

struct RUNDATA
{
	// Main header - required
	HeaderObject rHo;

	// Optional headers - depend on the OEFLAGS value, see documentation and examples for more info
//	rCom			rc;				// Common structure for movements & animations
//	rMvt			rm;				// Movements
//	Sprite			rs;				// Sprite (displayable objects)
//	AltVals			rv;				// Alterable values

	// Required
	Extension * pExtension;

	/*
		You shouldn't add any variables or anything here. Add them as members
		of the Extension class (Extension.h) instead.
	*/
};
DWORD WINAPI LacewingLoopThread(void * ThisExt);

#define COMMON_H
#include "Extension.h"
#undef COMMON_H

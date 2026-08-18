/*
* Main steamclient operator
*/
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "revCommon.h"
#include "steamclient/steamclient.h"
#include "logging.h"

extern int S_CALLTYPE steamclient_startup();
extern int S_CALLTYPE steamclient_shutdown();

// ik this looks like sh1t but it works
struct sc_init
{
	sc_init() { steamclient_startup(); }
	~sc_init() { steamclient_shutdown(); }

	void* _placeholder;
};

// What a dumbass valve worker inserted both SteamAPI_Init SteamAPI_Shutdown in THE SAME FUNCTION
// IN THE FCKING LAUNCHER, VALVe ARE YOU HIRING RETARDS?????
// Because of that i have to make other initializator... idiots!
static sc_init steamclient_init;
static CSteamClient steamclient;	

//---------------------------------------------------------//

S_API bool S_CALLTYPE SteamAPI_InitSafe() { 
	return SteamAPI_Init();
}
S_API bool S_CALLTYPE SteamAPI_Init() { 
	//steamclient_startup();
	return true; 
}

S_API void S_CALLTYPE SteamAPI_Shutdown() {
	//steamclient_shutdown();
}

// checks if a local Steam client is running 
S_API bool S_CALLTYPE SteamAPI_IsSteamRunning() { return true; }

S_API bool S_CALLTYPE SteamAPI_RestartAppIfNecessary(uint32 unOwnAppID) { return false; }

// crash dump recording functions
S_API void S_CALLTYPE SteamAPI_WriteMiniDump(uint32 uStructuredExceptionCode, void* pvExceptionInfo, uint32 uBuildID) { }
S_API void S_CALLTYPE SteamAPI_SetMiniDumpComment(const char* pchMsg) { }

// interface pointers, configured by SteamAPI_Init()
S_API ISteamClient* S_CALLTYPE SteamClient() { return &steamclient; }

S_API ISteamUser* S_CALLTYPE SteamUser() { return g_pSteamUser; }
S_API ISteamFriends* S_CALLTYPE SteamFriends() { return nullptr; }
S_API ISteamUtils* S_CALLTYPE SteamUtils() { return nullptr; }
S_API ISteamMatchmaking* S_CALLTYPE SteamMatchmaking() { return g_pSteamMatchMaking; }
S_API void* S_CALLTYPE SteamMasterServerUpdater() { return nullptr; }
S_API ISteamUserStats* S_CALLTYPE SteamUserStats() { return nullptr; }
S_API ISteamApps* S_CALLTYPE SteamApps() { return g_pSteamApps; }
S_API ISteamNetworking* S_CALLTYPE SteamNetworking() { return nullptr; }
S_API ISteamMatchmakingServers* S_CALLTYPE SteamMatchmakingServers() { return g_pSteamMatchMakingServers; }
S_API ISteamRemoteStorage* S_CALLTYPE SteamRemoteStorage() { return nullptr; }
S_API ISteamScreenshots* S_CALLTYPE SteamScreenshots() { return nullptr; }
S_API ISteamHTTP* S_CALLTYPE SteamHTTP() { return nullptr; }
S_API ISteamUnifiedMessages* S_CALLTYPE SteamUnifiedMessages() { return nullptr; }
S_API ISteamController* S_CALLTYPE SteamController() { return nullptr; }
S_API ISteamUGC* S_CALLTYPE SteamUGC() { return nullptr; }
S_API ISteamAppList* S_CALLTYPE SteamAppList() { return nullptr; }
S_API ISteamMusic* S_CALLTYPE SteamMusic() { return nullptr; }
S_API ISteamMusicRemote* S_CALLTYPE SteamMusicRemote() { return nullptr; }
S_API ISteamHTMLSurface* S_CALLTYPE SteamHTMLSurface() { return nullptr; }
S_API ISteamInventory* S_CALLTYPE SteamInventory() { return nullptr; }
S_API ISteamVideo* S_CALLTYPE SteamVideo() { return nullptr; }

S_API void S_CALLTYPE SteamAPI_RunCallbacks() { 
	// todo real callbacks
	g_pSteamMatchMakingServers->RunFrame();
}

// TODO: Internal functions used by the utility CCallback objects to receive callbacks
S_API void S_CALLTYPE SteamAPI_RegisterCallback(class CCallbackBase* pCallback, int iCallback) { }
S_API void S_CALLTYPE SteamAPI_UnregisterCallback(class CCallbackBase* pCallback) { }
// Internal functions used by the utility CCallResult objects to receive async call results
S_API void S_CALLTYPE SteamAPI_RegisterCallResult(class CCallbackBase* pCallback, SteamAPICall_t hAPICall) { }
S_API void S_CALLTYPE SteamAPI_UnregisterCallResult(class CCallbackBase* pCallback, SteamAPICall_t hAPICall) { }

// returns the filename path of the current running Steam process, used if you need to load an explicit steam dll by name
S_API const char* S_CALLTYPE SteamAPI_GetSteamInstallPath() { return nullptr; }

// returns the pipe we are communicating to Steam with
S_API HSteamPipe S_CALLTYPE SteamAPI_GetHSteamPipe() { return 0; }
S_API HSteamPipe S_CALLTYPE SteamAPI_GetHSteamUser() { return 0; }

// backwards compat export, passes through to SteamAPI_ variants
S_API HSteamPipe S_CALLTYPE GetHSteamPipe() { return 0; }
S_API HSteamUser S_CALLTYPE GetHSteamUser() { return 0; }

// sets whether or not Steam_RunCallbacks() should do a try {} catch (...) {} around calls to issuing callbacks
S_API void S_CALLTYPE SteamAPI_SetTryCatchCallbacks(bool bTryCatchCallbacks) { }

S_API void S_CALLTYPE SteamAPI_UseBreakpadCrashHandler(char const* pchVersion, char const* pchDate, char const* pchTime, bool bFullMemoryDumps, void* pvContext, PFNPreMinidumpCallback m_pfnPreMinidumpCallback) { }
S_API void S_CALLTYPE SteamAPI_SetBreakpadAppID(uint32 unAppID) { }

//---------------------------------------------------------//

S_API void* g_pSteamClientGameServer = nullptr;

S_API ISteamGameServer* S_CALLTYPE SteamGameServer() { return g_pSteamGameServer; }
S_API ISteamUtils* S_CALLTYPE SteamGameServerUtils() { return nullptr; }
S_API ISteamNetworking* S_CALLTYPE SteamGameServerNetworking() { return nullptr; }
S_API ISteamGameServerStats* S_CALLTYPE SteamGameServerStats() { return nullptr; }
S_API ISteamHTTP* S_CALLTYPE SteamGameServerHTTP() { return nullptr; }
S_API ISteamInventory* S_CALLTYPE SteamGameServerInventory() { return nullptr; }
S_API ISteamUGC* S_CALLTYPE SteamGameServerUGC() { return nullptr; }

S_API void  S_CALLTYPE SteamGameServer_Shutdown() { }

S_API void  S_CALLTYPE SteamGameServer_RunCallbacks() { 
	// todo real callbacks
	g_pSteamGameServer->RunCallbacks(); 
}

S_API bool  S_CALLTYPE SteamGameServer_BSecure() { return true; }
S_API uint64  S_CALLTYPE SteamGameServer_GetSteamID() { return g_pSteamGameServer->GetSteamID().ConvertToUint64(); }

S_API HSteamPipe  S_CALLTYPE SteamGameServer_GetHSteamPipe() { return 0; }
S_API HSteamUser S_CALLTYPE SteamGameServer_GetHSteamUser() { return 0; }

S_API bool S_CALLTYPE SteamGameServer_InitSafe(uint32 unIP, uint16 usSteamPort, uint16 usGamePort, uint16 usQueryPort, uint32 eServerMode, const char* pchVersionString)
{ 
	return true;
}

S_API bool S_CALLTYPE SteamGameServer_Init(uint32 unIP, uint16 usSteamPort, uint16 usGamePort, uint16 usQueryPort, uint32 eServerMode, const char* pchVersionString) 
{ 
	return true;
}

S_API int SteamGameServer_GetIPCCallCount() { 
	return 0; 
}

S_API void* S_CALLTYPE SteamInternal_ContextInit() {
	return NULL;
}

S_API void* S_CALLTYPE SteamInternal_CreateInterface() {
	return NULL;
}
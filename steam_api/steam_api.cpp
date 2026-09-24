/*
* Main steamclient operator
*/
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define VERSION_SAFE_STEAM_API_INTERFACES

#include "revCommon.h"
#include "steamclient.h"
#include "logging.h"

extern int S_CALLTYPE steam_startup();
extern int S_CALLTYPE steam_shutdown();

/*--*/	CSteamClient* g_pSteamClient = 0;				// main SteamClient object pointer, not exported
S_API	ISteamClient* g_pSteamClientGameServer = 0;		// global exported ClientGameServer interface

static double g_flLastInitTime;
std::recursive_mutex global_mutex{}; // global mutex used everywhere

//---------------------------------------------------------//

// This actually creates only one instance of CSteamClient.
CSteamClient* CreateClientInterface(const char* version)
{
	std::lock_guard<std::recursive_mutex> lock(global_mutex);
	static CSteamClient* s_client_interface;

	if (!s_client_interface)
		s_client_interface = new CSteamClient();

	s_client_interface->m_pszVersion = version;

	return s_client_interface;
}

// lool does delete only...
void DestroyClientInterface(ISteamClient* client_interface) 
{
	delete client_interface;
}

S_API bool S_CALLTYPE SteamAPI_Init() 
{ 
	std::lock_guard<std::recursive_mutex> lock(global_mutex);

	// check if already initialized
	if (g_flLastInitTime != 0.0f)
		return true;

	g_pSteamClient = CreateClientInterface(STEAMCLIENT_INTERFACE_VERSION);
	
	// Should only be called in SteamGameServer_Init, but i don't want random AVs appear somewhere
	g_pSteamClientGameServer = CreateClientInterface(STEAMCLIENT_INTERFACE_VERSION);

	steam_startup();
	g_flLastInitTime = Plat_FloatTime();

	return true; 
}

S_API bool S_CALLTYPE SteamAPI_InitSafe() {
	return SteamAPI_Init();
}

S_API void S_CALLTYPE SteamAPI_Shutdown() 
{
	std::lock_guard<std::recursive_mutex> lock(global_mutex);

	// not doing too fast Init && Shutdown
	if (Plat_FloatTime() - g_flLastInitTime < 10.0f)
		return;

	DestroyClientInterface(g_pSteamClient);
	DestroyClientInterface(g_pSteamClientGameServer);
	g_flLastInitTime = 0.0f;
	steam_shutdown();
}

// checks if a local Steam client is running 
S_API bool S_CALLTYPE SteamAPI_IsSteamRunning() 
{ 
	std::lock_guard<std::recursive_mutex> lock(global_mutex);

	if (g_flLastInitTime == 0.0f)
		return false;

	if (g_pSteamClient != nullptr && g_pSteamClientGameServer != nullptr)
		return true;
	
	return false; 
}

S_API bool S_CALLTYPE SteamAPI_RestartAppIfNecessary(uint32 unOwnAppID) { return false; }

// crash dump recording functions
S_API void S_CALLTYPE SteamAPI_WriteMiniDump(uint32 uStructuredExceptionCode, void* pvExceptionInfo, uint32 uBuildID) { }
S_API void S_CALLTYPE SteamAPI_SetMiniDumpComment(const char* pchMsg) { }

// interface pointers, configured by SteamAPI_Init()
S_API ISteamClient* S_CALLTYPE SteamClient() { 
	return g_pSteamClient;
}

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

	g_pSteamClient->RunCallbacks(true, false);
}

// TODO: Internal functions used by the utility CCallback objects to receive callbacks
S_API void S_CALLTYPE SteamAPI_RegisterCallback(class CCallbackBase* pCallback, int iCallback) 
{
	std::lock_guard<std::recursive_mutex> lock(global_mutex);
	g_pSteamClient->RegisterCallback(pCallback, iCallback);
}
S_API void S_CALLTYPE SteamAPI_UnregisterCallback(class CCallbackBase* pCallback) 
{
	std::lock_guard<std::recursive_mutex> lock(global_mutex);
	g_pSteamClient->UnregisterCallback(pCallback);
}
// Internal functions used by the utility CCallResult objects to receive async call results
S_API void S_CALLTYPE SteamAPI_RegisterCallResult(class CCallbackBase* pCallback, 
	SteamAPICall_t hAPICall)
{
	std::lock_guard<std::recursive_mutex> lock(global_mutex);
	g_pSteamClient->RegisterCallResult(pCallback, hAPICall);
}
S_API void S_CALLTYPE SteamAPI_UnregisterCallResult(class CCallbackBase* pCallback, 
	SteamAPICall_t hAPICall) 
{
	std::lock_guard<std::recursive_mutex> lock(global_mutex);
	g_pSteamClient->UnregisterCallResult(pCallback, hAPICall);
}

// returns the filename path of the current running Steam process, used if you need to load an explicit steam dll by name
S_API const char* S_CALLTYPE SteamAPI_GetSteamInstallPath() 
{
	return nullptr;
}

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

S_API ISteamGameServer* S_CALLTYPE SteamGameServer() { return g_pSteamGameServer; }
S_API ISteamUtils* S_CALLTYPE SteamGameServerUtils() { return nullptr; }
S_API ISteamNetworking* S_CALLTYPE SteamGameServerNetworking() { return nullptr; }
S_API ISteamGameServerStats* S_CALLTYPE SteamGameServerStats() { return nullptr; }
S_API ISteamHTTP* S_CALLTYPE SteamGameServerHTTP() { return nullptr; }
S_API ISteamInventory* S_CALLTYPE SteamGameServerInventory() { return nullptr; }
S_API ISteamUGC* S_CALLTYPE SteamGameServerUGC() { return nullptr; }

S_API bool S_CALLTYPE SteamGameServer_Init(uint32 unIP, uint16 usSteamPort, uint16 usGamePort, uint16 usQueryPort, uint32 eServerMode, const char* pchVersionString) 
{
	std::lock_guard<std::recursive_mutex> lock(global_mutex);

	// check if already initialized
	if (g_flLastInitTime != 0.0f)
		return true;

	g_pSteamClientGameServer = CreateClientInterface(STEAMCLIENT_INTERFACE_VERSION);
	g_flLastInitTime = Plat_FloatTime();

	return true;
}

S_API bool S_CALLTYPE SteamGameServer_InitSafe(uint32 unIP, uint16 usSteamPort, uint16 usGamePort, uint16 usQueryPort, uint32 eServerMode, const char* pchVersionString)
{
	return SteamGameServer_Init(unIP, usSteamPort, usGamePort, usQueryPort, eServerMode, pchVersionString);
}

S_API void S_CALLTYPE SteamGameServer_Shutdown() 
{
	std::lock_guard<std::recursive_mutex> lock(global_mutex);

	// not doing too fast Init && Shutdown
	if (Plat_FloatTime() - g_flLastInitTime < 10.0f)
		return;

	DestroyClientInterface(g_pSteamClientGameServer);
	g_flLastInitTime = 0.0f;
}

S_API void S_CALLTYPE SteamGameServer_RunCallbacks() 
{ 
	// todo real callbacks
	g_pSteamClient->RunCallbacks(false, true);
}

S_API bool S_CALLTYPE SteamGameServer_BSecure() { return true; }
S_API uint64 S_CALLTYPE SteamGameServer_GetSteamID() { return g_pSteamGameServer->GetSteamID().ConvertToUint64(); }

S_API HSteamPipe S_CALLTYPE SteamGameServer_GetHSteamPipe() { return 0; }
S_API HSteamUser S_CALLTYPE SteamGameServer_GetHSteamUser() { return 0; }


S_API int SteamGameServer_GetIPCCallCount() { 
	return 0; 
}

// https://github.com/ValveSoftware/source-sdk-2013/blob/a36ead80b3ede9f269314c08edd3ecc23de4b160/src/public/steam/steam_api_internal.h#L30-L32
// SteamInternal_ContextInit takes a base pointer for the equivalent of
// struct { void (*pFn)(void* pCtx); uintptr_t counter; void *ptr; }
// Do not change layout or add non-pointer aligned data!
struct ContextInitData {
	void (*pFn)(void* pCtx) = nullptr;
	uintp counter{};
	CSteamAPIContext ctx{};
};

S_API void* S_CALLTYPE SteamInternal_ContextInit( void* pContextInitData ) 
{
	std::lock_guard<std::recursive_mutex> lock(global_mutex);

	auto contextInitData = reinterpret_cast<struct ContextInitData*>(pContextInitData);
	contextInitData->counter++;

	void* local_ctx = &contextInitData->ctx;

	return local_ctx;
}

S_API void* S_CALLTYPE SteamInternal_CreateInterface( const char* version ) 
{
	return CreateClientInterface(version);
}
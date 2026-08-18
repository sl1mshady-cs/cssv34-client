#pragma once
#include "steamclient/steamapps.h"
#include "steamclient/steamgameserver.h"
#include "steamclient/steammatchmaking.h"
#include "steamclient/steammatchmakingservers.h"
#include "steamclient/steamuser.h"

/*
* Just a stub for multiple interfaces that we dont have
* but engine wants to use
*/
struct interface_stub_t {
	virtual void vf0() {}
	virtual void vf1() {}
	virtual void vf2() {}
	virtual void vf3() {}
	virtual void vf4() {}
	virtual void vf5() {}
	virtual void vf6() {}
	virtual void vf7() {}
	virtual void vf8() {}
	virtual void vf9() {}
	virtual void vf10() {}
	virtual void vf11() {}
	virtual void vf12() {}
	virtual void vf13() {}
	virtual void vf14() {}
	virtual void vf15() {}
	virtual void vf16() {}
	virtual void vf17() {}
	virtual void vf18() {}
	virtual void vf19() {}
	virtual void vf20() {}
	virtual void vf21() {}
	virtual void vf22() {}
	virtual void vf23() {}
	virtual void vf24() {}
	virtual void vf25() {}
	virtual void vf26() {}
};

/*
* SteamClient interface. Fully inlined
*/
class CSteamClient : public ISteamClient
{
public:
	CSteamClient() {
		m_nullInterface = new interface_stub_t();
	}

	~CSteamClient() {
		delete m_nullInterface;
	}

	virtual HSteamPipe CreateSteamPipe() { return 0; }
	virtual bool BReleaseSteamPipe(HSteamPipe hSteamPipe) { return false; }
	virtual HSteamUser ConnectToGlobalUser(HSteamPipe hSteamPipe) { return 0; }
	virtual HSteamUser CreateLocalUser(HSteamPipe* phSteamPipe, EAccountType eAccountType) { return 0; }
	virtual void ReleaseUser(HSteamPipe hSteamPipe, HSteamUser hUser) {}

	virtual ISteamUser* GetISteamUser(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return g_pSteamUser;
	}
	virtual ISteamGameServer* GetISteamGameServer(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return g_pSteamGameServer;
	}
	virtual void SetLocalIPBinding(uint32 unIP, uint16 usPort) {}
	virtual ISteamFriends* GetISteamFriends(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return (ISteamFriends*)m_nullInterface;
	}
	virtual ISteamUtils* GetISteamUtils(HSteamPipe hSteamPipe, const char* pchVersion) 
	{ 
		return (ISteamUtils * )m_nullInterface; 
	}
	virtual ISteamMatchmaking* GetISteamMatchmaking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return g_pSteamMatchMaking;
	}
	virtual ISteamMatchmakingServers* GetISteamMatchmakingServers(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return g_pSteamMatchMakingServers;
	}
	virtual void* GetISteamGenericInterface(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return m_nullInterface;
	}
	virtual ISteamUserStats* GetISteamUserStats(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return (ISteamUserStats*)m_nullInterface;
	}
	virtual ISteamGameServerStats* GetISteamGameServerStats(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return (ISteamGameServerStats*)m_nullInterface;
	}
	virtual ISteamApps* GetISteamApps(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return g_pSteamApps;
	}

	//
	// Starting from now we just return 0 instead of stub
	//

	virtual ISteamNetworking* GetISteamNetworking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return 0;
	}
	virtual ISteamRemoteStorage* GetISteamRemoteStorage(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return 0;
	}
	virtual ISteamScreenshots* GetISteamScreenshots(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return 0;
	}
	virtual void RunFrame()
	{
	}
	virtual uint32 GetIPCCallCount() 
	{
		return 0;
	}
	virtual void SetWarningMessageHook(SteamAPIWarningMessageHook_t pFunction) 
	{
	}
	virtual bool BShutdownIfAllPipesClosed()
	{
		return true;
	}
	virtual ISteamHTTP* GetISteamHTTP(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return 0;
	}
	virtual ISteamUnifiedMessages* GetISteamUnifiedMessages(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return 0;
	}
	virtual ISteamController* GetISteamController(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return 0;//(ISteamController*)m_nullInterface;
	}
	virtual ISteamUGC* GetISteamUGC(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return 0;
	}
	virtual ISteamAppList* GetISteamAppList(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return 0;
	}
	virtual ISteamMusic* GetISteamMusic(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return 0;
	}
	virtual ISteamMusicRemote* GetISteamMusicRemote(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return 0;
	}
	virtual ISteamHTMLSurface* GetISteamHTMLSurface(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return 0;
	}
	virtual void Set_SteamAPI_CPostAPIResultInProcess(SteamAPI_PostAPIResultInProcess_t func) 
	{
	}
	virtual void Remove_SteamAPI_CPostAPIResultInProcess(SteamAPI_PostAPIResultInProcess_t func)
	{
	}
	virtual void Set_SteamAPI_CCheckCallbackRegisteredInProcess(SteamAPI_CheckCallbackRegistered_t func) 
	{
	}
	virtual ISteamInventory* GetISteamInventory(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion) 
	{
		return 0;
	}
	virtual ISteamVideo* GetISteamVideo(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char* pchVersion)
	{
		return 0;
	}
private:
	void* m_nullInterface;
};
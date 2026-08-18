#pragma once
#include <mutex>
#include "socket.h"
#include "tier0/threadtools.h"
#include "tier1/utlvector.h"
#include "tier1/netadr.h"
#include "steam/steam_api.h"

#define NET_UDP_RECVSIZE		8192
#define MAX_THREADS				200

#pragma pack(push)	// Change alignment
#pragma pack(1) 

struct TMasterReply {
	byte			bytOctet1;		// First IP octet.
	byte			bytOctet2;		// Second IP octet.
	byte			bytOctet3;		// Third IP octet.
	byte			bytOctet4;		// Fourth IP octet.
	unsigned short	sPort;			// Server Port
};

struct TMasterRequest {
	byte			bytRegionCode;
	netadr_t		masterAddress;
	char			szIPIterator[MAX_PATH];
	char			szFilter[MAX_PATH];
};

struct TServerIP {
	char			szIP[16];
	unsigned short	sPort;
};

struct TServerHandle {
	bool			bAllowRefresh;
	char			szIP[19];
	unsigned short	sPort;
};

#pragma pack(pop)

// Exception Handler
int exceptionhandler(unsigned int code, struct _EXCEPTION_POINTERS* ep);

class CServerList;

class CServerManager
{
public:
	CServerManager(CServerList*);
	~CServerManager(void);

private:
	TServerHandle* GetFirstServer();
	bool			GetNextServer(TServerHandle* pHandle);
	TServerHandle* GetCloseServer(TServerHandle* pHandle);

public:
	void ServerIterator();

public:
	static bool SetConnectionInfo(sockaddr_in& sckAddrIn, netadr_t& netAddress);

	void PingServer(unsigned int uAddr,
		unsigned short sPort,
		ISteamMatchmakingPingResponse* pResponse);

	void PlayerDetails(unsigned int uAddr,
		unsigned short sPort,
		ISteamMatchmakingPlayersResponse* pResponse);

	bool StartRefreshFavorites();
	bool StartRefresh(TMasterRequest* pRequest, bool bQuick = false);
	bool StartRefreshLan(unsigned short uAppId);
	bool StopRefresh();
	void RunFrame();
	void Clear();

private:
	TServerHandle* m_pServerHandle;
	CServerList* m_pServerList;

public:
	std::mutex m_Critical;
	CUtlVector<gameserveritem_t*> m_vecRefreshed;
	CUtlVector<TServerIP*> m_vecServer;
	CUtlVector<TServerIP*> m_vecQuick;
	unsigned int m_uActiveThreads;
	bool m_bIsDownloading;
	bool m_bIsRefresh;
	bool m_bIsQuick;

};
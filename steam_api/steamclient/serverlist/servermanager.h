#ifndef _SERVERMANAGER_H
#define _SERVERMANAGER_H

#ifdef _WIN32
#pragma once
#endif

#include <mutex>
#include "socket.h"
#include "steam/steam_api.h"

#define MAX_THREADS				200

// forward declarations
class CServerList;

// Requests & replies
// * VDC: https://developer.valvesoftware.com/wiki/Master_Server_Query_Protocol
//////////////
#pragma pack(push)	// Change alignment
#pragma pack(1)

struct TMasterReply {
	uint32	unIP;	// ip in network order, do ntohl!
	uint16	usPort;	// port in network order, do ntohs!
};

struct TMasterRequest {
	byte	regionCode;
	netadr_t masterAddress;
	char	szIPIterator[MAX_PATH];
	char	szFilter[MAX_PATH];
};

struct TServerHandle {
	bool	bAllowRefresh;
	netadr_t serverAddress;
};

#pragma pack(pop)

/*
* Server manager class
*/
class CServerManager
{
public:
	CServerManager(CServerList*);
	~CServerManager(void);

	void ServerIterator();

	int PingServer(uint32 unIP, uint16 usPort, ISteamMatchmakingPingResponse* pResponse);
	int PlayerDetails(uint32 unIP, uint16 usPort, ISteamMatchmakingPlayersResponse* pResponse);
	int ServerRules(uint32 unIP, uint16 usPort, ISteamMatchmakingRulesResponse* pResponse);
	void CancelServerQuery(int query);

	bool StartRefreshFavorites();
	bool StartRefresh(TMasterRequest* pRequest, bool bQuick = false);
	bool StartRefreshLan(unsigned short uAppId);
	bool StopRefresh();
	void RunFrame();
	void Clear();

private:
	TServerHandle*	GetFirstServer();
	bool			GetNextServer(TServerHandle* pHandle);
	TServerHandle*	GetCloseServer(TServerHandle* pHandle);

private:
	TServerHandle* m_pServerHandle;
	CServerList* m_pServerList;

public:
	std::mutex m_Critical;
	CUtlVector<gameserveritem_t*> m_vecRefreshed;
	CUtlVector<netadr_t*> m_vecServer;
	CUtlVector<netadr_t*> m_vecQuick;
	unsigned int m_uActiveThreads;
	bool m_bIsDownloading;
	bool m_bIsRefresh;
	bool m_bIsQuick;

};

#endif // _SERVERMANAGER_H
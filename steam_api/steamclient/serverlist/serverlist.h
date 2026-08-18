#ifndef _SERVERLIST_H
#define _SERVERLIST_H

#ifdef _WIN32
#pragma once
#endif

#include <mutex>
#include "tier1/utlvector.h"
#include "steam/steam_api.h"
#include "vdf.h"

class CServerManager;

struct query_t
{
	servernetadr_t addr;
	int serverID;
	float sendTime;
};

typedef enum
{
	NONE = 0,
	INFO_REQUESTED,
	INFO_RECEIVED
} QUERYSTATUS;

extern void v_strncpy(char *dest, const char *src, int bufsize);

struct TServer
{
	char chServerName[255];
	char chServerIPPort[24];
	unsigned long long ulLastPlayed;
};

class CServerList
{
public:
	CServerList(const CServerList &serverList, EServerType eType);
	~CServerList();

public:
	unsigned int		AddNewServer(gameserveritem_t * server, bool bAddToRefreshList = true);
	void				SetListParameters(unsigned int nAppID, EServerType eType, ISteamMatchmakingServerListResponse * target);
	bool				IsRefreshing(void);
	gameserveritem_t*	GetServer(unsigned int serverID);
	void				StopRefresh(void);
	void				StartRefresh(void);
	void				SetFilters(MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters);
	unsigned int		ServerCount(EServerType eType);
	const char*			GetFilters();
	void				ServerResponded(int serverID);
	void				RefreshComplete();
	void				RunFrame();
	bool				GetSingleServer(unsigned int nServerIP, unsigned short nServerPort, gameserveritem_t* serverItem);
	EServerType			GetType();
	void				GetServers(EServerType eType);
	void				PingServer(unsigned int a1, unsigned short a2, ISteamMatchmakingPingResponse * a3);
	void				PlayerDetails(unsigned int a1, unsigned short a2, ISteamMatchmakingPlayersResponse * a3);
	void				QuickRefresh();
	void				AddToFavorites(uint32 nAppID, uint32 nIP, uint16 nConnPort, uint32 rTime32LastPlayedOnServer);
	void				RemoveFromFavorites(uint32 nAppID, uint32 nIP, uint16 nConnPort);
	void				StartRefreshLan();
	bool				GetFavoriteGame( int iGame, uint32 *pnAppID, uint32 *pnIP, uint16 *pnConnPort, uint16 *pnQueryPort, uint32 *punFlags, uint32 *pRTime32LastPlayedOnServer );

private:
	void				GetRegionCodeToFilter(const char* szRegion);
	void				Clear(void);

	// VDF helper functions
	void				reindex(VdfKey * key, int keyIndex=1);
	VdfKey*				createNewKey(char * name=0, char * value=0);
	char*				copyStr(char * str);
	VdfKey*				createNewFavoriteTree(char * index, char * name,char * address, char * lastPlayed, char * appId);
	VdfKey*				DefaultVdfFile();

public:
	CUtlVector<servernetadr_t*>				m_RefreshList;			// List of servers available for quick refresh

private:
	ISteamMatchmakingServerListResponse*	m_pResponseTarget;		// Callback to notify the VGUI when we make some progress here
	CUtlVector<gameserveritem_t*>			m_Servers;				// We will store processed valid servers here so we can pass them to the VGUI
	bool									m_bQuerying;			// Is refreshing taking place?
	char									m_chFilters[255];		// Filters to use when querying
	EServerType								m_Type;					// Query type - internet, favorites, LAN, spectator, or friends
	int										m_nRefreshedServers;	// Count of servers refreshed
	CServerManager*							m_pQuery;				// Server query socket
	unsigned char							m_chRegion;				// Server region to query
	int										m_iUpdateSerialNumber;	// serial number of current update so we don't get results overlapping between different server list updates
	std::mutex								m_Critical;
	unsigned short							m_nAppID;

};

#endif // _SERVERLIST_H
#pragma once

#include "serverlist/serverlist.h"

class CServerList;

// ListRequest 
struct ListRequest
{
	EServerType	type;
};

/*
* CSteamMatchMakingServers
*/
class CSteamMatchMakingServers : public ISteamMatchmakingServers
{
public:
	CSteamMatchMakingServers();
	~CSteamMatchMakingServers();

	virtual HServerListRequest RequestInternetServerList( AppId_t iApp, ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse );
	virtual HServerListRequest RequestLANServerList( AppId_t iApp, ISteamMatchmakingServerListResponse *pRequestServersResponse );
	virtual HServerListRequest RequestFriendsServerList( AppId_t iApp, ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse );
	virtual HServerListRequest RequestFavoritesServerList( AppId_t iApp, ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse );
	virtual HServerListRequest RequestHistoryServerList( AppId_t iApp, ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse );
	virtual HServerListRequest RequestSpectatorServerList( AppId_t iApp, ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse );

	// Releases the asynchronous request object and cancels any pending query on it if there's a pending query in progress.
	// RefreshComplete callback is not posted when request is released.
	virtual void ReleaseRequest( HServerListRequest hServerListRequest );

	virtual gameserveritem_t* GetServerDetails(HServerListRequest hRequest, int iServer);

	virtual void CancelQuery(HServerListRequest hRequest);

	virtual void RefreshQuery(HServerListRequest hRequest);

	virtual bool IsRefreshing(HServerListRequest hRequest);

	virtual int GetServerCount(HServerListRequest hRequest);

	virtual void RefreshServer(HServerListRequest hRequest, int a2);

	// Request updated ping time and other details from a single server
	virtual HServerQuery PingServer( uint32 unIP, uint16 usPort, ISteamMatchmakingPingResponse *pRequestServersResponse ); 

	// Request the list of players currently playing on a server
	virtual HServerQuery PlayerDetails( uint32 unIP, uint16 usPort, ISteamMatchmakingPlayersResponse *pRequestServersResponse );

	// Request the list of rules that the server is running (See ISteamGameServer::SetKeyValue() to set the rules server side)
	virtual HServerQuery ServerRules( uint32 unIP, uint16 usPort, ISteamMatchmakingRulesResponse *pRequestServersResponse ); 

	// Cancel an outstanding Ping/Players/Rules query from above.  You should call this to cancel
	virtual void CancelServerQuery( HServerQuery hServerQuery ); 

	// helper functions, added by us, they have got nothing to do with steam/game interface
	virtual ListRequest* CreateRequest(EServerType type);
	virtual int AddToFavorites(uint32 nAppID, uint32 nIP, uint16 nConnPort, uint32 rTime32LastPlayedOnServer);
	virtual bool RemoveFromFavorites(uint32 nAppID, uint32 nIP, uint16 nConnPort);
	virtual bool GetFavoriteGame( int iGame, uint32 *pnAppID, uint32 *pnIP, uint16 *pnConnPort, uint16 *pnQueryPort, uint32 *punFlags, uint32 *pRTime32LastPlayedOnServer );
	virtual void Refresh();
	virtual EServerType GetActiveType();
	virtual HServerListRequest GetCurrentRequest();
	virtual void RunFrame();

private:
	EServerType m_eActiveType;
	ListRequest* m_pRequests[6];
	CServerList* m_pServerList[6];
	int m_iAppId;
};

extern CSteamMatchMakingServers* g_pSteamMatchMakingServers;
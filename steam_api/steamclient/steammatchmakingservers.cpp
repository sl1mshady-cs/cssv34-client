#include "steammatchmakingservers.h"

static CSteamMatchMakingServers s_steammatchmakingservers;
CSteamMatchMakingServers* g_pSteamMatchMakingServers = &s_steammatchmakingservers;

CSteamMatchMakingServers::CSteamMatchMakingServers()
{
	m_eActiveType = eInvalidServer;

	for (int i = 0; i < 6; i++)
	{
		m_pRequests[i] = new ListRequest();
		m_pRequests[i]->type = eInvalidServer;
	}
}

CSteamMatchMakingServers::~CSteamMatchMakingServers()
{
	for (int idx = 0; idx < 6; idx++)
	{
		if (m_pServerList[idx])
		{
			delete m_pServerList[idx];
		}
	}
}

ListRequest* CSteamMatchMakingServers::CreateRequest(EServerType type)
{
	if (m_pRequests[type]->type == eInvalidServer)
		m_pRequests[type]->type = type;

	return m_pRequests[type];
}

HServerListRequest CSteamMatchMakingServers::RequestInternetServerList(AppId_t iApp, ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse* pRequestServersResponse)
{
	m_eActiveType = eLANServer;

	// Check to see if the list exists for this type of servers. If not, create a new instance.
	if (!m_pServerList[m_eActiveType])
	{
		m_pServerList[m_eActiveType] = new CServerList(*m_pServerList[m_eActiveType], m_eActiveType);
	}

	// Make sure to tell our list about a callback it should process
	m_pServerList[m_eActiveType]->SetListParameters(iApp, m_eActiveType, pRequestServersResponse);

	// Process filters as passed from the VGUI
	if (ppchFilters && nFilters)
		m_pServerList[m_eActiveType]->SetFilters(ppchFilters, nFilters);

	// Go go go!!!
	if (iApp != 0)
	{
		this->Refresh();
	}

	return CreateRequest(m_eActiveType);
}

HServerListRequest CSteamMatchMakingServers::RequestLANServerList(unsigned int a1, ISteamMatchmakingServerListResponse* a2)
{
	m_eActiveType = eLANServer;

	if (!m_pServerList[m_eActiveType])
	{
		m_pServerList[m_eActiveType] = new CServerList(*m_pServerList[m_eActiveType], m_eActiveType);
	}

	m_pServerList[m_eActiveType]->SetListParameters(a1, m_eActiveType, a2);

	//m_pServerList[m_eActiveType]->StartRefreshLan(a1);
	m_pServerList[m_eActiveType]->StartRefreshLan();

	return CreateRequest(m_eActiveType);
}
HServerListRequest CSteamMatchMakingServers::RequestFriendsServerList(uint32 iApp, MatchMakingKeyValuePair_t** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse* pRequestServersResponse)
{
	m_eActiveType = eFriendsServer;

	if (!m_pServerList[m_eActiveType])
	{
		m_pServerList[m_eActiveType] = new CServerList(*m_pServerList[m_eActiveType], m_eActiveType);
	}

	// Make sure to tell our list about a callback it should process
	m_pServerList[m_eActiveType]->SetListParameters(iApp, m_eActiveType, pRequestServersResponse);

	return CreateRequest(m_eActiveType);
}

HServerListRequest CSteamMatchMakingServers::RequestFavoritesServerList(uint32 iApp, MatchMakingKeyValuePair_t** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse* pRequestServersResponse)
{
	m_eActiveType = eFavoritesServer;

	if (!m_pServerList[m_eActiveType])
	{
		m_pServerList[m_eActiveType] = new CServerList(*m_pServerList[m_eActiveType], m_eActiveType);
	}

	// Make sure to tell our list about a callback it should process
	m_pServerList[m_eActiveType]->SetListParameters(iApp, m_eActiveType, pRequestServersResponse);

	// Process filters as passed from the VGUI
	if (ppchFilters && nFilters)
		m_pServerList[m_eActiveType]->SetFilters(ppchFilters, nFilters);

	// Go go go!!!
	if (iApp != 0)
		m_pServerList[m_eActiveType]->GetServers(m_eActiveType);

	return CreateRequest(m_eActiveType);
}

HServerListRequest CSteamMatchMakingServers::RequestHistoryServerList(uint32 iApp, MatchMakingKeyValuePair_t** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse* pRequestServersResponse)
{
	m_eActiveType = eHistoryServer;

	if (!m_pServerList[m_eActiveType])
	{
		m_pServerList[m_eActiveType] = new CServerList(*m_pServerList[m_eActiveType], m_eActiveType);
	}
	// Make sure to tell our list about a callback it should process
	m_pServerList[m_eActiveType]->SetListParameters(iApp, m_eActiveType, pRequestServersResponse);

	// Process filters as passed from the VGUI
	if (ppchFilters && nFilters)
		m_pServerList[m_eActiveType]->SetFilters(ppchFilters, nFilters);

	// Go go go!!!
	if (iApp != 0)
		m_pServerList[m_eActiveType]->GetServers(m_eActiveType);

	return CreateRequest(m_eActiveType);
}

HServerListRequest CSteamMatchMakingServers::RequestSpectatorServerList(uint32 iApp, MatchMakingKeyValuePair_t** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse* pRequestServersResponse)
{
	m_eActiveType = eSpectatorServer;

	if (!m_pServerList[m_eActiveType])
	{
		m_pServerList[m_eActiveType] = new CServerList(*m_pServerList[m_eActiveType], m_eActiveType);
	}

	m_pServerList[m_eActiveType]->SetListParameters(iApp, m_eActiveType, pRequestServersResponse);
	if (ppchFilters && nFilters)
		m_pServerList[m_eActiveType]->SetFilters(ppchFilters, nFilters);

	if (iApp != 0)
	{
		this->Refresh();
	}

	return CreateRequest(m_eActiveType);
}

void CSteamMatchMakingServers::ReleaseRequest(HServerListRequest hServerListRequest) {
	hServerListRequest = nullptr;
}

gameserveritem_t* CSteamMatchMakingServers::GetServerDetails(HServerListRequest hRequest, int iServer)
{
	if (m_eActiveType != eInvalidServer)
		return m_pServerList[m_eActiveType]->GetServer(iServer);

	return nullptr;
}

void CSteamMatchMakingServers::CancelQuery(HServerListRequest hRequest)
{
	if (!hRequest)
		return;

	ListRequest* request = (ListRequest*)hRequest;

	if (request->type == eInvalidServer || m_eActiveType == eInvalidServer) 
		return;

	this->m_pServerList[m_eActiveType]->StopRefresh();
}
void CSteamMatchMakingServers::RefreshQuery(HServerListRequest hRequest)
{
	if (!hRequest)
		return;

	ListRequest* request = (ListRequest*)hRequest;

	if (request->type == eInvalidServer || m_eActiveType == eInvalidServer) 
		return;

	this->m_pServerList[m_eActiveType]->QuickRefresh();
}
bool CSteamMatchMakingServers::IsRefreshing(HServerListRequest hRequest)
{
	if (!hRequest)
		return false;

	ListRequest* request = (ListRequest*)hRequest;

	if (request->type != eInvalidServer && m_eActiveType != eInvalidServer)
	{
		if (m_pServerList[m_eActiveType])
		{
			return this->m_pServerList[m_eActiveType]->IsRefreshing();
		}
	}
	return false;
}
int CSteamMatchMakingServers::GetServerCount(HServerListRequest hRequest)
{
	if (!hRequest)
		return 0;

	ListRequest* request = (ListRequest*)hRequest;

	m_eActiveType = request->type;
	if (!m_pServerList[m_eActiveType])
	{
		m_pServerList[m_eActiveType] = new CServerList(*m_pServerList[m_eActiveType], m_eActiveType);
	}
	return m_pServerList[m_eActiveType]->ServerCount(m_eActiveType);
}

void CSteamMatchMakingServers::RefreshServer(HServerListRequest hRequest, int a2)
{
	// voided
}

HServerQuery CSteamMatchMakingServers::PingServer(unsigned int a1, unsigned short a2, ISteamMatchmakingPingResponse* a3)
{
	if (m_eActiveType != eInvalidServer)
		m_pServerList[m_eActiveType]->PingServer(a1, a2, a3);

	return 1;
}

HServerQuery CSteamMatchMakingServers::PlayerDetails(unsigned int a1, unsigned short a2, ISteamMatchmakingPlayersResponse* a3)
{
	if (m_eActiveType != eInvalidServer)
		m_pServerList[m_eActiveType]->PlayerDetails(a1, a2, a3);

	return 2;
}

HServerQuery CSteamMatchMakingServers::ServerRules(unsigned int a1, unsigned short a2, ISteamMatchmakingRulesResponse* a3)
{
	return 3;
}

void CSteamMatchMakingServers::CancelServerQuery(HServerQuery query)
{
	return;
}

void CSteamMatchMakingServers::Refresh()
{
	if (m_eActiveType != eInvalidServer)
		m_pServerList[m_eActiveType]->StartRefresh();
}

void CSteamMatchMakingServers::RunFrame()
{
	if (m_eActiveType != eInvalidServer) 
		m_pServerList[m_eActiveType]->RunFrame();
}

EServerType CSteamMatchMakingServers::GetActiveType()
{
	return m_eActiveType;
}

HServerListRequest CSteamMatchMakingServers::GetCurrentRequest()
{
	return m_pRequests[m_eActiveType];
}

int CSteamMatchMakingServers::AddToFavorites(uint32 nAppID, uint32 nIP, uint16 nConnPort, uint32 rTime32LastPlayedOnServer)
{
	if (nAppID)
	{
		m_eActiveType = eFavoritesServer;
	}
	else
		m_eActiveType = eHistoryServer;

	m_pServerList[m_eActiveType]->AddToFavorites(nAppID, nIP, nConnPort, rTime32LastPlayedOnServer);
	m_eActiveType = eInvalidServer;
	return 1;
}

bool CSteamMatchMakingServers::RemoveFromFavorites(uint32 nAppID, uint32 nIP, uint16 nConnPort)
{
	m_eActiveType = eFavoritesServer;
	m_pServerList[m_eActiveType]->RemoveFromFavorites(nAppID, nIP, nConnPort);
	return true;
}

bool CSteamMatchMakingServers::GetFavoriteGame(int iGame, uint32* pnAppID, uint32* pnIP, uint16* pnConnPort, uint16* pnQueryPort, uint32* punFlags, uint32* pRTime32LastPlayedOnServer)
{
	m_eActiveType = eFavoritesServer;
	return m_pServerList[m_eActiveType]->GetFavoriteGame(iGame, pnAppID, pnIP, pnConnPort, pnQueryPort, punFlags, pRTime32LastPlayedOnServer);
}
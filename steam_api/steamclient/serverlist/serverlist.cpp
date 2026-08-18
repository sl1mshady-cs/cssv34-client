#include "socket.h"
#include "serverlist.h"
#include "steammatchmakingservers.h"
#include "servermanager.h"
#include "logging.h"

extern CLoggingSystem* Logger;
extern bool g_bLogging;
extern char g_pchServerBrowser[MAX_PATH];
extern char g_chMasterServer[MAX_PATH];

CServerList::CServerList(const CServerList& serverList, EServerType eType)
{
	// Initialize m_bQuerying
	m_bQuerying = false;

	// Set the region to ALL by default
	m_chRegion = 0xFF;

	// Initialize filter string
	strcpy(m_chFilters, "");

	// Set the MatchMaking type for this CServerList instance
	m_Type = eType;

	// Calculate max sockets based on users' rate
	int internetSpeed;
	char steamRate[16] = "-1";

	//getRegistry("Software\\Valve\\Steam", "Rate", steamRate, (DWORD)16);
	if (!strcmp(steamRate, "-1"))
		// Default to DSL speed if no reg key found (i.e. if Steam is not installed)
		strcpy(steamRate, "7500");

	internetSpeed = atoi(steamRate);
	int maxSockets = (255 * internetSpeed) / 10000;
	if (internetSpeed < 6000)
	{
		// Reduce the number of active queries again for slow internet speeds
		maxSockets /= 2;
	}

	m_nRefreshedServers = 0;

	// Initialize the query socket -> Matthias :p
	m_pQuery = new CServerManager(this);
}

CServerList::~CServerList()
{
	delete m_pQuery;
}

unsigned int CServerList::AddNewServer(gameserveritem_t* server, bool bAddToRefreshList)
{
	HServerListRequest hRequest = g_pSteamMatchMakingServers->GetCurrentRequest();

	if (bAddToRefreshList)
	{
		// Add the server to refresh list for use with Quick Refresh
		unsigned int serverID = m_Servers.AddToTail(server);
		server->serverID = serverID;
		m_Servers[serverID]->serverID = serverID;

		servernetadr_t* servers = new servernetadr_t;
		servers->Init(m_Servers[serverID]->m_NetAdr.GetIP(),
			m_Servers[serverID]->m_NetAdr.GetConnectionPort(),
			m_Servers[serverID]->m_NetAdr.GetQueryPort());
		m_RefreshList.AddToTail(servers);

		// Notify the Game UI of a new server response
		m_pResponseTarget->ServerResponded(hRequest, serverID);
		return serverID;
	}
	else
	{
		// We have to do this or GetServer will always fail
		// Result: Dummy

		for (int i = 0; i < m_Servers.Count(); i++)
		{
			if (!m_Servers.IsValidIndex(i))
				continue;

			if (!strcmp(m_Servers[i]->m_NetAdr.GetConnectionAddressString(), server->m_NetAdr.GetConnectionAddressString()))
			{
				m_pResponseTarget->ServerResponded(hRequest, i);
				return i;
			}
		}
	}

	// Nothing found.
	m_pResponseTarget->ServerResponded(hRequest, 0);
	return 0;
}

void CServerList::SetListParameters(unsigned int nAppID, EServerType eType, ISteamMatchmakingServerListResponse* target)
{
	//if (g_bLogging) Logger->Write("%s\n", __FUNCTION__);
	m_pResponseTarget = target;
	m_Type = eType;
	m_nAppID = nAppID;
}

bool CServerList::IsRefreshing(void)
{
	return m_bQuerying;
}

gameserveritem_t* CServerList::GetServer(unsigned int serverID)
{
	//if (g_bLogging) Logger->Write("%s\n", __FUNCTION__);
	if (m_Servers.IsValidIndex(serverID))
	{
		return m_Servers[serverID];
	}

	// return a dummy -> Caused via invalid id
	gameserveritem_t* dummyServer = new gameserveritem_t;
	memset(dummyServer, 0, sizeof(dummyServer));
	return dummyServer;
}

void CServerList::StopRefresh(void)
{
	m_nRefreshedServers = 0;
	m_bQuerying = false;
	m_pQuery->StopRefresh();
}

void CServerList::StartRefresh(void)
{
	Clear();
	m_bQuerying = true;
	m_nRefreshedServers = 0;

	TMasterRequest* pRequest = new TMasterRequest;
	pRequest->bytRegionCode = m_chRegion;
	pRequest->masterAddress = netadr_t(g_chMasterServer);

	strcpy(pRequest->szIPIterator, "0.0.0.0:0");
	strcpy(pRequest->szFilter, GetFilters());

	m_pQuery->StartRefresh(pRequest);
}

void CServerList::SetFilters(MatchMakingKeyValuePair_t** ppchFilters, uint32 nFilters)
{
	MatchMakingKeyValuePair_t* pchFilters = *ppchFilters;

	for (unsigned int i = 0; i < nFilters; i++)
	{
		char chFilter[255] = "";
		// "region" value should not be included in filter string
		if (!_stricmp(pchFilters[i].m_szKey, "region"))
		{
			GetRegionCodeToFilter(pchFilters[i].m_szValue);
			continue;
		}

		// is "proxy" key set even if we are looking for internet server list ?
		if (!_stricmp(pchFilters[i].m_szKey, "proxy"))
		{
			if (m_Type != eSpectatorServer)
				continue;
		}

		if (!strcmp(pchFilters[i].m_szKey, "gametype"))
		{
			// prevent tags - disable Custom games :/
			continue;
		}
		else
		{
			strcat(chFilter, "\\");
			strcat(chFilter, pchFilters[i].m_szKey);
			strcat(chFilter, "\\");
			strcat(chFilter, pchFilters[i].m_szValue);
		}

		// does the filter already exist? We don't want to have duplicate filters.
		if (!strstr(m_chFilters, chFilter))
		{
			strcat(m_chFilters, chFilter);
		}

		// is EServerType set to eSpectatorServer but there is no filter key?
		if (m_Type == eSpectatorServer)
			if (!strstr(m_chFilters, "proxy"))
				strcat(m_chFilters, "\\proxy\\1");
	}
	if (g_bLogging) Logger->Write("Filter string: %s\n", m_chFilters);
}

void CServerList::GetRegionCodeToFilter(const char* szRegion)
{
	//if (g_bLogging) Logger->Write("%s\n", __FUNCTION__);
	m_chRegion = (char)atoi(szRegion);
}

unsigned int CServerList::ServerCount(EServerType eType)
{
	VdfKey* VdfFile;
	unsigned int nNumServers;

	switch ((int)eType)
	{
	case eFavoritesServer:
		VdfFile = vdf::parse(g_pchServerBrowser);

		if (!VdfFile)
			return 0;

		nNumServers = 0;

		if (!VdfFile->firstChild)
			return 0;

		VdfFile = VdfFile->firstChild;

		while (_stricmp(VdfFile->name, "Favorites"))
		{
			if (!VdfFile->nextSibiling)
				return 0;
			VdfFile = VdfFile->nextSibiling;
		}

		if (VdfFile->firstChild)
		{
			VdfFile = VdfFile->firstChild;
			nNumServers++;

			while (VdfFile->nextSibiling)
			{
				nNumServers++;
				VdfFile = VdfFile->nextSibiling;
			}
		}

		return nNumServers;

	case eHistoryServer:
		VdfFile = vdf::parse(g_pchServerBrowser);

		if (!VdfFile)
			return 0;

		nNumServers = 0;
		VdfFile = VdfFile->firstChild;

		while (_stricmp(VdfFile->name, "History"))
		{
			VdfFile = VdfFile->nextSibiling;
		}

		if (VdfFile->firstChild)
		{
			VdfFile = VdfFile->firstChild;
			nNumServers++;

			while (VdfFile->nextSibiling)
			{
				nNumServers++;
				VdfFile = VdfFile->nextSibiling;
			}
		}

		return nNumServers;

	default:
		return m_Servers.Count();
	}

}

void CServerList::Clear(void)
{
	//if (g_bLogging) Logger->Write("%s\n", __FUNCTION__);
	if (m_pQuery)
	{
		m_pQuery->Clear();
	}
	StopRefresh();
	m_Servers.RemoveAll();
	m_RefreshList.RemoveAll();
}

const char* CServerList::GetFilters()
{
	//if (g_bLogging) Logger->Write("%s\n", __FUNCTION__);
	return m_chFilters;
}

void CServerList::ServerResponded(int serverID)
{
	HServerListRequest hRequest = g_pSteamMatchMakingServers->GetCurrentRequest();
	if (m_Servers.IsValidIndex(serverID))
	{
		m_pResponseTarget->ServerResponded(hRequest, serverID);
		m_nRefreshedServers++;
	}
}

void CServerList::RefreshComplete()
{
	m_bQuerying = false;
	HServerListRequest hRequest = g_pSteamMatchMakingServers->GetCurrentRequest();
	m_pResponseTarget->RefreshComplete(hRequest, eServerResponded);
}

void CServerList::RunFrame()
{
	if (m_pQuery)
	{
		m_pQuery->RunFrame();
	}
}

bool CServerList::GetSingleServer(unsigned int nServerIP, unsigned short nServerPort, gameserveritem_t* serverItem)
{
	for (int idx = 0; m_Servers.IsValidIndex(idx); idx++)
	{
		servernetadr_t netadr = m_Servers[idx]->m_NetAdr;
		if (netadr.GetIP() == nServerIP && netadr.GetConnectionPort() == nServerPort)
		{
			*serverItem = *m_Servers[idx];
			return true;
		}
	}
	return false;
}

EServerType CServerList::GetType()
{
	return m_Type;
}

void CServerList::GetServers(EServerType eType)
{
	char pchType[16];

	if (eType == eFavoritesServer)
	{
		strcpy(pchType, "Favorites");
	}
	else if (eType == eHistoryServer)
	{
		strcpy(pchType, "History");
	}
	else return;

	m_RefreshList.RemoveAll();

	// Initialize the VDF file
	VdfKey* VdfFile = (VdfKey*)malloc(sizeof(VdfKey));
	VdfFile = vdf::parse(g_pchServerBrowser);

	if (!VdfFile)
	{
		m_bQuerying = false;
		return;
	}

	VdfFile = VdfFile->firstChild;

	// Find the requested subkey
	while (_stricmp(VdfFile->name, pchType))
	{
		// Return if the subkey doesn't exist
		if (!VdfFile->nextSibiling)
		{
			m_bQuerying = false;
			free(VdfFile);
			return;
		}

		VdfFile = VdfFile->nextSibiling;
	}

	if (!VdfFile->firstChild)
	{
		m_bQuerying = false;
		free(VdfFile);
		return;
	}

	char test[24] = "";
	char* port;

	VdfFile = VdfFile->firstChild;
	while (VdfFile)
	{
		servernetadr_t* servers = new servernetadr_t;
		// Parse the server info and add each server to m_RefreshList
		if (!VdfFile->firstChild->nextSibiling->value)
		{
			VdfFile = NULL;
			continue;
		}
		strcpy(test, "");
		strcpy(test, VdfFile->firstChild->nextSibiling->value);
		port = strchr(test, ':') + 1;
		servers->SetConnectionPort(atoi(port));
		servers->SetQueryPort(atoi(port));
		port = port - 1;
		*port = 0;
		servers->SetIP(inet_addr(test));
		m_RefreshList.AddToTail(servers);

		if (!VdfFile->nextSibiling)
			VdfFile = NULL;
		else
			VdfFile = VdfFile->nextSibiling;
	}

	int a = m_RefreshList.Count();
	if (a)
	{
		m_bQuerying = true;
		m_pQuery->StartRefreshFavorites();
	}
}

void CServerList::PingServer(unsigned int a1, unsigned short a2, ISteamMatchmakingPingResponse* a3)
{
	m_pQuery->PingServer(a1, a2, a3);
}

void CServerList::PlayerDetails(unsigned int nServerIP, unsigned short nServerPort, ISteamMatchmakingPlayersResponse* a3)
{
	m_pQuery->PlayerDetails(nServerIP, nServerPort, a3);
}

void CServerList::QuickRefresh()
{
	m_bQuerying = true;
	//m_nRefreshedServers = 0;
	//m_Servers.RemoveAll();
	m_pQuery->StartRefresh(NULL, true);
}

void CServerList::AddToFavorites(uint32 nAppID, uint32 nIP, uint16 nConnPort, uint32 rTime32LastPlayedOnServer)
{
	VdfKey* VdfFile = vdf::parse(g_pchServerBrowser);
	VdfKey* VdfFavs = vdf::parse(g_pchServerBrowser);
	char* index = (char*)malloc(8);
	char* appID = (char*)malloc(8);
	char* port = (char*)malloc(8);
	char* ipadr = (char*)malloc(24);
	char* lastplayed = (char*)malloc(16);
	char* favsCaption = (char*)malloc(16);

	if (nAppID)
		strcpy(favsCaption, "Favorites");
	else
		strcpy(favsCaption, "History");

	strcpy(index, "0");

	if (!VdfFile)
	{
		VdfFile = DefaultVdfFile();
	}

	if (_stricmp(VdfFile->firstChild->nextSibiling->name, favsCaption))
	{
		VdfKey* OldVdf = vdf::parse(g_pchServerBrowser);
		OldVdf = VdfFile->firstChild->nextSibiling;
		VdfFavs = createNewKey(favsCaption);
		VdfFavs->nextSibiling = OldVdf;
		VdfFavs->firstChild = createNewKey(index);
		VdfFile->firstChild->nextSibiling = VdfFavs;
	}

	_itoa(nAppID, appID, 10);
	_itoa(nConnPort, port, 10);
	in_addr test;
	test.S_un.S_addr = ntohl(nIP);
	strcpy(ipadr, inet_ntoa(test));
	strcat(ipadr, ":");
	strcat(ipadr, port);
	_itoa(rTime32LastPlayedOnServer, lastplayed, 10);

	VdfFavs = VdfFile->firstChild->nextSibiling->firstChild;

	if (!VdfFavs)
		goto cleanup;

	while (VdfFavs->nextSibiling)
	{
		if (VdfFavs->firstChild && VdfFavs->firstChild->nextSibiling && VdfFavs->firstChild->nextSibiling->value)
			if (!strcmp(VdfFavs->firstChild->nextSibiling->value, ipadr))
				goto cleanup;

		VdfFavs = VdfFavs->nextSibiling;
	}

	VdfFavs->nextSibiling = createNewFavoriteTree(index, ipadr, ipadr, lastplayed, appID);

	vdf::save(g_pchServerBrowser, VdfFile);
	VdfFile = vdf::parse(g_pchServerBrowser);
	reindex(VdfFile->firstChild->nextSibiling->firstChild);
	vdf::save(g_pchServerBrowser, VdfFile);

cleanup:
	free(index);
	free(appID);
	free(port);
	free(ipadr);
	free(lastplayed);
	free(favsCaption);
	free(VdfFile);
	free(VdfFavs);
}

void CServerList::reindex(VdfKey* key, int keyIndex)
{
	free(key->name);
	key->name = (char*)malloc(5); // a bit huge, but so easier code :p
	sprintf(key->name, "%d", keyIndex);
	if (key->nextSibiling) reindex(key->nextSibiling, keyIndex + 1);
	//if (key->firstChild) reindex(key->firstChild); // this may not be needed, i don't know the file hierarchy
}

VdfKey* CServerList::createNewKey(char* name, char* value)
{
	VdfKey* newKey = (VdfKey*)malloc(sizeof(VdfKey));
	memset(newKey, 0, sizeof(VdfKey));
	newKey->name = name;
	newKey->value = value;
	return newKey;
}

char* CServerList::copyStr(char* str)
{
	char* tmp = (char*)malloc(strlen(str) + 1);
	strcpy(tmp, str);
	return tmp;
}

VdfKey* CServerList::createNewFavoriteTree(char* index, char* name, char* address, char* lastPlayed, char* appId)
{
	VdfKey* newFav = createNewKey(index);

	VdfKey* newName = createNewKey(copyStr("name"), name);
	newFav->firstChild = newName;

	VdfKey* newAddress = createNewKey(copyStr("address"), address);
	newName->nextSibiling = newAddress;

	VdfKey* newLastPlayed = createNewKey(copyStr("lastPlayed"), lastPlayed);
	newAddress->nextSibiling = newLastPlayed;

	VdfKey* newAppId = createNewKey(copyStr("AppId"), appId);
	newLastPlayed->nextSibiling = newAppId;

	return newFav;
}

void CServerList::RemoveFromFavorites(uint32 nAppID, uint32 nIP, uint16 nConnPort)
{
	VdfKey* VdfFile = (VdfKey*)malloc(sizeof(VdfKey));
	VdfFile = vdf::parse(g_pchServerBrowser);

	VdfKey* VdfFavs = (VdfKey*)malloc(sizeof(VdfKey));
	VdfFavs = vdf::parse(g_pchServerBrowser);

	VdfKey* OldVdf = (VdfKey*)malloc(sizeof(VdfKey));
	OldVdf = vdf::parse(g_pchServerBrowser);

	char* port = (char*)malloc(8);
	char* ip = (char*)malloc(24);
	char* favsCaption = (char*)malloc(16);

	strcpy(favsCaption, "Favorites");

	if (_stricmp(VdfFile->firstChild->nextSibiling->name, favsCaption))
	{
		goto cleanup;
	}

	_itoa(nConnPort, port, 10);
	in_addr test;
	test.S_un.S_addr = ntohl(nIP);
	strcpy(ip, inet_ntoa(test));
	strcat(ip, ":");
	strcat(ip, port);

	OldVdf = VdfFile->firstChild->nextSibiling->firstChild;
	VdfFavs = VdfFile->firstChild->nextSibiling->firstChild;

	while (VdfFavs)
	{
		if (_stricmp(VdfFavs->firstChild->nextSibiling->name, "address") || _stricmp(VdfFavs->firstChild->nextSibiling->value, ip))
		{
			VdfFavs = VdfFavs->nextSibiling;
		}
		else
			break;
	}

	if (!VdfFavs)
		goto cleanup;

	if (VdfFavs->nextSibiling)
	{
		OldVdf = VdfFavs->nextSibiling;
		VdfFavs->firstChild = 0;
		VdfFavs->nextSibiling = OldVdf;
	}
	else
	{
		VdfFavs->firstChild = 0;
	}

	vdf::save(g_pchServerBrowser, VdfFile);

	VdfFile = vdf::parse(g_pchServerBrowser);

	if (VdfFile->firstChild->nextSibiling->firstChild)
	{
		reindex(VdfFile->firstChild->nextSibiling->firstChild);
		vdf::save(g_pchServerBrowser, VdfFile);
	}
	else
	{
		OldVdf = VdfFile->firstChild->nextSibiling->nextSibiling;
		VdfFile->firstChild->nextSibiling = OldVdf;
		vdf::save(g_pchServerBrowser, VdfFile);
	}

cleanup:
	free(port);
	free(ip);
	free(favsCaption);
	free(VdfFile);
	free(OldVdf);
	if (VdfFavs)
		free(VdfFavs);

	return;
}

VdfKey* CServerList::DefaultVdfFile()
{
	VdfKey* VdfFile = createNewKey(copyStr("Filters"));

	VdfKey* gamelist = createNewKey(copyStr("gamelist"), copyStr("internet"));
	VdfFile->firstChild = gamelist;

	VdfKey* filters = createNewKey(copyStr("Filters"));
	gamelist->nextSibiling = filters;

	VdfKey* internet = createNewKey(copyStr("InternetGames"));
	filters->firstChild = internet;

	VdfKey* appid = createNewKey(copyStr("appid"), copyStr("0"));
	VdfKey* ping = createNewKey(copyStr("ping"), copyStr("0"));
	appid->nextSibiling = ping;
	VdfKey* nofull = createNewKey(copyStr("NoFull"), copyStr("0"));
	ping->nextSibiling = nofull;
	VdfKey* noempty = createNewKey(copyStr("NoEmpty"), copyStr("0"));
	nofull->nextSibiling = noempty;
	VdfKey* nopassword = createNewKey(copyStr("NoPassword"), copyStr("0"));
	noempty->nextSibiling = nopassword;
	VdfKey* secure = createNewKey(copyStr("Secure"), copyStr("0"));
	nopassword->nextSibiling = secure;

	internet->firstChild = appid;

	VdfKey* favorites = createNewKey(copyStr("FavoriteGames"));
	favorites->firstChild = appid;
	internet->nextSibiling = favorites;

	VdfKey* history = createNewKey(copyStr("HistoryGames"));
	history->firstChild = appid;
	favorites->nextSibiling = history;

	VdfKey* spectate = createNewKey(copyStr("SpectateGames"));
	spectate->firstChild = appid;
	history->nextSibiling = spectate;

	VdfKey* lan = createNewKey(copyStr("LanGames"));
	lan->firstChild = appid;
	spectate->nextSibiling = lan;

	VdfKey* friends = createNewKey(copyStr("FriendsGames"));
	friends->firstChild = appid;
	lan->nextSibiling = friends;

	vdf::save(g_pchServerBrowser, VdfFile);
	return VdfFile;
}

void CServerList::StartRefreshLan(/* uint32 uAppId */)
{
	Clear();
	m_bQuerying = true;
	m_nRefreshedServers = 0;

	// Broadcast to 27015
	m_pQuery->StartRefreshLan(27015);
}

bool CServerList::GetFavoriteGame(int iGame, uint32* pnAppID, uint32* pnIP, uint16* pnConnPort, uint16* pnQueryPort, uint32* punFlags, uint32* pRTime32LastPlayedOnServer)
{
	static char test[24] = "";
	static char* port;

	// Initialize the VDF file
	static VdfKey* VdfFile = (VdfKey*)malloc(sizeof(VdfKey));
	VdfFile = vdf::parse(g_pchServerBrowser);
	char Idx[4];
	_itoa(iGame + 1, Idx, 10);

	if (!VdfFile)
	{
		m_bQuerying = false;
		return false;
	}

	VdfFile = VdfFile->firstChild;

	// Find the requested subkey
	while (_stricmp(VdfFile->name, "Favorites"))
	{
		// Return if the subkey doesn't exist
		if (!VdfFile->nextSibiling)
		{
			m_bQuerying = false;
			return false;
		}

		VdfFile = VdfFile->nextSibiling;
	}

	if (!VdfFile->firstChild)
	{
		m_bQuerying = false;
		return false;
	}

	VdfFile = VdfFile->firstChild;

	while (strcmp(VdfFile->name, Idx))
	{
		if (VdfFile->nextSibiling)
			VdfFile = VdfFile->nextSibiling;
		else
			break;
	}

	if (!strcmp(VdfFile->name, Idx))
	{
		if (!_stricmp(VdfFile->firstChild->nextSibiling->name, "address"))
		{
			strcpy(test, "");
			strcpy(test, VdfFile->firstChild->nextSibiling->value);
			port = strchr(test, ':') + 1;
			*pnConnPort = atoi(copyStr(port));
			*pnQueryPort = atoi(copyStr(port));
			port = port - 1;
			*port = 0;
			*punFlags = 1;
			*pnIP = ntohl(inet_addr(copyStr(test)));
			*pRTime32LastPlayedOnServer = atoi(copyStr(VdfFile->firstChild->nextSibiling->nextSibiling->value));
			*pnAppID = atoi(copyStr(VdfFile->firstChild->nextSibiling->nextSibiling->nextSibiling->value));
			return true;
		}
	}
	return false;
}
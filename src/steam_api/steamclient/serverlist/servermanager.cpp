#include "servermanager.h"
#include "serverinfo.h"
#include "servermaster.h"
#include "serverlist.h"
#include "serverlan.h"

CServerManager::CServerManager(CServerList* pList)
{
	m_pServerList = pList;
	m_uActiveThreads = 0;
	m_bIsRefresh = false;
	m_bIsDownloading = false;
	m_pServerHandle = NULL;
}

CServerManager::~CServerManager(void)
{
}

void CServerManager::ServerIterator()
{
	if (!m_pServerHandle)
	{
		m_pServerHandle = GetFirstServer();
		if (m_pServerHandle)
		{
			if (m_pServerHandle->bAllowRefresh)
				CServerInfo* pInfo = new CServerInfo(this,
					m_pServerHandle->serverAddress);
		}
		else
		{
			m_bIsRefresh = false;
			Msg("ServerIterator #if!m_pServerHandle: set m_bIsRefresh to 0\n");
			m_pServerList->RefreshComplete();
		}
	}
	else
	{
		if (GetNextServer(m_pServerHandle))
		{
			if (m_pServerHandle->bAllowRefresh)
				CServerInfo* pInfo = new CServerInfo(this,
					m_pServerHandle->serverAddress);
		}
		else {
			m_pServerHandle = GetCloseServer(m_pServerHandle);
			Msg("ServerIterator #if.m_pServerHandle.!GetNextServer: set m_bIsRefresh to 0\n");
			m_bIsRefresh = false;
			m_pServerList->RefreshComplete();
		}
	}
}

bool CServerManager::StartRefresh(TMasterRequest* pRequest, bool bQuick)
{
	if (!m_bIsRefresh)
	{
		m_bIsQuick = bQuick;

		if (!bQuick)
		{
			Clear();
			CServerMaster* pMaster = new CServerMaster(this, pRequest);
		}
		else
		{
			m_vecServer.PurgeAndDeleteElements();

			for (int i = 0; i < m_pServerList->m_RefreshList.Count(); i++)
			{
				netadr_t *adr = new netadr_t();
				uint32 unIP = m_pServerList->m_RefreshList[i]->GetIP();
				uint16 usPort = m_pServerList->m_RefreshList[i]->GetQueryPort();
				adr->SetIPAndPort(unIP, usPort);

				m_vecServer.AddToTail(adr);
			}

			m_pServerList->m_RefreshList.PurgeAndDeleteElements();
		}

		m_bIsRefresh = true;
		return true;
	}

	return false;
}

bool CServerManager::StartRefreshFavorites()
{
	if (!m_bIsRefresh)
	{
		m_bIsQuick = false;

		m_vecServer.PurgeAndDeleteElements();

		for (int i = 0; i < m_pServerList->m_RefreshList.Count(); i++)
		{
			netadr_t* adr = new netadr_t();
			uint32 unIP = m_pServerList->m_RefreshList[i]->GetIP();
			uint16 usPort = m_pServerList->m_RefreshList[i]->GetQueryPort();
			adr->SetIPAndPort(unIP, usPort);

			m_vecServer.AddToTail(adr);
		}

		m_pServerList->m_RefreshList.PurgeAndDeleteElements();

		m_bIsRefresh = true;
		return true;
	}
	return false;
}

bool CServerManager::StartRefreshLan(unsigned short uAppId)
{
	if (m_bIsRefresh)
		return true;

	m_bIsQuick = false;
	netadr_t broadcast;
	broadcast.SetType(NA_BROADCAST);

	broadcast.SetPort(27015);
	CServerInfo* pLan1 = new CServerInfo(this, broadcast);

	broadcast.SetPort(27016);
	CServerInfo* pLan2 = new CServerInfo(this, broadcast);

	broadcast.SetPort(27017);
	CServerInfo* pLan3 = new CServerInfo(this, broadcast);

	broadcast.SetPort(27018);
	CServerInfo* pLan4 = new CServerInfo(this, broadcast);

	broadcast.SetPort(27019);
	CServerInfo* pLan5 = new CServerInfo(this, broadcast);

	broadcast.SetPort(27020);
	CServerInfo* pLan6 = new CServerInfo(this, broadcast);

	m_bIsRefresh = true;


	return true;
}

bool CServerManager::StopRefresh()
{
	//m_vecRefreshed.PurgeAndRemoveElements();
	m_bIsRefresh = false;
	return true;
}

TServerHandle* CServerManager::GetFirstServer()
{
	unsigned int uAttempts = 0;
	TServerHandle* pHandle = new TServerHandle;

	while (!m_vecServer.IsValidIndex(0))
	{
		pHandle->bAllowRefresh = false;
		return pHandle;
	}

	pHandle->bAllowRefresh = true;
	pHandle->serverAddress = *m_vecServer[0];
	m_vecServer.Remove(0);

	return pHandle;
}

bool CServerManager::GetNextServer(TServerHandle* pHandle)
{
	std::lock_guard<std::mutex> lock(m_Critical);
	if (m_vecServer.IsValidIndex(0))
	{
		if (m_uActiveThreads >= MAX_THREADS)
		{
			pHandle->bAllowRefresh = false;
			return true;
		}
		pHandle->bAllowRefresh = true;
		//if (!m_vecServer[0]->sPort)
		//{
		//	LeaveCriticalSection(&m_gCritical);
		//	return true;
		//}
		pHandle->serverAddress = *m_vecServer[0];
		m_vecServer.Remove(0);
		return true;
	}
	else
	{
		if (m_bIsDownloading)
		{
			pHandle->bAllowRefresh = false;
			return true;
		}
		else
		{
			if (m_uActiveThreads > 0)
			{
				pHandle->bAllowRefresh = false;
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	pHandle->bAllowRefresh = false;
	return false;
}

TServerHandle* CServerManager::GetCloseServer(TServerHandle* pHandle)
{
	delete pHandle;
	return NULL;
}

void CServerManager::Clear()
{
	StopRefresh();
	m_vecServer.PurgeAndDeleteElements();
}

void CServerManager::RunFrame()
{
	if (m_bIsRefresh)
	{
		if (m_vecRefreshed.IsValidIndex(0))
		{
			m_pServerList->AddNewServer(m_vecRefreshed[0], !m_bIsQuick);
			m_vecRefreshed.Remove(0);
		}
		ServerIterator();
	}
}

int CServerManager::PingServer(uint32 unIP, uint16 usPort, ISteamMatchmakingPingResponse* pResponse)
{
	CServerInfo* info = new CServerInfo(this, netadr_t(unIP, usPort), pResponse);

	return (int)info->GetID();
}

int CServerManager::PlayerDetails(uint32 unIP, uint16 usPort, ISteamMatchmakingPlayersResponse* pResponse)
{
	CServerInfo* info = new CServerInfo(netadr_t(unIP, usPort), pResponse);

	return (int)info->GetID();
}

int CServerManager::ServerRules(uint32 unIP, uint16 usPort, ISteamMatchmakingRulesResponse* pResponse)
{
	CServerInfo *info = new CServerInfo(netadr_t(unIP, usPort), pResponse);

	return (int)info->GetID();
}

void CServerManager::CancelServerQuery(int query) 
{
	std::thread::id threadID;
	uint32* ptr = (uint32*)&threadID; // we are doing ptr casting yaay (ub guaranteed)
	*ptr = (uint32)query;

	ServerRefreshThreads_Stop(threadID);
}
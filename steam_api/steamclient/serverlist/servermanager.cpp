#include "servermanager.h"
#include "serverinfo.h"
#include "servermaster.h"
#include "serverlist.h"
#include "serverlan.h"

int exceptionhandler(unsigned int code, struct _EXCEPTION_POINTERS * ep) {
	if (ep) 
	{
		/* Stop bother */
	}

	printf("Catching Error (Address: 0x%p): EBP: 0x%p\n",
		ep->ExceptionRecord->ExceptionAddress,
		/*ep->ContextRecord->Ebp*/nullptr);

	#ifdef _DEBUG
		//_asm { int 3 }	// Trigger breakpoint
	#endif

	if (code == ((DWORD)0xC0000005L)) {
		return 1;
	}
	else 
	{
		return 0;
	};
} 

CServerManager::CServerManager(CServerList* pList)
{
	// Start Socket
	WSADATA wsData;
	int uRes = WSAStartup( MAKEWORD(2, 0), &wsData);
	m_pServerList = pList;
	m_uActiveThreads = 0;
	m_bIsRefresh = false;
	m_bIsDownloading = false;
	m_pServerHandle = NULL;
}

CServerManager::~CServerManager(void)
{
	// Shutdown Socket
	WSACleanup();
}

bool CServerManager::SetConnectionInfo(SOCKADDR_IN &sckAddrIn, netadr_t& netAddress)
{
	// Resolve IP (If Host)
	if(netAddress.IsValid()) {
		// Remote Host information
		sckAddrIn.sin_family = AF_INET;
		netAddress.ToSockadr((sockaddr*)&sckAddrIn);
		return true;
	}

	return false;
}

void CServerManager::ServerIterator()
{
	if(!m_pServerHandle)
	{
		m_pServerHandle = GetFirstServer();
		if(m_pServerHandle)
		{
			if(m_pServerHandle->bAllowRefresh)
				CServerInfo * pInfo = new CServerInfo(this,
				m_pServerHandle->szIP, m_pServerHandle->sPort);	
		}
		else
		{
			m_bIsRefresh = false;
			m_pServerList->RefreshComplete();
		}
	}
	else
	{
		if(GetNextServer(m_pServerHandle))
		{
			if(m_pServerHandle->bAllowRefresh)
				CServerInfo * pInfo = new CServerInfo(this,
				m_pServerHandle->szIP, m_pServerHandle->sPort);
		} else {
			m_pServerHandle = GetCloseServer(m_pServerHandle);
			m_bIsRefresh = false;
			m_pServerList->RefreshComplete();
		}
	}
}

bool CServerManager::StartRefresh(TMasterRequest * pRequest, bool bQuick)
{
	if(!m_bIsRefresh)
	{
		m_bIsQuick = bQuick;
		if(!bQuick)
		{
			Clear();
			CServerMaster * pMaster = new CServerMaster(this, pRequest);
		}
		else
		{
			m_vecServer.RemoveAll();
			for(int i = 0; i < m_pServerList->m_RefreshList.Count(); i++)
			{
				TServerIP * pIP = new TServerIP;
				char szIP[19];
				unsigned int uAddr = m_pServerList->m_RefreshList[i]->GetIP();
				sprintf(szIP, "%d.%d.%d.%d",
					((unsigned char *)&uAddr)[3],
					((unsigned char *)&uAddr)[2],
					((unsigned char *)&uAddr)[1],
					((unsigned char *)&uAddr)[0]);
				strcpy(pIP->szIP, szIP);
				pIP->sPort = m_pServerList->m_RefreshList[i]->GetQueryPort();
				m_vecServer.AddToTail(pIP);
			}
			m_pServerList->m_RefreshList.RemoveAll();
		}
		m_bIsRefresh = true;
		return true;
	}
	return false;
}

bool CServerManager::StartRefreshFavorites()
{
	if(!m_bIsRefresh)
	{
		m_bIsQuick = false;
		m_vecServer.RemoveAll();
		for(int i = 0; i < m_pServerList->m_RefreshList.Count(); i++)
		{
			TServerIP * pIP = new TServerIP;
			char szIP[19];
			unsigned int uAddr = m_pServerList->m_RefreshList[i]->GetIP();
			sprintf(szIP, "%d.%d.%d.%d",
				((unsigned char *)&uAddr)[0],
				((unsigned char *)&uAddr)[1],
				((unsigned char *)&uAddr)[2],
				((unsigned char *)&uAddr)[3]);
			strcpy(pIP->szIP, szIP);
			pIP->sPort = m_pServerList->m_RefreshList[i]->GetQueryPort();
			m_vecServer.AddToTail(pIP);
		}
		m_pServerList->m_RefreshList.RemoveAll();
		m_bIsRefresh = true;
		return true;
	}
	return false;
}

bool CServerManager::StartRefreshLan(unsigned short uAppId)
{
	if(!m_bIsRefresh)
	{
		m_bIsQuick = false;
		CServerLan * pLan1 = new CServerLan(this, htons(27015));
		CServerLan * pLan2 = new CServerLan(this, htons(27016));
		CServerLan * pLan3 = new CServerLan(this, htons(27017));
		CServerLan * pLan4 = new CServerLan(this, htons(27018));
		CServerLan * pLan5 = new CServerLan(this, htons(27019));
		CServerLan * pLan6 = new CServerLan(this, htons(27020));
		m_bIsRefresh = true;
	}
	return true;
}

bool CServerManager::StopRefresh()
{
	//m_vecRefreshed.RemoveAll();
	m_bIsRefresh = false;
	return true;
}

TServerHandle * CServerManager::GetFirstServer() 
{
	unsigned int uAttempts = 0;
	TServerHandle * pHandle = new TServerHandle;

	while(!m_vecServer.IsValidIndex(0))
	{
		pHandle->bAllowRefresh = false;
		return pHandle;
	}

	pHandle->bAllowRefresh = true;
	strcpy(pHandle->szIP, m_vecServer[0]->szIP);
	pHandle->sPort = m_vecServer[0]->sPort;
	m_vecServer.Remove(0);

	return pHandle;
}

bool CServerManager::GetNextServer(TServerHandle * pHandle) 
{
	std::lock_guard<std::mutex> lock(m_Critical);
	if(m_vecServer.IsValidIndex(0))
	{
		if(m_uActiveThreads >= MAX_THREADS)
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
		pHandle->sPort = m_vecServer[0]->sPort;
		strcpy(pHandle->szIP, m_vecServer[0]->szIP);
		m_vecServer.Remove(0);
		return true;
	}
	else
	{
		if(m_bIsDownloading)
		{
			pHandle->bAllowRefresh = false;
			return true;
		}
		else
		{
			if(m_uActiveThreads > 0)
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

TServerHandle * CServerManager::GetCloseServer(TServerHandle * pHandle)
{
	delete pHandle;
	return NULL;
}

void CServerManager::Clear()
{
	//while(m_vecServer.IsValidIndex(0))
	//{
	//	delete m_vecServer[0];
	//	m_vecServer.Remove(0);
	//}
	StopRefresh();
	m_vecServer.RemoveAll();
}

void CServerManager::RunFrame()
{
	if(m_bIsRefresh)
	{
		if(m_vecRefreshed.IsValidIndex(0))
		{
			m_pServerList->AddNewServer(m_vecRefreshed[0], !m_bIsQuick);
			m_vecRefreshed.Remove(0) ;
		}
		ServerIterator();
	}
}

void CServerManager::PingServer(unsigned int uAddr, 
								unsigned short sPort, 
								ISteamMatchmakingPingResponse * pResponse)
{
	static char szIP[19];
	sprintf(szIP, "%d.%d.%d.%d",
		((unsigned char *)&uAddr)[3],
		((unsigned char *)&uAddr)[2],
		((unsigned char *)&uAddr)[1],
		((unsigned char *)&uAddr)[0]);

	CServerInfo * pServerInfo = new CServerInfo(this, szIP, sPort, pResponse);
}

void CServerManager::PlayerDetails(unsigned int uAddr, 
								   unsigned short sPort, 
								   ISteamMatchmakingPlayersResponse * pResponse)
{
	static char szIP[19];
	sprintf(szIP, "%d.%d.%d.%d",
		((unsigned char *)&uAddr)[3],
		((unsigned char *)&uAddr)[2],
		((unsigned char *)&uAddr)[1],
		((unsigned char *)&uAddr)[0]);

	CServerInfoPlayers * pPlayers = new CServerInfoPlayers(szIP, sPort, pResponse);
}
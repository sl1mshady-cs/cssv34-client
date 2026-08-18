#if 0
#include "masterserver.h"
#include "serverlist.h"

struct TThreadArguments {
	CMasterServer * pClassPtr;
	TMasterQueryRules * pQueryRules;
};

struct TQueryThread {
	CMasterServer * pMasterServer;
	TServerHandle2 * pHandle;
	//void * pCallback;
	DWORD dwStartTime;
};

struct TRefreshThread {
	//CServerList * pCallbackStatus;
	//CServerList * pCallbackComplete;
	CMasterServer * pMaster;
};

CMasterServer::CMasterServer(const CMasterServer &masterServer, CServerList* pList)
{
	// Initialize Socket
	m_bIsDownloadingList = false;
	m_uServerCountListed = 0;
	m_uActiveThreads = 0;
	m_bIsRefresh = false;
	m_bResponse = false;
	m_pServerList = pList;
	m_pHandle = NULL;
	StartSocket();
}

CMasterServer::~CMasterServer(void)
{
	// We don't want to have a Memory Leak!
	ClearServerList();
	StopSocket();
}

std::string CMasterServer::AddFilterTable(std::string strName, std::string strValue) {
	return std::string("\\") + strName + std::string("\\") + strValue;
}

void CMasterServer::ClearServerList( void ) {
	std::vector<TMasterServerList*>::iterator ServerIterator;
	for(ServerIterator = ServerList.begin(); ServerIterator != ServerList.end(); ServerIterator++) {
		TMasterServerList * pServer = ((TMasterServerList*)*ServerIterator);
		if(!pServer)
			continue;

		// Delete from Memory
		delete[] pServer->szAddress;
		delete pServer;
	}
	ServerList.clear();
	ServerList.resize(0, NULL);
}

void CMasterServer::SetConnectionInfo(SOCKADDR_IN &sckAddrIn, const char * cszAddress, unsigned int uPort) {
	// Resolve IP (If Host)
	hostent * host;
	host = gethostbyname(cszAddress);

	if(host) {
		// Remote Host information
		sckAddrIn.sin_family = AF_INET;
		sckAddrIn.sin_port = htons(uPort);
		sckAddrIn.sin_addr.s_addr = *((unsigned long *)host->h_addr);
	}
}

void CMasterServer::StartSocket( void ) {
	WSADATA wsData;
	WSAStartup( MAKEWORD(2, 2), &wsData );
}

void CMasterServer::StopSocket( void ) {
	WSACleanup();
}

void CMasterServer::GetServerIpList( void * pArgs ) {

	TThreadArguments * pArguments = (TThreadArguments*)pArgs;
	CMasterServer * pMaster = (CMasterServer*)pArguments->pClassPtr;
	TMasterQueryRules * pQueryRules = (TMasterQueryRules*)pArguments->pQueryRules;

	std::string strPacket;
	int iRemoteAddrSize = sizeof(SOCKADDR_IN);
	TMasterServerQueryReponse pMasterServerResponse;
	int iResult = 0;
	char szData[8192];
	char szIP[22];

	pMaster->sckMaster = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); 
	SOCKADDR_IN sckAddrMaster;

	SetConnectionInfo(sckAddrMaster, pQueryRules->strAddress.c_str(), pQueryRules->sPortNumber);

	// Construct the Packet
	strPacket.append(1, 0x31);						// (byte) Magic Key
	strPacket.append(1, pQueryRules->bRegion);		// (byte) Region Code

	strPacket += pQueryRules->strIPIterator;		// (string zero) 0.0.0.0:0 default
	strPacket.append(1, 0x00);

	strPacket += pQueryRules->strFilter;			// (string zero) Filter
	strPacket.append(1, 0x00);

	if( !(sendto (pMaster->sckMaster, strPacket.c_str(), strPacket.size(), 0, (SOCKADDR*)&sckAddrMaster, (int)sizeof(SOCKADDR_IN)) > 0) ) {
		goto clean_up;
	}

	// Loop for Data
	while(pMaster->m_bIsDownloadingList) {

		iResult = recvfrom( pMaster->sckMaster, szData , 8192, 0, (SOCKADDR*)&sckAddrMaster, &iRemoteAddrSize );
		if(!(iResult > 0)) {
			goto clean_up;
		}

		for(int i = 6; i < iResult; i += 6) {

			if(!pMaster->m_bIsDownloadingList)
				goto clean_up;

			memcpy(&pMasterServerResponse, szData + i, 6);

			sprintf(szIP, "%d.%d.%d.%d:%u", 
				pMasterServerResponse.bIpOctet1, 
				pMasterServerResponse.bIpOctet2, 
				pMasterServerResponse.bIpOctet3, 
				pMasterServerResponse.bIpOctet4, 
				ntohs(pMasterServerResponse.sPortNumber));

			//printf("IP: %s\n", szIP);

			if(!_stricmp(szIP,"0.0.0.0:0")) {
				// End of Server List
				//printf("End of List.\n");
				goto clean_up;

			} else {
				TMasterServerList * pServer = new TMasterServerList;

				// Address
				pServer->szAddress = new char[23];
				sprintf(pServer->szAddress, "%d.%d.%d.%d", 
					pMasterServerResponse.bIpOctet1, 
					pMasterServerResponse.bIpOctet2, 
					pMasterServerResponse.bIpOctet3, 
					pMasterServerResponse.bIpOctet4);
					pServer->sPortNumber = pMasterServerResponse.sPortNumber;
					pMaster->m_uServerCountListed++;
					pMaster->ServerList.push_back(pServer);
			}
			Sleep(1);
		}

		if(!pMaster->m_bIsDownloadingList)
			goto clean_up;

		// Construct the Packet
		//if (g_bLogging) Logger->Write("Construct the packet\n");
		strPacket = "";
		strPacket.append(1, 0x31);						// (byte) Magic Key
		strPacket.append(1, pQueryRules->bRegion);		// (byte) Region Code
		strPacket.append(szIP, 
			strlen(szIP) + 1);							// (string zero) last received IP
		strPacket += pQueryRules->strFilter;			// (string zero) Filter

		if( !sendto (pMaster->sckMaster, strPacket.c_str(), strPacket.size(), 0, (SOCKADDR*)&sckAddrMaster, (int)sizeof(SOCKADDR_IN)) ) {
			goto clean_up;
		}
	}

clean_up:

	if(pArguments)
		delete pArguments;
	if(pQueryRules)
		delete pQueryRules;

	pArguments = NULL;
	pQueryRules = NULL;

	pMaster->m_bIsDownloadingList = false;
	closesocket(pMaster->sckMaster);
	_endthread();
}

void CMasterServer::QueryServerList(TMasterQueryRules * pQueryRules) {
	std::string strPacket;

	TThreadArguments * pArguments = new TThreadArguments;
	pArguments->pClassPtr = this;
	pArguments->pQueryRules = pQueryRules;

	m_bIsDownloadingList = true;
	_beginthread(GetServerIpList, 0, (void*)pArguments );
}


TServerHandle2 * CMasterServer::GetFirstServer( void ) {
	TServerHandle2 * pServerHandle = new TServerHandle2;
	pServerHandle->ServerIterator = 0;
	if(m_bIsDownloadingList) {
		if( ((pServerHandle->ServerIterator + 1) >= ServerList.size()) ) {
			pServerHandle->bAllowRefresh = false;
			return pServerHandle;
		}
	} else {
		return false;
	}
	
	pServerHandle->bAllowRefresh = true;
	pServerHandle->szServerIp = ((TMasterServerList*)ServerList.at(pServerHandle->ServerIterator))->szAddress;
	pServerHandle->sServerPort = ((TMasterServerList*)ServerList.at(pServerHandle->ServerIterator))->sPortNumber;

	return pServerHandle;
}

bool CMasterServer::GetNextServer(TServerHandle2 * pServerHandle) {
	if(pServerHandle) {
		if( ( (pServerHandle->ServerIterator + 1) >= ServerList.size() ) ) {
			if(m_bIsDownloadingList) {
				pServerHandle->bAllowRefresh = false;
				return true;
			} else {
				return false;
			}
		}
		if( m_uActiveThreads > MAX_QUERYTHREADS ) {
			pServerHandle->bAllowRefresh = false;
			return true;
		}

		pServerHandle->ServerIterator++;
		pServerHandle->bAllowRefresh = true;
		pServerHandle->szServerIp = ((TMasterServerList*)ServerList.at(pServerHandle->ServerIterator))->szAddress;
		pServerHandle->sServerPort = ((TMasterServerList*)ServerList.at(pServerHandle->ServerIterator))->sPortNumber;
		return true;
	}
	return true;
}

void CMasterServer::GetCloseServer(TServerHandle2 * pServerHandle) {
	if(pServerHandle) {
		delete pServerHandle;
		pServerHandle = NULL;
	}
}

void CMasterServer::GetServerInfo(TServerHandle2 * pServerHandle/*, void * pCallbackFunction*/) {

	std::string strPacket;

	if(pServerHandle) {
		TQueryThread * pServerResponse = new TQueryThread;
		pServerResponse->pMasterServer = this;
		pServerResponse->dwStartTime = GetTickCount();
		//pServerResponse->pCallback = pCallbackFunction;
		pServerResponse->pHandle = pServerHandle;

		m_uActiveThreads++;
		_beginthread(ServerInfoResponse, 0, (void*)pServerResponse );
	}
}

char * CMasterServer::ReadString(char * szData, unsigned int * uRead) {
	char * szRet = new char[MAX_PATH];
	unsigned int i = 0;

	// Read till 0
	while(szData[i] != '\0') {
		szRet[i] = szData[i];
		i++;
		if(i > MAX_PATH-1) {
			*uRead = i + 1;
			return "Invalid string";
		}
		Sleep(10);
	}
	szRet[i] = '\0';
	*uRead = i + 1;

	return szRet;
}

void CMasterServer::ServerInfoResponse( void * pArgs) {
	
	// Game Server Info
	BYTE bVersion = 0;
	char * szGameIp = NULL;
	char * szName = NULL;
	char * szMap = NULL;
	char * szGameDirectory = NULL;
	char * szGameDescription = NULL;
	char * szGameVersion = NULL;
	char * szSpectatorName = NULL;
	char * szTags = NULL;

	short sAppId = 0;
	short sGamePort = 0;
	short sSpectatorPort = 0;

	BYTE bNumOfPlayers = 0;
	BYTE bMaxPlayers = 0;
	BYTE bGameVersion = 0;
	BYTE bDedicated = 0;
	BYTE bOS = 0;
	bool bPassword = false;
	bool bIsMod = false;
	bool bIsSecure = false;
	BYTE bNumOfBots = 0;
	BYTE bDataFlag = 0;

	// bIsMod == 1 ->
	char * szUrlInfo = NULL;
	char * szUrlDL = NULL;
	BYTE bNull = 0;
	long lModVersion = 0;
	long lModSize = 0;
	BYTE bSvOnly = 0;
	BYTE bClDll = 0;

	SOCKET sckQuery = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); ;
	SOCKADDR_IN sckAddrIn;
	TQueryThread * pArguments = (TQueryThread*)pArgs;
	CMasterServer * pMaster = (CMasterServer*)pArguments->pMasterServer;

	std::string strPacket;
	int iRemoteAddrSize = sizeof(SOCKADDR_IN);
	int iResult = 0;
	char szData[8192];
	struct timeval stTimeOut;
	fd_set stReadFDS;
	fd_set stWriteFDS;
	int iSelect = 0;

	// Time out after 3 seconds.
	FD_ZERO(&stReadFDS);
	FD_ZERO(&stWriteFDS);
	stTimeOut.tv_sec = 3;
	stTimeOut.tv_usec = 0;
	FD_SET(sckQuery, &stReadFDS);
	FD_SET(sckQuery, &stWriteFDS);

	// Set the Connection Info
	SetConnectionInfo(sckAddrIn, pArguments->pHandle->szServerIp, ntohs(pArguments->pHandle->sServerPort));
	
	unsigned int serverIP = ntohl(sckAddrIn.sin_addr.S_un.S_addr);
	unsigned short serverPort = ntohs(pArguments->pHandle->sServerPort);

	// Construct Packet
	strPacket.append(4, 0xFF);
	strPacket.append(1, 0x54);
	strPacket += "Source Engine Query";

	iSelect = select(sckQuery, NULL, &stWriteFDS, NULL, &stTimeOut);
	if(iSelect > 0) {
		if( !(sendto (sckQuery, strPacket.c_str(), strPacket.size(), 0, (SOCKADDR*)&sckAddrIn, (int)sizeof(SOCKADDR_IN)) > 0) ) {
			printf("Error: %u\n", WSAGetLastError());
			goto clean_up;
		}
	} else {
		goto clean_up;
	}

	// Parse Info
	iSelect = select(sckQuery, &stReadFDS, NULL, NULL, &stTimeOut);
	if(iSelect > 0) {

		iResult = recvfrom( sckQuery, szData , 8192, 0, (SOCKADDR*)&sckAddrIn, &iRemoteAddrSize );
		if(!(iResult > 0))
			goto clean_up;

		// Make sure this is -1 as requested
		int uMagicKey = 0;
		memcpy(&uMagicKey, szData, 4);
		if(uMagicKey != -1)
			goto clean_up;

		// The ping counts a lot so we set it before everything else.
		gameserveritem_t * pGameServer = new gameserveritem_t;
		pGameServer->m_nPing = (GetTickCount() - pArguments->dwStartTime);

		// We require the Pointer for better parsing.
		char * szInfo = (char*)&szData;
		unsigned int uRead = 0;

		szInfo += 4;
		if(szInfo[0] == 0x49) {
			// Source Engine Type
			szInfo++;

			// Version
			memcpy(&bVersion, szInfo, 1);
			szInfo++;

			// Name
			szName = ReadString(szInfo, &uRead);
			szInfo += uRead;

			// Map
			szMap = ReadString(szInfo, &uRead);
			szInfo += uRead;

			// Game Directory
			szGameDirectory = ReadString(szInfo, &uRead);
			szInfo += uRead;

			// Game Description
			szGameDescription = ReadString(szInfo, &uRead);
			szInfo += uRead;

			// Application Id
			memcpy(&sAppId, szInfo, 2);
			szInfo += 2;

			// Number of Players
			memcpy(&bNumOfPlayers, szInfo, 1);
			szInfo += 1;

			// Max Player
			memcpy(&bMaxPlayers, szInfo, 1);
			szInfo += 1;

			// Number of Bots
			memcpy(&bNumOfBots, szInfo, 1);
			szInfo += 1;

			// Dedicated Serer
			memcpy(&bDedicated, szInfo, 1);
			szInfo += 1;

			// OS
			memcpy(&bOS, szInfo, 1);
			szInfo += 1;

			// Password
			memcpy(&bPassword, szInfo, 1);
			szInfo += 1;

			// Secure
			memcpy(&bIsSecure, szInfo, 1);
			szInfo += 1;

			// Game Version
			szGameVersion = ReadString(szInfo, &uRead);
			szInfo += uRead;

			// Just call the new code when the server does send it.
			if((int)szInfo - (int)&szData > iResult) {

				// Data Flag
				memcpy(&bDataFlag, szInfo, 1);
				szInfo += 1;

				// Game Port
				if(bDataFlag & 0x80) {
					// Data Flag
					memcpy(&sGamePort, szInfo, 2);
					szInfo += 2;
				}

				// Spectator
				if(bDataFlag & 0x40) {
					memcpy(&sGamePort, szInfo, 2);
					szInfo += 2;
					szSpectatorName = ReadString(szInfo, &uRead);
					szInfo += uRead;
				}

				// Tags [Pretty new and not used yet]
				if(bDataFlag & 0x20) {
					szTags = ReadString(szInfo, &uRead);
					szInfo += uRead;
				}

			}
		} else if(szInfo[0] == 0x6D) {

			// Goldsource Type
			szInfo++;

			// Game IP
			szGameIp = ReadString(szInfo, &uRead);
			szInfo += uRead;

			// Server Name
			szName = ReadString(szInfo, &uRead);
			szInfo += uRead;

			// Map
			szMap = ReadString(szInfo, &uRead);
			szInfo += uRead;

			// Game Directory
			szGameDirectory = ReadString(szInfo, &uRead);
			szInfo += uRead;

			// Game Description
			szGameDescription = ReadString(szInfo, &uRead);
			szInfo += uRead;

			// Number of Players
			memcpy(&bNumOfPlayers, szInfo, 1);
			szInfo += 1;

			// Max Player
			memcpy(&bMaxPlayers, szInfo, 1);
			szInfo += 1;

			// Version
			memcpy(&bVersion, szInfo, 1);
			szInfo++;

			// Dedicated Serer
			memcpy(&bDedicated, szInfo, 1);
			szInfo += 1;

			// OS
			memcpy(&bOS, szInfo, 1);
			szInfo += 1;

			// Password
			memcpy(&bPassword, szInfo, 1);
			szInfo += 1;

			// Password
			memcpy(&bIsMod, szInfo, 1);
			szInfo += 1;

			// Includes Modification infos
			if(bIsMod == 1) {
				// Url Info
				szUrlInfo = ReadString(szInfo, &uRead);
				szInfo += uRead;

				// Url DL
				szUrlDL = ReadString(szInfo, &uRead);
				szInfo += uRead;

				// Null
				szInfo += 1;

				// Mod Version
				memcpy(&lModVersion, szInfo, 4);
				szInfo += 4;

				// Mod Size
				memcpy(&lModSize, szInfo, 4);
				szInfo += 4;

				// Server mod only
				memcpy(&bSvOnly, szInfo, 1);
				szInfo += 1;

				memcpy(&bClDll, szInfo, 1);
				szInfo += 1;
			}

			// Secure
			memcpy(&bIsSecure, szInfo, 1);
			szInfo += 1;

			// Bots
			memcpy(&bNumOfBots, szInfo, 1);
			szInfo += 1;

		}

		pGameServer->m_bSecure = bIsSecure;
		pGameServer->m_bPassword = bPassword;
		pGameServer->m_nAppID = sAppId;
		pGameServer->m_nPlayers = bNumOfPlayers;
		pGameServer->m_nBotPlayers = bNumOfBots;
		pGameServer->m_nMaxPlayers = bMaxPlayers;
		pGameServer->m_bHadSuccessfulResponse = 1;
		pGameServer->m_nServerVersion = bVersion;
		pGameServer->m_NetAdr.SetIP( serverIP );
		pGameServer->m_NetAdr.SetConnectionPort( serverPort );
		pGameServer->m_NetAdr.SetQueryPort( serverPort );

		if(szGameDescription) {
			strcpy(pGameServer->m_szGameDescription,szGameDescription);
		} else {
			goto clean_up;
		}

		if(szGameDirectory) {
			strcpy(pGameServer->m_szGameDir,szGameDirectory);
		} else {
			goto clean_up;
		}

		if(szTags) {
			strcpy(pGameServer->m_szGameTags, szTags);
		} else {
			//goto clean_up;
			// Dont require Tags
		}

		if(szMap) {
			strcpy(pGameServer->m_szMap,szMap);
		} else {
			goto clean_up;
		}

		if(szName) {
			pGameServer->SetName(szName);
		} else {
			goto clean_up;
		}

		
		//EnterCriticalSection(&pMaster->m_gCritical);
		//while(pMaster->m_bResponse) {
		//	Sleep(1);
		//}
		
		//cbStatus pCall = (cbStatus)pArguments->pCallback;

		//pMaster->m_bResponse = true;
		//pMaster->m_pServerList->AddNewServer(pGameServer);
		//pMaster->m_bResponse = false;
		//LeaveCriticalSection(&pMaster->m_gCritical);

		std::unique_lock<std::mutex> lock(pMaster->m_Critical);
		pMaster->ServerItems.AddToTail(pGameServer);
		lock.unlock();
	}

clean_up:
	delete pArguments;
	closesocket(sckQuery);
	std::unique_lock<std::mutex> lock(pMaster->m_Critical);
	pMaster->m_uActiveThreads--;
	lock.unlock();
	_endthread();

}

void CMasterServer::NextRefresh(void * pMasterClass) {

}

void CMasterServer::RunFrame() {

	if(!m_bIsRefresh)
		return;

	std::unique_lock<std::mutex> lock(m_Critical);
	if(ServerItems.Count() > 0) 
		{
			m_pServerList->AddNewServer( ServerItems[0] );
			ServerItems.Remove(0);
		}
	lock.unlock();

	if(!m_pHandle) {
		m_pHandle = GetFirstServer();
		if(m_pHandle) {
			if(m_pHandle->bAllowRefresh) {
				GetServerInfo(m_pHandle);
			}
		}
	} else {
		if(GetNextServer(m_pHandle)) {
			if(m_pHandle->bAllowRefresh) {
				GetServerInfo(m_pHandle);
			}
		} else {
			m_bIsRefresh = false;
			GetCloseServer(m_pHandle);
			m_pServerList->RefreshComplete();
			m_pHandle = NULL;
		}
	}

}

void CMasterServer::RefreshServerList(void * pArgs) {

	TRefreshThread * pArguments = (TRefreshThread*)pArgs;
	CMasterServer * pMaster = (CMasterServer*)pArguments->pMaster;

	std::unique_lock<std::mutex> lock(pMaster->m_Critical);
	pMaster->m_bIsRefresh = true;
	TServerHandle2 * pHandle = pMaster->GetFirstServer();
	lock.unlock();
	if(pHandle) {
		do {
			if(pHandle->bAllowRefresh) {
				pMaster->GetServerInfo(pHandle);
				Sleep(1);
			}
		} while( pMaster->GetNextServer(pHandle) == true && pMaster->m_bIsRefresh == true);
		pMaster->GetCloseServer(pHandle);
	}
	lock.lock();
	pMaster->m_bIsRefresh = false;
	lock.unlock();
	pMaster->m_pServerList->RefreshComplete();

	delete pArguments;
	_endthread();
}

bool CMasterServer::StartRefresh() {
	
	TRefreshThread * pRefreshThread = new TRefreshThread;
	if(!m_bIsRefresh) {
		m_bIsRefresh = true;
		pRefreshThread->pMaster = this;
		_beginthread(RefreshServerList, 0, (void*)pRefreshThread);
		return true;
	}
	
	//m_bIsRefresh = true;
	return false;
}

bool CMasterServer::StopRefresh() {
	m_bIsRefresh = false;
	return true;
}

bool CMasterServer::IsRefreshing() {
	return m_bIsRefresh;
}
#endif
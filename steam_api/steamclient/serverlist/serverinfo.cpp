#include <thread>
#include <cstdio>
#include "serverinfo.h"
#include "tier1/bitbuf.h"

//--------------------------------------------------------------------------
// $CServerInfo
// Purpose: Retrieves Information from Server.
//--------------------------------------------------------------------------
CServerInfo::CServerInfo(CServerManager * pServerManager, 
						 const char * cszIp, 
						 unsigned short sPort,
						 ISteamMatchmakingPingResponse * pResponse)
{

	m_pServerManager = pServerManager;
	m_pServerManager->m_uActiveThreads++;
	m_sckQuery = -1;

	TServerRefresh * pServer = new TServerRefresh;

	// Server Address.
	pServer->szAddress = (char*)cszIp;

	// Server Port.
	pServer->sPort = sPort;

	// Reference.
	pServer->pServerInfo = this;

	pServer->pResponse = pResponse;

	// Start the Thread.
	std::thread thread(CServerInfo::RefreshServer, pServer);
	thread.detach();
}

CServerInfo::~CServerInfo(void)
{
	if(m_sckQuery != -1)
		closesocket(m_sckQuery);
	m_pServerManager->m_uActiveThreads--;
}

byte CServerInfo::ReadByte(char ** szData)
{
	byte bytRet = 0;
	if(*szData)
	{
		memcpy(&bytRet, *szData, sizeof(byte));
		*szData += sizeof(byte);
	}
	return bytRet;
}

float CServerInfo::ReadFloat(char ** szData)
{
	float fRet = 0;
	if(*szData)
	{
		memcpy(&fRet, szData, sizeof(float));
		*szData += sizeof(float);
	}
	return fRet;
}

bool CServerInfo::ReadBool(char ** szData)
{
	return (ReadByte(szData) == 1 ? true : false);
}

char * CServerInfo::ReadString(char ** szData)
{
	char * szRet = new char[NET_UDP_RECVSIZE];
	unsigned int i = 0;
	char * szBuffer = *szData;

	if(*szData)
	{
		memset(szRet, 0, sizeof(szRet));
		while(szBuffer[i] != '\0')
		{
			if(i >= NET_UDP_RECVSIZE)
				break;
			szRet[i] = szBuffer[i];
			i++;
			//Sleep(1);
		}
		szRet[i] = '\0';
		*szData += i + 1;
	}
	
	return szRet;
}

int CServerInfo::ReadInt32(char ** szData)
{
	int iRet = 0;
	if(*szData)
	{
		memcpy(&iRet, *szData, sizeof(int));
		*szData += sizeof(int);
	}
	return iRet;
}

short CServerInfo::ReadShort(char ** szData)
{
	short sRet = 0;
	if(*szData)
	{
		memcpy(&sRet, *szData, sizeof(short));
		*szData += sizeof(short);
	}
	return sRet;
}

void CServerInfo::RefreshServer(void * pServer)
{

	// Reference Arguments
	TServerRefresh * pServerRefresh = (TServerRefresh*)pServer;
	if(!pServerRefresh)
	{
		delete pServerRefresh; // No Pointer, do not clean up!
		return;
	}

	// Reference Class
	CServerInfo * pServerInfo = (CServerInfo*)pServerRefresh->pServerInfo;
	if(!pServerInfo)
		return;
		
	// Get Server Info
	pServerInfo->GetServerInfo(pServerRefresh->szAddress,
		pServerRefresh->sPort,
		pServerRefresh->pResponse);

	delete pServerRefresh;
	delete pServerInfo;
}

void CServerInfo::GetServerInfo(const char * cszAddress, 
								unsigned short sPort, 
								ISteamMatchmakingPingResponse * pResponse)
{
	struct timeval	stTime;
	SOCKADDR_IN		sckAddress;
	fd_set			stReadFDS;
	int				iSelectStatus;
	int				iBytesReceived;
	char			szBuffer[NET_UDP_RECVSIZE];
	char			szPacket[25];
	char *			szData;
	int iRemoteAddrSize = sizeof(SOCKADDR_IN);
	byte			EDF;
	DWORD			dwStartTime;

	// Socket
	m_sckQuery = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(m_sckQuery == -1)
		return;

	netadr_t adr(cszAddress);
	adr.SetType(NA_IP);
	adr.SetPort(sPort);

	// Set Connection
	if (!CServerManager::SetConnectionInfo(sckAddress, adr))
	{
		Msg("Failed setting connection info for socket %i\n", m_sckQuery);
		return;
	}

	bf_write packet(szPacket, sizeof(szPacket));
	packet.WriteLong(-1);
	packet.WriteByte('T');
	packet.WriteString("Source Engine Query");

	//DevMsg("Send %s (sz=%i) to %s\n", szPacket, packet.GetNumBytesWritten(), cszAddress);

	// Send Request
	if (!sendto(m_sckQuery, szPacket, packet.GetNumBytesWritten(), 0, (sockaddr*)&sckAddress, sizeof(SOCKADDR_IN))) {
		Msg("Failed sending Server Info Query to %s\n", cszAddress);
		return;
	}

	dwStartTime = Plat_MSTime();

	// Timeout
	FD_ZERO(&stReadFDS);
	stTime.tv_sec = 3;
	stTime.tv_usec = 0;
	FD_SET(m_sckQuery, &stReadFDS);

	// Select
	iSelectStatus = select(m_sckQuery, &stReadFDS, NULL, NULL, &stTime);
	if(!(iSelectStatus > 0)) 
	{
		if(pResponse)
			pResponse->ServerFailedToRespond();
		return;
	}

	iBytesReceived = recvfrom( m_sckQuery, 
		szBuffer, 
		NET_UDP_RECVSIZE, 
		0, 
		(SOCKADDR*)&sckAddress, 
		&iRemoteAddrSize );

	if(!(iBytesReceived > 0))
	{
		if(pResponse)
			pResponse->ServerFailedToRespond();
		return;
	}

	szData = (char*)&szBuffer;

	// Magic Key
	int iMagicKey = ReadInt32(&szData);
	if(iMagicKey != -1)
	{
		if(pResponse)
			pResponse->ServerFailedToRespond();
		return;
	}

	// Looks like a valid Packet
	gameserveritem_t * pGameServer = new gameserveritem_t;

	pGameServer->m_nPing = Plat_MSTime() - dwStartTime;
	// Goldsource | Source
	byte bytGameType = ReadByte(&szData);
	if(bytGameType == 0x49) 
	{
		// Source
		pGameServer->m_nServerVersion = (int)ReadByte(&szData);
		pGameServer->SetName(ReadString(&szData));
		strcpy(pGameServer->m_szMap, ReadString(&szData));
		strcpy(pGameServer->m_szGameDir, ReadString(&szData));
		strcpy(pGameServer->m_szGameDescription, ReadString(&szData));
		pGameServer->m_nAppID = (int)ReadShort(&szData);
		pGameServer->m_nPlayers = (int)ReadByte(&szData);
		pGameServer->m_nMaxPlayers = (int)ReadByte(&szData);
		pGameServer->m_nBotPlayers = (int)ReadByte(&szData);
		ReadByte(&szData);		// 'Dedicated', not used.
		ReadByte(&szData);		// 'OS', not used.
		pGameServer->m_bPassword = ReadBool(&szData);
		pGameServer->m_bSecure = ReadBool(&szData);
		ReadString(&szData);	// 'Game Version', not used.
		pGameServer->m_bHadSuccessfulResponse = true;

		EDF = ReadByte(&szData);
		if(EDF & 0x80)
			ReadShort(&szData);	
		if(EDF & 0x40)
			ReadShort(&szData) && ReadString(&szData);
		if(EDF & 0x20)
			strcpy(pGameServer->m_szGameTags, ReadString(&szData));
	}
	else if(bytGameType == 0x6D)
	{
		// Gold Source
		ReadString(&szData);	// 'Game IP', <- They are pretty stupid.
		pGameServer->SetName(ReadString(&szData));
		strcpy(pGameServer->m_szMap, ReadString(&szData));
		strcpy(pGameServer->m_szGameDir, ReadString(&szData));
		strcpy(pGameServer->m_szGameDescription, ReadString(&szData));
		pGameServer->m_nPlayers = (int)ReadByte(&szData);
		pGameServer->m_nMaxPlayers = (int)ReadByte(&szData);
		pGameServer->m_nServerVersion = (int)ReadByte(&szData);
		ReadByte(&szData);		// 'Dedicated', not used.
		ReadByte(&szData);		// 'OS', not used.
		pGameServer->m_bPassword = ReadBool(&szData);
		if(ReadBool(&szData)) 
		{
			ReadString(&szData);	// URLInfo
			ReadString(&szData);	// URLDL
			ReadByte(&szData);		// Null
			ReadInt32(&szData);		// Mod Version
			ReadInt32(&szData);		// Mod Size
			ReadByte(&szData);		// SvOnly
			ReadByte(&szData);		// CIDLL
		}
		pGameServer->m_bSecure = ReadBool(&szData);
		pGameServer->m_nBotPlayers = ReadByte(&szData);
		pGameServer->m_bHadSuccessfulResponse = true;
	}
	pGameServer->m_NetAdr.Init( ntohl(sckAddress.sin_addr.S_un.S_addr) , sPort, sPort);
	//m_pServerManager->m_vecRefreshed.AddToHead(pGameServer);
	if(pResponse)
	{
		pResponse->ServerResponded(*pGameServer);
		delete pGameServer;
	} 
	else 
	{
		if(m_pServerManager->m_bIsRefresh)
			m_pServerManager->m_vecRefreshed.AddToTail(pGameServer);
	}
	return;
}

//--------------------------------------------------------------------------
// $CServerInfoPlayers
// Purpose: Retrieves Players and statistics.
//--------------------------------------------------------------------------
CServerInfoPlayers::CServerInfoPlayers(const char * cszIP, 
									   unsigned short sPort, 
									   ISteamMatchmakingPlayersResponse * pResponse)
{
	m_sckQuery = -1;

	TPlayerQuery * pQuery = new TPlayerQuery;

	pQuery->pPlayerInfo = this;
	pQuery->pResponse = pResponse;
	pQuery->sPort = sPort;
	pQuery->szAddress = (char*)cszIP;

	std::thread thread(CServerInfoPlayers::GetPlayerInfo, (void*)pQuery);
	thread.detach();
}

CServerInfoPlayers::~CServerInfoPlayers()
{
	if(m_sckQuery != -1)
		closesocket(m_sckQuery);
}

void CServerInfoPlayers::GetPlayerInfo( void * pQuery )
{
	TPlayerQuery * pPlayerQuery = (TPlayerQuery*)pQuery;
	if(!pPlayerQuery)
		return;

	CServerInfoPlayers * pServerPlayers = (CServerInfoPlayers*)pPlayerQuery->pPlayerInfo;
	if(!pServerPlayers)
		return;

	// Get Players
	pServerPlayers->GetPlayers(pPlayerQuery->szAddress,
		pPlayerQuery->sPort,
		pPlayerQuery->pResponse);

	// Cleanup
	delete pPlayerQuery;
	delete pServerPlayers;
}

unsigned int CServerInfoPlayers::GetChallenge(SOCKADDR_IN sckAddress)
{
	struct timeval	stTime;
	fd_set			stReadFDS;
	int				iSelectStatus;
	int				iBytesReceived;
	char			szBuffer[NET_UDP_RECVSIZE];
	int iRemoteAddrSize = sizeof(SOCKADDR_IN);

	// Socket
	if(m_sckQuery == -1)
		return 0;

	// Send Request
	if(!sendto(m_sckQuery, "\xFF\xFF\xFF\xFF\x57", 
		5, 
		0, 
		(sockaddr*)&sckAddress, sizeof(SOCKADDR_IN))
		)
		return 0;

	// Timeout
	FD_ZERO(&stReadFDS);
	stTime.tv_sec = 3;
	stTime.tv_usec = 0;
	FD_SET(m_sckQuery, &stReadFDS);

	// Select
	iSelectStatus = select(m_sckQuery, &stReadFDS, NULL, NULL, &stTime);
	if(!(iSelectStatus > 0)) 
		return 0;

	// Get Challenge
	iBytesReceived = recvfrom(m_sckQuery, szBuffer, NET_UDP_RECVSIZE, 0,
		(sockaddr*)&sckAddress, &iRemoteAddrSize);
	if(!(iBytesReceived > 0))
		return 0;

	// Challenge
	char * szData = (char*)&szBuffer;
	if(CServerInfo::ReadInt32(&szData) != -1)
		return 0;

	if(CServerInfo::ReadByte(&szData) == 0x41)
	{
		return CServerInfo::ReadInt32(&szData);
	}
	return 0;
}

void CServerInfoPlayers::GetPlayers(const char * cszAddress, 
									unsigned short sPort, 
									ISteamMatchmakingPlayersResponse * pResponse)
{
	struct timeval	stTime;
	SOCKADDR_IN		sckAddress;
	fd_set			stReadFDS;
	int				iSelectStatus;
	int				iBytesReceived;
	char			szBuffer[NET_UDP_RECVSIZE];
	char			szPacket[10];
	unsigned int	uChallenge = 0;
	int iRemoteAddrSize = sizeof(SOCKADDR_IN);
	byte			bytNumOfPlayers;
	byte bytIndex = 0;
	char * szPlayer = NULL;
	long lKills = 0;
	float fTime = 0;

	// Socket.
	m_sckQuery = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(m_sckQuery == -1)
		return;

	netadr_t adr(cszAddress);
	adr.SetType(NA_IP);
	adr.SetPort(sPort);

	// Set Connection.
	if (!CServerManager::SetConnectionInfo(sckAddress, adr))
		return;

	// Get Challenge.
	uChallenge = GetChallenge(sckAddress);
	if(uChallenge == 0)
		return;

	// Construct Packet
	bf_write packet(szPacket, sizeof(szPacket));
	packet.WriteLong(-1);
	packet.WriteByte('U');
	packet.WriteLong(uChallenge);

	//DevMsg("Send %s (sz=%i) to %s\n", szPacket, packet.GetNumBytesWritten(), cszAddress);

	// Send Request
	if(!sendto(m_sckQuery, szPacket, packet.GetNumBytesWritten(), 0, (sockaddr*)&sckAddress, sizeof(SOCKADDR_IN)))
	{
		Msg("Failed sending Server Player Query to %s\n", cszAddress);
		pResponse->PlayersFailedToRespond();
		return;
	}

	// Timeout
	FD_ZERO(&stReadFDS);
	stTime.tv_sec = 3;
	stTime.tv_usec = 0;
	FD_SET(m_sckQuery, &stReadFDS);

	// Select
	iSelectStatus = select(m_sckQuery, &stReadFDS, NULL, NULL, &stTime);
	if(!(iSelectStatus > 0)) 
	{
		pResponse->PlayersFailedToRespond();
		return;
	}

	// Receive Data
	iBytesReceived = recvfrom(m_sckQuery, szBuffer, NET_UDP_RECVSIZE, 0,
		(sockaddr*)&sckAddress, &iRemoteAddrSize);
	if(!(iBytesReceived > 0))
	{
		pResponse->PlayersFailedToRespond();
		return;
	}

	char * szData = (char*)&szBuffer;
	if(CServerInfo::ReadInt32(&szData) != -1)
	{
		pResponse->PlayersFailedToRespond();
		return;
	}

	if(CServerInfo::ReadByte(&szData) == 0x44)
	{
		bytNumOfPlayers = CServerInfo::ReadByte(&szData);
		for(unsigned int i = 0; i != (unsigned int)bytNumOfPlayers; i++)
		{
			bytIndex = CServerInfo::ReadByte(&szData);
			szPlayer = CServerInfo::ReadString(&szData);
			lKills = (long)CServerInfo::ReadInt32(&szData);
			fTime = CServerInfo::ReadFloat(&szData);

			pResponse->AddPlayerToList(szPlayer, lKills, fTime);

			delete szPlayer;
		}
		pResponse->PlayersRefreshComplete();
	}
	else
	{
		pResponse->PlayersFailedToRespond();
	}
	return;
}
#include "ServerLan.h"

CServerLan::CServerLan(CServerManager * pManager, unsigned short sPort)
{
	m_pServerManager = pManager;
	m_pServerManager->m_uActiveThreads++;
	m_sckQuery = INVALID_SOCKET;

	TServerRefreshLan * pLan = new TServerRefreshLan;
	pLan->pServerInfo = this;
	pLan->sPort = sPort;
	_beginthread(RefreshServer, 0, (void*)pLan);
}

CServerLan::~CServerLan(void)
{
	if(m_sckQuery != INVALID_SOCKET)
		closesocket(m_sckQuery);
	m_pServerManager->m_uActiveThreads--;
}

byte CServerLan::ReadByte(char ** szData)
{
	byte bytRet = 0;
	if(*szData)
	{
		memcpy(&bytRet, *szData, sizeof(byte));
		*szData += sizeof(byte);
	}
	return bytRet;
}

float CServerLan::ReadFloat(char ** szData)
{
	float fRet = 0;
	if(*szData)
	{
		memcpy(&fRet, szData, sizeof(float));
		*szData += sizeof(float);
	}
	return fRet;
}

bool CServerLan::ReadBool(char ** szData)
{
	return (ReadByte(szData) == 1 ? true : false);
}

char * CServerLan::ReadString(char ** szData)
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

int CServerLan::ReadInt32(char ** szData)
{
	int iRet = 0;
	if(*szData)
	{
		memcpy(&iRet, *szData, sizeof(int));
		*szData += sizeof(int);
	}
	return iRet;
}

short CServerLan::ReadShort(char ** szData)
{
	short sRet = 0;
	if(*szData)
	{
		memcpy(&sRet, *szData, sizeof(short));
		*szData += sizeof(short);
	}
	return sRet;
}

void CServerLan::RefreshServer(void * pServer)
{
	// Reference Arguments
	TServerRefreshLan * pServerRefresh = (TServerRefreshLan*)pServer;
	if(!pServerRefresh)
	{
		delete pServerRefresh; // No Pointer, do not clean up!
		_endthread();
	}

	// Reference Class
	CServerLan * pServerLan = (CServerLan*)pServerRefresh->pServerInfo;
	if(!pServerLan)
		_endthread();

	__try {

		// Get Server Info
		pServerLan->GetLanIP(pServerRefresh->sPort);
		goto end_thread;

	} __except( exceptionhandler( GetExceptionCode(), GetExceptionInformation() ) ) {

		// Error inside function.
		printf("Error (%s): %u", __FUNCTION__, GetLastError());
		goto end_thread;
	}

end_thread:
	delete pServerRefresh;
	delete pServerLan;
	_endthread();
}

void CServerLan::GetLanIP(unsigned short sPort)
{
	struct timeval	stTime;
	sockaddr_in		sckAddress;
	fd_set			stReadFDS;
	int				iSelectStatus;
	int				iBytesReceived;
	char			szPacket[25];
	char			szBuffer[NET_UDP_RECVSIZE];
	int				iRemoteAddrSize = sizeof(sockaddr_in);
	int				opt = 1;
	unsigned long	_true = 1;
	DWORD			dwStartTime;
	byte			bytEDF;

	// Socket
	m_sckQuery = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(m_sckQuery == INVALID_SOCKET)
		return;

	// Broadcast
	setsockopt(m_sckQuery, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt));

	// Set it to non-blocking
	ioctlsocket ( m_sckQuery, FIONBIO, &_true );

	// Listen on port 0
	sckAddress.sin_family = AF_INET;
	sckAddress.sin_port = 0;
	sckAddress.sin_addr.s_addr = INADDR_ANY;

	// Requires bind to 0.0.0.0
	// Possibility for Bill's gayness
	if(bind(m_sckQuery, (sockaddr*)&sckAddress, sizeof(sockaddr_in)) == -1)
		return;

	sckAddress.sin_family = AF_INET;
	sckAddress.sin_port = sPort;
	sckAddress.sin_addr.s_addr = inet_addr("255.255.255.255");

	char * szRequest = (char*)&szPacket;
	memset(szRequest, 0xFF, 4);
	memcpy(szRequest + 4, "TSource Engine Query\x00", strlen("TSource Engine Query") + 1);

	// Send Request
	if(!sendto(m_sckQuery, szRequest, 25, 0, (sockaddr*)&sckAddress, sizeof(sockaddr_in)))
		return;
	
	dwStartTime = GetTickCount();

	// Timeout
	FD_ZERO(&stReadFDS);
	stTime.tv_sec = 2;
	stTime.tv_usec = 0;
	FD_SET(m_sckQuery, &stReadFDS);

	m_pServerManager->m_bIsDownloading = true;
	while(true)
	{

		// Select
		iSelectStatus = select(m_sckQuery, &stReadFDS, NULL, NULL, &stTime);
		if(!(iSelectStatus > 0)) 
		{
			break;
		}

		iBytesReceived = recvfrom( m_sckQuery, 
			szBuffer, 
			NET_UDP_RECVSIZE, 
			0, 
			(sockaddr*)&sckAddress, 
			&iRemoteAddrSize );

		if(!(iBytesReceived > 0))
		{
			break;
		}

		char * szData = (char*)&szBuffer;

		int iMagicKey = ReadInt32(&szData);
		if(iMagicKey != -1)
		{
			continue;
		}

		// Looks like a valid Packet
		gameserveritem_t * pGameServer = new gameserveritem_t;
		
		pGameServer->m_nPing = GetTickCount() - dwStartTime;
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

			bytEDF = ReadByte(&szData);
			if(bytEDF & 0x80)
				ReadShort(&szData);	
			if(bytEDF & 0x40)
				ReadShort(&szData) && ReadString(&szData);
			if(bytEDF & 0x20)
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
		pGameServer->m_NetAdr.Init( ntohl(sckAddress.sin_addr.S_un.S_addr) , ntohs(sPort), ntohs(sPort));
		//m_pServerManager->m_vecRefreshed.AddToHead(pGameServer);

		std::lock_guard<std::mutex> lock(m_pServerManager->m_Critical);
		if(m_pServerManager->m_bIsRefresh)
			m_pServerManager->m_vecRefreshed.AddToTail(pGameServer);
	}
	m_pServerManager->m_bIsDownloading = false;
}
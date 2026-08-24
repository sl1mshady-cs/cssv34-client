// a bunch of STL here
#include "serverinfo.h"

// threads
std::vector<ServerRefreshThread> g_vecServerRefreshThreads;

/* 
* Worker function that needs to be runned every frame
*/
class ServerRefreshThreadsWorker
{
public:
	std::thread t;
	std::atomic<bool> running;

	ServerRefreshThreadsWorker() {
		running.store(true);

		t = std::thread(&ServerRefreshThreadsWorker::Worker, this);
	}

	~ServerRefreshThreadsWorker() {
		running.store(false);

		if (t.joinable()) t.join();
	}

	void Worker()
	{
		while (running.load())
		{
			for (auto it = g_vecServerRefreshThreads.begin();
				it != g_vecServerRefreshThreads.end();)
			{
				if (it->is_alive)
				{
					if (it->is_alive->load())
					{
						++it;
					}
					else
					{
						it = g_vecServerRefreshThreads.erase(it);
					}
				}
			}
		}
	}
};

static ServerRefreshThreadsWorker s_threadsworker;

//////////////////////////////////////////
/* --------- Public Functions --------- */
//////////////////////////////////////////

// Find a thread
ServerRefreshThread* ServerRefreshThreads_Find(std::thread::id threadID)
{
	for (auto& it : g_vecServerRefreshThreads)
	{
		if (it.is_alive->load())
		{
			if (it.t.get_id() == threadID)
				return &it;
		}
	}

	return nullptr;
}

// Start the Thread.
void ServerRefreshThreads_Start(server_refresh_t serverRefresh)
{
	ServerRefreshThread thread(serverRefresh);
	g_vecServerRefreshThreads.emplace_back(std::move(thread));
}

// Terminate the thread (bad!)
void ServerRefreshThreads_Stop(std::thread::id threadID)
{
	ServerRefreshThread* pThread = ServerRefreshThreads_Find(threadID);
	if (!pThread)
		return;

	// platform specific
#ifdef _WIN32
	//TerminateThread(pThread->t.native_handle(), 0);
#else
	pthread_cancel(pThread->t.native_handle());
#endif

	Warning("[srt] stopping thread under id %u\n", threadID);
	
	/// freeing memory now ///
	if (pThread->is_alive->load()) {
		pThread->is_alive->store(false);
	}
}

//--------------------------------------------------------------------------
// Purpose: Retrieves Information from Server.
//--------------------------------------------------------------------------
CServerInfo::CServerInfo(CServerManager * pServerManager, netadr_t& adr,
						 ISteamMatchmakingPingResponse * pResponse)
{
	m_pServerManager = pServerManager;
	m_pServerManager->m_uActiveThreads++;
	m_pQuery = new CSocket();

	if (!pServerManager && !pResponse)
		return;

	server_refresh_t serverRefresh{};

	// Reference.
	serverRefresh.pThis = this;

	serverRefresh.pResponse = pResponse;

	// Request type
	serverRefresh.nRequest = 1;

	// Server Address and port.
	serverRefresh.adr = adr;

	// Start the thread
	ServerRefreshThreads_Start(serverRefresh);
}

//--------------------------------------------------------------------------
// Purpose: Retrieves Information about players from Server.
//--------------------------------------------------------------------------
CServerInfo::CServerInfo(netadr_t& adr,
	ISteamMatchmakingPlayersResponse* pResponse)
{
	m_pServerManager = 0;
	m_pQuery = new CSocket();

	if (!pResponse)
		return;

	server_refresh_t serverRefresh{};

	// Reference.
	serverRefresh.pThis = this;

	serverRefresh.pResponse = pResponse;

	// Request type
	serverRefresh.nRequest = 2;

	// Server Address and port.
	serverRefresh.adr = adr;

	// Start the thread
	ServerRefreshThreads_Start(serverRefresh);
}

//--------------------------------------------------------------------------
// Purpose: Retrieves rules from Server.
//--------------------------------------------------------------------------
CServerInfo::CServerInfo(netadr_t& adr,
	ISteamMatchmakingRulesResponse* pResponse)
{
	m_pServerManager = 0;
	m_pQuery = new CSocket();

	if (!pResponse)
		return;

	server_refresh_t serverRefresh{};

	// Reference.
	serverRefresh.pThis = this;

	serverRefresh.pResponse = pResponse;

	// Request type
	serverRefresh.nRequest = 3;

	// Server Address and port.
	serverRefresh.adr = adr;

	// Start the thread
	ServerRefreshThreads_Start(serverRefresh);
}

//--------------------------------------------------------------------------
// Purpose: Destructor
//--------------------------------------------------------------------------
CServerInfo::~CServerInfo(void)
{
	delete m_pQuery;

	if (m_pServerManager)
		m_pServerManager->m_uActiveThreads--;
}

/*
* Get server info (now allows broadcasting)
*/
void CServerInfo::GetServerInfo(netadr_t& adr, ISteamMatchmakingPingResponse* pResponse)
{
	m_bIsRefreshing = true;

	// are we broadcasting
	bool bBroadcast = (adr.GetType() == NA_BROADCAST);

	// Socket
	m_pQuery->Open(0, bBroadcast);
	if (!m_pQuery->IsValid())
		return;

	char szPacket[25];
	bf_write packet(szPacket, sizeof(szPacket));
	packet.WriteLong(0xFFFFFFFF);
	packet.WriteByte('T');
	packet.WriteString("Source Engine Query");

	// Send Request
	if (bBroadcast)
	{
		if (m_pQuery->Broadcast(adr.GetPort(), packet) == -1) {
			Msg("Failed sending server info query to %s\n", adr.ToString());
			return;
		}
	}
	else
	{
		if (m_pQuery->Send(adr, packet) == -1) {
			Msg("Failed broadcasting server info query to port %u\n", adr.GetPort());
			return;
		}
	}
	uint32 dwStartTime = Plat_MSTime();

	fd_set stReadFDS; // read fds
	timeval	stTime{};

	// Timeout
	FD_ZERO(&stReadFDS);
	stTime.tv_sec = 3;
	stTime.tv_usec = 0;
	FD_SET(m_pQuery->GetSocket(), &stReadFDS);

	if (m_pServerManager)
		m_pServerManager->m_bIsDownloading = true;

	while (true)
	{
		// Select
		int iSelectStatus = select(m_pQuery->GetSocket(), &stReadFDS, NULL, NULL, &stTime);
		if (!(iSelectStatus > 0))
		{
			if (pResponse)
				pResponse->ServerFailedToRespond();
			return;
		}

		// read out received message
		char recvBuffer[MAX_RECEIVEABLE_PACKET];
		bf_read read;
		netadr_t from;

		int iBytesReceived = m_pQuery->Receive(recvBuffer, MAX_RECEIVEABLE_PACKET, from);
		read.StartReading(recvBuffer, iBytesReceived);

		if (!(iBytesReceived > 0))
		{
			if (pResponse)
				pResponse->ServerFailedToRespond();

			return;
		}

		// Connectionless header
		int nConnectionless = read.ReadLong();

		if (nConnectionless != -1)
		{
			if (pResponse)
				pResponse->ServerFailedToRespond();

			return;
		}

		// Looks like a valid Packet
		gameserveritem_t* pGameServer = new gameserveritem_t();

		pGameServer->m_NetAdr.Init(from.GetIPHostByteOrder(), from.GetPort(), from.GetPort());
		pGameServer->m_nPing = Plat_MSTime() - dwStartTime;
		pGameServer->m_bHadSuccessfulResponse = false;

		// Goldsource | Source
		byte gameType = read.ReadByte();
		byte EDF; // extra data flag

		if (gameType == 0x49)
		{
			// Source
			pGameServer->m_nServerVersion = read.ReadByte();;

			char szName[64];
			read.ReadString(szName, sizeof(szName));
			pGameServer->SetName(szName);

			read.ReadString(pGameServer->m_szMap, sizeof(pGameServer->m_szMap));
			read.ReadString(pGameServer->m_szGameDir, sizeof(pGameServer->m_szGameDir));
			read.ReadString(pGameServer->m_szGameDescription, sizeof(pGameServer->m_szGameDescription));

			pGameServer->m_nAppID = read.ReadShort();
			pGameServer->m_nPlayers = read.ReadByte();
			pGameServer->m_nMaxPlayers = read.ReadByte();
			pGameServer->m_nBotPlayers = read.ReadByte();

			read.ReadByte();		// 'Dedicated', not used.
			read.ReadByte();		// 'OS', not used.

			pGameServer->m_bPassword = read.ReadByte();
			pGameServer->m_bSecure = read.ReadByte();

			// 'Game Version', not used.
			char szVersion[64];
			read.ReadString(szVersion, sizeof(szVersion));

			pGameServer->m_bHadSuccessfulResponse = true;

			// Read out EDF (if present)
			EDF = read.ReadByte();

			if (EDF)
			{
				if (EDF & 0x80) // ???
					read.ReadShort();
				if (EDF & 0x40) { // ???
					read.ReadShort();
					char unused[64];
					read.ReadString(unused, sizeof(unused));
				}
				if (EDF & 0x20) // tags
					read.ReadString(pGameServer->m_szGameTags, sizeof(pGameServer->m_szGameTags));
			}
		}

		if (pResponse)
		{
			pResponse->ServerResponded(*pGameServer);
			delete pGameServer;
		}
		else
		{
			if (bBroadcast) {
				std::lock_guard<std::mutex> lock(m_pServerManager->m_Critical);

				if (m_pServerManager->m_bIsRefresh)
					m_pServerManager->m_vecRefreshed.AddToTail(pGameServer);
			}
			else
			{
				if (m_pServerManager->m_bIsRefresh)
					m_pServerManager->m_vecRefreshed.AddToTail(pGameServer);
			}
		}

		if (!bBroadcast)
			break;
	}

	if (m_pServerManager)
		m_pServerManager->m_bIsDownloading = false;

	m_bIsRefreshing = false;
}

/*
* Get server info
*/
void CServerInfo::GetPlayers(netadr_t& adr, ISteamMatchmakingPlayersResponse* pResponse)
{
	m_bIsRefreshing = true;

	// Socket
	m_pQuery->Open();
	if (!m_pQuery->IsValid())
		return;

	// first, request challenge
	static int32 iChallengeNr = -1;

	char szPacket[25];
	bf_write packet(szPacket, sizeof(szPacket));
	packet.WriteLong(0xFFFFFFFF);
	packet.WriteByte('U');
	packet.WriteLong(iChallengeNr);

	// Send Request
	if (m_pQuery->Send(adr, packet) == -1) {
		Msg("Failed sending player info query to %s\n", adr.ToString());
		return;
	}

	uint32 dwStartTime = Plat_MSTime();

	fd_set stReadFDS; // read fds
	timeval	stTime{};

	// Timeout
	FD_ZERO(&stReadFDS);
	stTime.tv_sec = 3;
	stTime.tv_usec = 0;
	FD_SET(m_pQuery->GetSocket(), &stReadFDS);

	// Select
	int iSelectStatus = select(m_pQuery->GetSocket(), &stReadFDS, NULL, NULL, &stTime);
	if(!(iSelectStatus > 0)) 
	{
		if(pResponse)
			pResponse->PlayersFailedToRespond();

		return;
	}

	// read out received message
	char recvBuffer[MAX_RECEIVEABLE_PACKET];
	bf_read read;
	netadr_t from;

	int iBytesReceived = m_pQuery->Receive(recvBuffer, MAX_RECEIVEABLE_PACKET, from);
	read.StartReading(recvBuffer, iBytesReceived);

	if (!(iBytesReceived > 0))
	{
		if (pResponse)
			pResponse->PlayersFailedToRespond();

		return;
	}

	// update bytes and bits
	read.m_nDataBytes = iBytesReceived;
	read.m_nDataBits = read.m_nDataBytes << 3;

	// Connectionless header
	int nConnectionless = read.ReadLong();

	if (nConnectionless != -1)
	{
		if (pResponse)
			pResponse->PlayersFailedToRespond();

		return;
	}

	// Looks like a valid Packet
	byte packetType = read.ReadByte();

	if (packetType == 'A')
	{
		iChallengeNr == read.ReadLong();
		return GetPlayers(adr, pResponse);
	}
	else if (packetType == 'D')
	{
		int numPlayers = read.ReadByte();
		int id, score;
		float time;
		char name[64];

		for (int i = 0; i != numPlayers; i++)
		{
			memset(name, 0, sizeof(name));
			id = read.ReadByte();
			read.ReadString(name, sizeof(name));
			score = read.ReadLong();
			time = read.ReadFloat();

			pResponse->AddPlayerToList(name, score, time);
		}

		pResponse->PlayersRefreshComplete();
	}
	else
	{ 
		// unknown packet
		pResponse->PlayersFailedToRespond();
	}

	m_bIsRefreshing = false;
}


/*
* Get server info
*/
void CServerInfo::GetRules(netadr_t& adr, ISteamMatchmakingRulesResponse* pResponse)
{
	m_bIsRefreshing = true;

	// Socket
	m_pQuery->Open();
	if (!m_pQuery->IsValid())
		return;

	// first, request challenge
	static int32 iChallengeNr = -1;

	char szPacket[25];
	bf_write packet(szPacket, sizeof(szPacket));
	packet.WriteLong(0xFFFFFFFF);
	packet.WriteByte('V');
	packet.WriteLong(iChallengeNr);

	// Send Request
	if (m_pQuery->Send(adr, packet) == -1)
	{
		Msg("Failed sending server rules query to %s\n", adr.ToString());
		return;
	}

	uint32 dwStartTime = Plat_MSTime();

	fd_set stReadFDS; // read fds
	timeval	stTime{};

	// Timeout
	FD_ZERO(&stReadFDS);
	stTime.tv_sec = 3;
	stTime.tv_usec = 0;
	FD_SET(m_pQuery->GetSocket(), &stReadFDS);

	// Select
	int iSelectStatus = select(m_pQuery->GetSocket(), &stReadFDS, NULL, NULL, &stTime);
	if (!(iSelectStatus > 0))
	{
		if (pResponse)
			pResponse->RulesFailedToRespond();

		return;
	}

	// read out received message
	char recvBuffer[MAX_RECEIVEABLE_PACKET];
	bf_read read;
	netadr_t from;

	int iBytesReceived = m_pQuery->Receive(recvBuffer, MAX_RECEIVEABLE_PACKET, from);
	read.StartReading(recvBuffer, iBytesReceived);

	if (!(iBytesReceived > 0))
	{
		if (pResponse)
			pResponse->RulesFailedToRespond();

		return;
	}

	// update bytes and bits
	read.m_nDataBytes = iBytesReceived;
	read.m_nDataBits = read.m_nDataBytes << 3;

	// Connectionless header
	int nConnectionless = read.ReadLong();

	if (nConnectionless != -1)
	{
		if (pResponse)
			pResponse->RulesFailedToRespond();

		return;
	}

	// Looks like a valid Packet
	byte packetType = read.ReadByte();

	if (packetType == 'A')
	{
		iChallengeNr == read.ReadLong();
		return GetRules(adr, pResponse);
	}
	else if (packetType == 'E')
	{
		int numRules = read.ReadShort();
		char name[64]; // names are not bigger than that (i think)
		char value[256];

		for (int i = 0; i != numRules; i++)
		{
			memset(name, 0, sizeof(name));
			memset(value, 0, sizeof(value));

			read.ReadString(name, sizeof(name));
			read.ReadString(value, sizeof(value));

			pResponse->RulesResponded(name, value);
		}

		pResponse->RulesRefreshComplete();
	}
	else
	{
		// unknown packet
		pResponse->RulesFailedToRespond();
	}

	m_bIsRefreshing = false;
}

// a bunch of STL here
#include "serverinfo.h"

// threads
std::mutex g_ServerRefreshThreadsMutex;
std::vector<std::shared_ptr<ServerRefreshThread>> g_vecServerRefreshThreads;

/* 
* Worker
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
			ServerRefreshThreads_Cleanup();

			std::this_thread::sleep_for(
				std::chrono::milliseconds(10));
		}
	}
};

static ServerRefreshThreadsWorker s_threadsworker;

//////////////////////////////////////////
/* --------- Public Functions --------- */
//////////////////////////////////////////

// cleanup threads
void ServerRefreshThreads_Cleanup()
{
	std::lock_guard<std::mutex> lock(
		g_ServerRefreshThreadsMutex);

	for (auto it = g_vecServerRefreshThreads.begin();
		it != g_vecServerRefreshThreads.end();)
	{
		const auto& thread = *it;

		if (!thread)
		{
			it = g_vecServerRefreshThreads.erase(it);
			continue;
		}

		if (!thread->is_alive->load())
		{
			if (thread->t.joinable())
				thread->t.join();

			it = g_vecServerRefreshThreads.erase(it);
			continue;
		}

		++it;
	}
}

// Start the Thread.
void ServerRefreshThreads_Start(server_refresh_t serverRefresh)
{
	std::lock_guard<std::mutex> lock(
		g_ServerRefreshThreadsMutex);

	g_vecServerRefreshThreads.emplace_back(
		std::make_shared<ServerRefreshThread>(
			std::move(serverRefresh)));
}

// Terminate the thread (bad!)
void ServerRefreshThreads_Stop(std::thread::id threadID)
{
	std::shared_ptr<ServerRefreshThread> thread;

	{
		std::lock_guard<std::mutex> lock(
			g_ServerRefreshThreadsMutex);

		for (auto& candidate :
			g_vecServerRefreshThreads)
		{
			if (candidate &&
				candidate->t.get_id() == threadID)
			{
				thread = candidate;
				break;
			}
		}
	}

	if (!thread)
		return;

	thread->stopRequested->store(
		true,
		std::memory_order_release);

	if (thread->t.joinable())
		thread->t.join();
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
void CServerInfo::GetServerInfo(netadr_t& adr, ISteamMatchmakingPingResponse* pResponse, 
	std::shared_ptr<std::atomic<bool>> stopRequested)
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

	if (stopRequested->load())
		return;

	uint32 dwStartTime = Plat_MSTime();

	fd_set stReadFDS; // read fds
	timeval	stTime{};

	if (m_pServerManager)
		m_pServerManager->m_bIsDownloading = true;

	while (!stopRequested->load())
	{
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
				pResponse->ServerFailedToRespond();
			return;
		}

		if (stopRequested->load())
			break;

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
void CServerInfo::GetPlayers(netadr_t& adr, ISteamMatchmakingPlayersResponse* pResponse,
	std::shared_ptr<std::atomic<bool>> stopRequested)
{
	m_bIsRefreshing = true;

	// Socket
	m_pQuery->Open();
	if (!m_pQuery->IsValid())
		return;

	// challenge, (-1) is default, we need to request it first
	int32 challenge = -1;

	while (!stopRequested->load())
	{
		char szPacket[25];
		bf_write packet(szPacket, sizeof(szPacket));
		packet.WriteLong(0xFFFFFFFF);
		packet.WriteByte('U');
		packet.WriteLong(challenge);

		// Send Request
		if (m_pQuery->Send(adr, packet) == -1) {
			Msg("Failed sending player info query to %s\n", adr.ToString());
			break;
		}

		if (stopRequested->load())
			break;

		uint32 dwStartTime = Plat_MSTime();

		fd_set stReadFDS; // read fds
		timeval	stTime{};

		// Timeout
		FD_ZERO(&stReadFDS);
		stTime.tv_sec = 1;
		stTime.tv_usec = 0;
		FD_SET(m_pQuery->GetSocket(), &stReadFDS);

		// Select
		int iSelectStatus = select(m_pQuery->GetSocket(), &stReadFDS, NULL, NULL, &stTime);

		if (stopRequested->load())
			break;

		if (!(iSelectStatus > 0))
		{
			if (pResponse)
				pResponse->PlayersFailedToRespond();

			break;
		}

		// read out received message
		char recvBuffer[MAX_RECEIVEABLE_PACKET];
		bf_read read;
		netadr_t from;

		int iBytesReceived = m_pQuery->Receive(recvBuffer, MAX_RECEIVEABLE_PACKET, from);
		read.StartReading(recvBuffer, iBytesReceived);

		if (stopRequested->load())
			break;

		if (!(iBytesReceived > 0))
		{
			if (pResponse)
				pResponse->PlayersFailedToRespond();

			break;
		}

		// update bytes and bits
		read.m_nDataBytes = iBytesReceived;
		read.m_nDataBits = read.m_nDataBytes << 3;

		// Connectionless header
		int nConnectionless = read.ReadLong();

		if (stopRequested->load())
			break;

		if (nConnectionless != -1)
		{
			if (pResponse)
				pResponse->PlayersFailedToRespond();

			break;
		}

		// Looks like a valid Packet
		byte packetType = read.ReadByte();

		if (packetType == 'A')
		{
			if (challenge != -1) // server sent invalid packet
			{
				if (pResponse)
					pResponse->PlayersFailedToRespond();

				break;
			}

			challenge = read.ReadLong();
			continue;
		}
		
		if (packetType == 'D')
		{
			if (stopRequested->load())
				break;

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

				if (pResponse)
					pResponse->AddPlayerToList(name, score, time);
			}

			if (pResponse)
				pResponse->PlayersRefreshComplete();

			break;
		}

		break;
	}

	m_bIsRefreshing = false;
}


/*
* Get server info
*/
void CServerInfo::GetRules(netadr_t& adr, ISteamMatchmakingRulesResponse* pResponse,
	std::shared_ptr<std::atomic<bool>> stopRequested)
{
	m_bIsRefreshing = true;

	// Socket
	m_pQuery->Open();
	if (!m_pQuery->IsValid())
		return;

	// challenge, (-1) is default, we need to request it first
	int32 challenge = -1;

	while (!stopRequested->load())
	{
		char szPacket[25];
		bf_write packet(szPacket, sizeof(szPacket));
		packet.WriteLong(0xFFFFFFFF);
		packet.WriteByte('V');
		packet.WriteLong(challenge);

		// Send Request
		if (m_pQuery->Send(adr, packet) == -1)
		{
			Msg("Failed sending server rules query to %s\n", adr.ToString());
			break;
		}

		if (stopRequested->load())
			break;

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

		if (stopRequested->load())
			break;

		if (!(iSelectStatus > 0))
		{
			if (pResponse)
				pResponse->RulesFailedToRespond();

			break;
		}

		// read out received message
		char recvBuffer[MAX_RECEIVEABLE_PACKET];
		bf_read read;
		netadr_t from;

		int iBytesReceived = m_pQuery->Receive(recvBuffer, MAX_RECEIVEABLE_PACKET, from);
		read.StartReading(recvBuffer, iBytesReceived);

		if (stopRequested->load())
			break;

		if (!(iBytesReceived > 0))
		{
			if (pResponse)
				pResponse->RulesFailedToRespond();

			break;
		}

		// update bytes and bits
		read.m_nDataBytes = iBytesReceived;
		read.m_nDataBits = read.m_nDataBytes << 3;

		// Connectionless header
		int nConnectionless = read.ReadLong();

		if (stopRequested->load())
			break;

		if (nConnectionless != -1)
		{
			if (pResponse)
				pResponse->RulesFailedToRespond();

			break;
		}

		// Looks like a valid Packet
		byte packetType = read.ReadByte();

		if (packetType == 'A')
		{
			if (challenge != -1) // server sent invalid packet
			{
				if (pResponse)
					pResponse->RulesFailedToRespond();

				break;
			}

			challenge = read.ReadLong();
			continue;
		}
		else if (packetType == 'E')
		{
			if (stopRequested->load())
				break;

			int numRules = read.ReadShort();
			char name[64]; // names are not bigger than that (i think)
			char value[256];

			for (int i = 0; i != numRules; i++)
			{
				memset(name, 0, sizeof(name));
				memset(value, 0, sizeof(value));

				read.ReadString(name, sizeof(name));
				read.ReadString(value, sizeof(value));

				if (pResponse)
					pResponse->RulesResponded(name, value);
			}

			if (pResponse)
				pResponse->RulesRefreshComplete();
		}

		break;
	}

	m_bIsRefreshing = false;
}

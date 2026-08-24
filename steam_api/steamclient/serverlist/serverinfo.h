#ifndef _SERVERINFO_H
#define _SERVERINFO_H

#ifdef _WIN32
#pragma once
#endif

#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include "servermanager.h"

// forward declarations
class CServerInfo;

/*
* Internal server refresh structure
*/
struct server_refresh_t
{
	CServerInfo*	pThis;		// thisptr
	void*			pResponse;	// target response
	uint8			nRequest;	// request type, 1=info, 2=players, 3=rules
	netadr_t		adr;		// server address
};

////////////////////////////////////

/*
* Requests info about a single server
*/
class CServerInfo
{
	friend void ServerRefreshThreads_Stop(std::thread::id threadID);
public:
	// constructor for server manager (if pResponse == 0, uses pServerManager)
	CServerInfo(CServerManager* pServerManager, netadr_t& adr, ISteamMatchmakingPingResponse* pResponse = 0);

	// standalone constructors
	CServerInfo(netadr_t& adr, ISteamMatchmakingPlayersResponse* pResponse);
	CServerInfo(netadr_t& adr, ISteamMatchmakingRulesResponse* pResponse);
	~CServerInfo(void);

	uintp GetID() { return m_unID; }

	void GetServerInfo(netadr_t& adr, ISteamMatchmakingPingResponse* pResponse);
	void GetPlayers(netadr_t& adr, ISteamMatchmakingPlayersResponse* pResponse);
	void GetRules(netadr_t& adr, ISteamMatchmakingRulesResponse* pResponse);

	bool IsRefreshing() { return m_bIsRefreshing; }

private:
	// TYPE: WIN32 - unsigned int, POSIX - pthread_t(unsigned long int/struct __pthread *)
	// uintp is an integer that can accomodate a pointer:
	//  - this is useful since 64-bit Linux follows the LP64 data model, where long, unsigned long, and void* are all mapped to 64 bits
	//	- it doesn't interfere with windows compatibility because msvc uses always uses 32-bit unsigned int under thread::id
	uintp m_unID;

	bool m_bIsRefreshing; // i used this somewhere, now its unused lol
	CServerManager* m_pServerManager;
	CSocket* m_pQuery;
};


//////////////////////////////////

/*
* Trackable server refresh thread
*/
class ServerRefreshThread
{
public:
    std::thread t;
    std::shared_ptr<std::atomic<bool>> is_alive;

    ServerRefreshThread(server_refresh_t refresh)
        : is_alive(std::make_shared<std::atomic<bool>>(true))
    {
        t = std::thread(
            [alive = is_alive, refresh]() mutable
            {
                CServerInfo* pServerInfo = refresh.pThis;

                if (!pServerInfo)
                {
                    alive->store(false);
                    return;
                }

                switch (refresh.nRequest)
                {
                case 1:
                    pServerInfo->GetServerInfo(
                        refresh.adr,
                        static_cast<ISteamMatchmakingPingResponse*>(
                            refresh.pResponse));
                    break;

                case 2:
                    pServerInfo->GetPlayers(
                        refresh.adr,
                        static_cast<ISteamMatchmakingPlayersResponse*>(
                            refresh.pResponse));
                    break;

                case 3:
                    pServerInfo->GetRules(
                        refresh.adr,
                        static_cast<ISteamMatchmakingRulesResponse*>(
                            refresh.pResponse));
                    break;
                }

                delete pServerInfo;

                alive->store(false);
            });
    }

    ServerRefreshThread(ServerRefreshThread&& other) noexcept
        : t(std::move(other.t)),
        is_alive(std::move(other.is_alive))
    {}

    ~ServerRefreshThread()
    {
        if (t.joinable())
            t.join();
    }

    ServerRefreshThread& operator=(ServerRefreshThread&& other) noexcept
    {
        if (this != &other)
        {
            if (t.joinable())
                t.join();

            t = std::move(other.t);
            is_alive = std::move(other.is_alive);
        }

        return *this;
    }

    ServerRefreshThread(const ServerRefreshThread&) = delete;
    ServerRefreshThread& operator=(const ServerRefreshThread&) = delete;
};

//////////////////////////////////

// Find a thread
ServerRefreshThread* ServerRefreshThreads_Find(std::thread::id threadID);

// Start the thread
void ServerRefreshThreads_Start(server_refresh_t serverRefresh);

// Terminate thread under given id (bad!)
void ServerRefreshThreads_Stop(std::thread::id threadID);

// Terminate thread under given uintptr id (bad!)
inline void ServerRefreshThreads_Stop(uintp threadID) {
	ServerRefreshThreads_Stop(*(std::thread::id*)&threadID);
}

#endif // _SERVERINFO_H
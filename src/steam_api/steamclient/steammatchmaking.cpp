#include "SteamMatchmaking.h"

extern CSteamMatchMakingServers* g_pSteamMatchMakingServers;

CSteamMatchMaking::CSteamMatchMaking()
{
	// voided
}

CSteamMatchMaking::~CSteamMatchMaking()
{
	// voided
}

int CSteamMatchMaking::GetFavoriteGameCount()
{
	ListRequest req;
	req.type = eFavoritesServer;

	return g_pSteamMatchMakingServers->GetServerCount(&req);
}

bool CSteamMatchMaking::GetFavoriteGame(int iGame, AppId_t* pnAppID, uint32* pnIP, uint16* pnConnPort, uint16* pnQueryPort, uint32* punFlags, uint32* pRTime32LastPlayedOnServer)
{
	if (g_pSteamMatchMakingServers)
	{
		return g_pSteamMatchMakingServers->GetFavoriteGame(iGame, pnAppID, pnIP, pnConnPort, pnQueryPort, punFlags, pRTime32LastPlayedOnServer);
	}
	return 1;
}
int CSteamMatchMaking::AddFavoriteGame(AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags, uint32 rTime32LastPlayedOnServer)
{
	if (g_pSteamMatchMakingServers)
	{
		return g_pSteamMatchMakingServers->AddToFavorites(nAppID, nIP, nConnPort, rTime32LastPlayedOnServer);
	}
	return 0;
}
bool CSteamMatchMaking::RemoveFavoriteGame(AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags)
{
	if (g_pSteamMatchMakingServers)
	{
		return g_pSteamMatchMakingServers->RemoveFromFavorites(nAppID, nIP, nConnPort);
	}
	return 0;
}

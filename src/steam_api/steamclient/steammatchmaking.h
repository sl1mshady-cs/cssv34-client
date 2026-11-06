#pragma once

#include "SteamMatchMakingServers.h"

//-----------------------------------------------------------------------------
// Purpose: Functions for match making services for clients to get to favorites
//			and to operate on game lobbies.
//-----------------------------------------------------------------------------
class CSteamMatchMaking : public ISteamMatchmaking
{
public:
	CSteamMatchMaking();
	~CSteamMatchMaking();
	virtual int GetFavoriteGameCount();
	virtual bool GetFavoriteGame(int iGame, AppId_t* pnAppID, uint32* pnIP, uint16* pnConnPort, uint16* pnQueryPort, uint32* punFlags, uint32* pRTime32LastPlayedOnServer);
	virtual int AddFavoriteGame(AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags, uint32 rTime32LastPlayedOnServer);
	virtual bool RemoveFavoriteGame(AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags);
	virtual SteamAPICall_t RequestLobbyList() { return -1; }
	virtual void AddRequestLobbyListStringFilter(const char* pchKeyToMatch, const char* pchValueToMatch, ELobbyComparison eComparisonType) {}
	virtual void AddRequestLobbyListNumericalFilter(const char* pchKeyToMatch, int nValueToMatch, ELobbyComparison eComparisonType) {}
	virtual void AddRequestLobbyListNearValueFilter(const char* pchKeyToMatch, int nValueToBeCloseTo) {}
	virtual void AddRequestLobbyListFilterSlotsAvailable(int nSlotsAvailable) {}
	virtual void AddRequestLobbyListDistanceFilter(ELobbyDistanceFilter eLobbyDistanceFilter) {}
	virtual void AddRequestLobbyListResultCountFilter(int cMaxResults) {}
	virtual void AddRequestLobbyListCompatibleMembersFilter(CSteamID steamIDLobby) {}
	virtual CSteamID GetLobbyByIndex(int iLobby) { return CSteamID(); }
	virtual SteamAPICall_t CreateLobby(ELobbyType eLobbyType, int cMaxMembers) { return -1; }
	virtual SteamAPICall_t JoinLobby(CSteamID steamIDLobby) { return -1; }
	virtual void LeaveLobby(CSteamID steamIDLobby) {}
	virtual bool InviteUserToLobby(CSteamID steamIDLobby, CSteamID steamIDInvitee) { return false; }
	virtual int GetNumLobbyMembers(CSteamID steamIDLobby) { return 0; }
	virtual CSteamID GetLobbyMemberByIndex(CSteamID steamIDLobby, int iMember) { return CSteamID(); }
	virtual const char* GetLobbyData(CSteamID steamIDLobby, const char* pchKey) { return ""; }
	virtual bool SetLobbyData(CSteamID steamIDLobby, const char* pchKey, const char* pchValue) { return false; }
	virtual int GetLobbyDataCount(CSteamID steamIDLobby) { return 0; }
	virtual bool GetLobbyDataByIndex(CSteamID steamIDLobby, int iLobbyData, char* pchKey, int cchKeyBufferSize, char* pchValue, int cchValueBufferSize) { return false; }
	virtual bool DeleteLobbyData(CSteamID steamIDLobby, const char* pchKey) { return false; }
	virtual const char* GetLobbyMemberData(CSteamID steamIDLobby, CSteamID steamIDUser, const char* pchKey) { return ""; }
	virtual void SetLobbyMemberData(CSteamID steamIDLobby, const char* pchKey, const char* pchValue) {}
	virtual bool SendLobbyChatMsg(CSteamID steamIDLobby, const void* pvMsgBody, int cubMsgBody) { return false; }
	virtual int GetLobbyChatEntry(CSteamID steamIDLobby, int iChatID, OUT_STRUCT() CSteamID* pSteamIDUser, void* pvData, int cubData, EChatEntryType* peChatEntryType) { return 0; }
	virtual bool RequestLobbyData(CSteamID steamIDLobby) { return false; }
	virtual void SetLobbyGameServer(CSteamID steamIDLobby, uint32 unGameServerIP, uint16 unGameServerPort, CSteamID steamIDGameServer) {}
	virtual bool GetLobbyGameServer(CSteamID steamIDLobby, uint32* punGameServerIP, uint16* punGameServerPort, OUT_STRUCT() CSteamID* psteamIDGameServer) { return false; }
	virtual bool SetLobbyMemberLimit(CSteamID steamIDLobby, int cMaxMembers) { return false; }
	virtual int GetLobbyMemberLimit(CSteamID steamIDLobby) { return -1; }
	virtual bool SetLobbyType(CSteamID steamIDLobby, ELobbyType eLobbyType) { return false; }
	virtual bool SetLobbyJoinable(CSteamID steamIDLobby, bool bLobbyJoinable) { return false; }
	virtual CSteamID GetLobbyOwner(CSteamID steamIDLobby) { return CSteamID(); }
	virtual bool SetLobbyOwner(CSteamID steamIDLobby, CSteamID steamIDNewOwner) { return false; }
	virtual bool SetLinkedLobby(CSteamID steamIDLobby, CSteamID steamIDLobbyDependent) { return false; }
};

extern CSteamMatchMaking* g_pSteamMatchMaking;
#ifdef _WIN32
#include <winlite.h>
#endif
#include "steamfriends.h"
#include "logging.h"

extern CLoggingFile* Logger;

//-----------------------------------------------------------------------------
// Purpose: Retrieves a string value from the registry
//-----------------------------------------------------------------------------
#if defined(_WIN32)
void Sys_GetRegKeyValueUnderRoot( HKEY rootKey, const char *pszSubKey, const char *pszElement, OUT_Z_CAP(nReturnLength) char *pszReturnString, int nReturnLength, const char *pszDefaultValue )
{
	LONG lResult;           // Registry function result code
	HKEY hKey;              // Handle of opened/created key
	char szBuff[128];       // Temp. buffer
	ULONG dwDisposition;    // Type of key opening event
	DWORD dwType;           // Type of key
	DWORD dwSize;           // Size of element data

	// Copying a string to itself is both unnecessary and illegal.
	// Address sanitizer prohibits this so we have to fix this in order
	// to continue testing with it.
	if ( pszReturnString != pszDefaultValue )
	{
		// Assume the worst
		Q_strncpy(pszReturnString, pszDefaultValue, nReturnLength );
	}

	// Create it if it doesn't exist.  (Create opens the key otherwise)
	lResult = RegCreateKeyEx(
		rootKey,	// handle of open key 
		pszSubKey,			// address of name of subkey to open 
		0ul,					// DWORD ulOptions,	  // reserved 
		"String",			// Type of value
		REG_OPTION_NON_VOLATILE, // Store permanently in reg.
		KEY_ALL_ACCESS,		// REGSAM samDesired, // security access mask 
		NULL,
		&hKey,				// Key we are creating
		&dwDisposition);    // Type of creation
	
	if (lResult != ERROR_SUCCESS)  // Failure
		return;

	// First time, just set to Valve default
	if (dwDisposition == REG_CREATED_NEW_KEY)
	{
		// Just Set the Values according to the defaults
		lResult = RegSetValueEx( hKey, pszElement, 0, REG_SZ, (CONST BYTE *)pszDefaultValue, Q_strlen(pszDefaultValue) + 1 ); 
	}
	else
	{
		// We opened the existing key. Now go ahead and find out how big the key is.
		dwSize = nReturnLength;
		lResult = RegQueryValueEx( hKey, pszElement, 0, &dwType, (unsigned char *)szBuff, &dwSize );

		// Success?
		if (lResult == ERROR_SUCCESS)
		{
			// Only copy strings, and only copy as much data as requested.
			if (dwType == REG_SZ)
			{
				Q_strncpy(pszReturnString, szBuff, nReturnLength);
				pszReturnString[nReturnLength - 1] = '\0';
			}
		}
		else
		// Didn't find it, so write out new value
		{
			// Just Set the Values according to the defaults
			lResult = RegSetValueEx( hKey, pszElement, 0, REG_SZ, (CONST BYTE *)pszDefaultValue, Q_strlen(pszDefaultValue) + 1 ); 
		}
	};

	// Always close this key before exiting.
	RegCloseKey(hKey);

}
#endif

void Sys_GetRegKeyValue( const char *pszSubKey, const char *pszElement, OUT_Z_CAP(nReturnLength) char *pszReturnString, int nReturnLength, const char *pszDefaultValue )
{
#if defined(_WIN32)
	Sys_GetRegKeyValueUnderRoot( HKEY_CURRENT_USER, pszSubKey, pszElement, pszReturnString, nReturnLength, pszDefaultValue );
#else
	//hushed Assert( !"Impl me" );
	// Copying a string to itself is both unnecessary and illegal.
	if ( pszReturnString != pszDefaultValue )
	{
		Q_strncpy( pszReturnString, pszDefaultValue, nReturnLength );
	}
#endif
}

/////////
CSteamFriends::CSteamFriends() 
{

}

/////////
CSteamFriends::~CSteamFriends() 
{

}

// returns the local players name - guaranteed to not be NULL.
const char* CSteamFriends::GetPersonaName() 
{
	static char szPersonaName[32];
	Sys_GetRegKeyValue("Software\\Valve\\Steam", "LastGameNameUsed", szPersonaName, sizeof(szPersonaName), g_chName);

	return szPersonaName;
}

// Sets the player name, stores it on the server and publishes the changes to all friends who are online.
SteamAPICall_t CSteamFriends::SetPersonaName(const char* pchPersonaName) {
	return generateSteamAPICallID();
}

// gets the status of the current user
EPersonaState CSteamFriends::GetPersonaState() { return k_EPersonaStateOnline; }

// friend iteration
int CSteamFriends::GetFriendCount(int iFriendFlags) { return 1; }

// returns the steamID of a user
// iFriend is a index of range [0, GetFriendCount())
// iFriendsFlags must be the same value as used in GetFriendCount()
// the returned CSteamID can then be used by all the functions below to access details about the user
CSteamID CSteamFriends::GetFriendByIndex(int iFriend, int iFriendFlags) 
{ return generateSteamIDUser(); }

// returns a relationship to a user
EFriendRelationship CSteamFriends::GetFriendRelationship(CSteamID steamIDFriend) {
	return k_EFriendRelationshipNone;
}

// returns the current status of the specified user
// this will only be known by the local user if steamIDFriend is in their friends list{} on the same game server{} in a chat room or lobby{} or in a small group with the local user
EPersonaState CSteamFriends::GetFriendPersonaState(CSteamID steamIDFriend) {
	return k_EPersonaStateOffline;
}

// returns the name another user - guaranteed to not be NULL.
const char* CSteamFriends::GetFriendPersonaName(CSteamID steamIDFriend) 
{
	return g_chName;
}

// returns true if the friend is actually in a game, and fills in pFriendGameInfo with an extra details 
bool CSteamFriends::GetFriendGamePlayed(CSteamID steamIDFriend, OUT_STRUCT() FriendGameInfo_t* pFriendGameInfo) 
{
	memset(pFriendGameInfo, 0, sizeof(*pFriendGameInfo));
	return false;
}
// accesses old friends names - returns an empty string when their are no more items in the history
const char* CSteamFriends::GetFriendPersonaNameHistory(CSteamID steamIDFriend, int iPersonaName) 
{
	return 0;
}
// friends steam level
int CSteamFriends::GetFriendSteamLevel(CSteamID steamIDFriend) 
{
	int8 level;
	randombytes((uint8*)&level, sizeof(level));
	return level;
}

// Returns nickname the current user has set for the specified player. Returns NULL if the no nickname has been set for that player.
const char* CSteamFriends::GetPlayerNickname(CSteamID steamIDPlayer) 
{
	return 0;
}

// friend grouping (tag) apis
// returns the number of friends groups
int CSteamFriends::GetFriendsGroupCount() 
{
	return 0;
}
// returns the friends group ID for the given index (invalid indices return k_FriendsGroupID_Invalid)
FriendsGroupID_t CSteamFriends::GetFriendsGroupIDByIndex(int iFG) 
{
	return k_FriendsGroupID_Invalid;
}
// returns the name for the given friends group (NULL in the case of invalid friends group IDs)
const char* CSteamFriends::GetFriendsGroupName(FriendsGroupID_t friendsGroupID) 
{
	return 0;
}
// returns the number of members in a given friends group
int CSteamFriends::GetFriendsGroupMembersCount(FriendsGroupID_t friendsGroupID)
{
	return 0;
}
// gets up to nMembersCount members of the given friends group, if fewer exist than requested those positions' SteamIDs will be invalid
void CSteamFriends::GetFriendsGroupMembersList(FriendsGroupID_t friendsGroupID, OUT_ARRAY_CALL(nMembersCount, GetFriendsGroupMembersCount, friendsGroupID) CSteamID* pOutSteamIDMembers, int nMembersCount) {}

// returns true if the specified user meets any of the criteria specified in iFriendFlags
bool CSteamFriends::HasFriend(CSteamID steamIDFriend, int iFriendFlags) {
	return false;
}

// clan (group) iteration and access functions
int CSteamFriends::GetClanCount() { return 0; }
CSteamID CSteamFriends::GetClanByIndex(int iClan) { return generateSteamIDClan(); }
const char* CSteamFriends::GetClanName(CSteamID steamIDClan) { return ""; }
const char* CSteamFriends::GetClanTag(CSteamID steamIDClan) { return ""; }
// returns the most recent information we have about what's happening in a clan
bool CSteamFriends::GetClanActivityCounts(CSteamID steamIDClan, int* pnOnline, int* pnInGame, int* pnChatting) 
{
	return false;
}
// for clans a user is a member of, they will have reasonably up-to-date information, but for others you'll have to download the info to have the latest
SteamAPICall_t CSteamFriends::DownloadClanActivityCounts(ARRAY_COUNT(cClansToRequest) CSteamID* psteamIDClans, int cClansToRequest) 
{
	return generateSteamAPICallID();
}

// iterators for getting users in a chat room, lobby, game server or clan
int CSteamFriends::GetFriendCountFromSource(CSteamID steamIDSource) 
{
	return 0;
}
CSteamID CSteamFriends::GetFriendFromSourceByIndex(CSteamID steamIDSource, int iFriend) 
{
	return generateSteamIDUser();
}

// returns true if the local user can see that steamIDUser is a member or in steamIDSource
bool CSteamFriends::IsUserInSource(CSteamID steamIDUser, CSteamID steamIDSource) 
{
	return false;
}

// User is in a game pressing the talk button (will suppress the microphone for all voice comms from the Steam friends UI)
void CSteamFriends::SetInGameVoiceSpeaking(CSteamID steamIDUser, bool bSpeaking) {}

// activates the game overlay, with an optional dialog to open 
// valid options are "Friends", "Community", "Players", "Settings", "OfficialGameGroup", "Stats", "Achievements"
void CSteamFriends::ActivateGameOverlay(const char* pchDialog) {}

// activates game overlay to a specific place
void CSteamFriends::ActivateGameOverlayToUser(const char* pchDialog, CSteamID steamID) {}

// activates game overlay web browser directly to the specified URL
// full address with protocol type is required, e.g. http://www.steamgames.com/
void CSteamFriends::ActivateGameOverlayToWebPage(const char* pchURL) {}

// activates game overlay to store page for app
void CSteamFriends::ActivateGameOverlayToStore(AppId_t nAppID, EOverlayToStoreFlag eFlag) {}

// Mark a target user as 'played with'. This is a client-side only feature that requires that the calling user is 
// in game 
void CSteamFriends::SetPlayedWith(CSteamID steamIDUserPlayedWith) {}

// activates game overlay to open the invite dialog. Invitations will be sent for the provided lobby.
void CSteamFriends::ActivateGameOverlayInviteDialog(CSteamID steamIDLobby) {}

// gets the small (32x32) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
int CSteamFriends::GetSmallFriendAvatar(CSteamID steamIDFriend) { return 0; }

// gets the medium (64x64) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
int CSteamFriends::GetMediumFriendAvatar(CSteamID steamIDFriend) { return 0; }

// gets the large (184x184) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
// returns -1 if this image has yet to be loaded, in this case wait for a AvatarImageLoaded_t callback and then call this again
int CSteamFriends::GetLargeFriendAvatar(CSteamID steamIDFriend) { return 0; }

// requests information about a user - persona name & avatar
bool CSteamFriends::RequestUserInformation(CSteamID steamIDUser, bool bRequireNameOnly) { return false; }

// requests information about a clan officer list
SteamAPICall_t CSteamFriends::RequestClanOfficerList(CSteamID steamIDClan)
{
	return generateSteamAPICallID();
}

// iteration of clan officers - can only be done when a RequestClanOfficerList() call has completed

// returns the steamID of the clan owner
CSteamID CSteamFriends::GetClanOwner(CSteamID steamIDClan) {
	return generateSteamIDUser();
}
// returns the number of officers in a clan (including the owner)
int CSteamFriends::GetClanOfficerCount(CSteamID steamIDClan) {
	return 0;
}
// returns the steamID of a clan officer, by index, of range [0,GetClanOfficerCount)
CSteamID CSteamFriends::GetClanOfficerByIndex(CSteamID steamIDClan, int iOfficer) {
	return generateSteamIDAnonUser();
}
// if current user is chat restricted, he can't send or receive any text/voice chat messages.
uint32 CSteamFriends::GetUserRestrictions() {
	return 0;
}

// Rich Presence data is automatically shared between friends who are in the same game
bool CSteamFriends::SetRichPresence(const char* pchKey, const char* pchValue) { return false; }
void CSteamFriends::ClearRichPresence() {}
const char* CSteamFriends::GetFriendRichPresence(CSteamID steamIDFriend, const char* pchKey) { return 0; }
int CSteamFriends::GetFriendRichPresenceKeyCount(CSteamID steamIDFriend) { return 0; }
const char* CSteamFriends::GetFriendRichPresenceKeyByIndex(CSteamID steamIDFriend, int iKey) { return 0; }
// Requests rich presence for a specific user.
void CSteamFriends::RequestFriendRichPresence(CSteamID steamIDFriend) {}

// rich invite support
bool CSteamFriends::InviteUserToGame(CSteamID steamIDFriend, const char* pchConnectString) { return false; }

// recently-played-with friends iteration
int CSteamFriends::GetCoplayFriendCount() { return 0; }
CSteamID CSteamFriends::GetCoplayFriend(int iCoplayFriend) { return generateSteamIDUser(); }
int CSteamFriends::GetFriendCoplayTime(CSteamID steamIDFriend) { return 0; }
AppId_t CSteamFriends::GetFriendCoplayGame(CSteamID steamIDFriend) {
	AppId_t appid;
	randombytes((uint8*)&appid, sizeof(appid));
	return appid;
}

// chat interface for games
SteamAPICall_t CSteamFriends::JoinClanChatRoom(CSteamID steamIDClan) { return generateSteamAPICallID(); }
bool CSteamFriends::LeaveClanChatRoom(CSteamID steamIDClan) { return true; }
int CSteamFriends::GetClanChatMemberCount(CSteamID steamIDClan) { return 0; }
CSteamID CSteamFriends::GetChatMemberByIndex(CSteamID steamIDClan, int iUser) { return generateSteamIDUser(); }
bool CSteamFriends::SendClanChatMessage(CSteamID steamIDClanChat, const char* pchText) { return false; }
int CSteamFriends::GetClanChatMessage(CSteamID steamIDClanChat, int iMessage, void* prgchText, int cchTextMax, EChatEntryType* peChatEntryType, OUT_STRUCT() CSteamID* psteamidChatter) { return 0; }
bool CSteamFriends::IsClanChatAdmin(CSteamID steamIDClanChat, CSteamID steamIDUser) { return false; }

// interact with the Steam (game overlay / desktop)
bool CSteamFriends::IsClanChatWindowOpenInSteam(CSteamID steamIDClanChat) { return false; }
bool CSteamFriends::OpenClanChatWindowInSteam(CSteamID steamIDClanChat) { return false; }
bool CSteamFriends::CloseClanChatWindowInSteam(CSteamID steamIDClanChat) { return false; }

// peer-to-peer chat interception
bool CSteamFriends::SetListenForFriendsMessages(bool bInterceptEnabled) { return false; }
bool CSteamFriends::ReplyToFriendMessage(CSteamID steamIDFriend, const char* pchMsgToSend) { return false; }
int CSteamFriends::GetFriendMessage(CSteamID steamIDFriend, int iMessageID, void* pvData, int cubData, EChatEntryType* peChatEntryType) { return 0; }

// following apis
SteamAPICall_t CSteamFriends::GetFollowerCount(CSteamID steamID) { return generateSteamAPICallID(); }
SteamAPICall_t CSteamFriends::IsFollowing(CSteamID steamID) { return generateSteamAPICallID(); }
SteamAPICall_t CSteamFriends::EnumerateFollowingList(uint32 unStartIndex) { return generateSteamAPICallID(); }

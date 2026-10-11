#pragma once

#include "revCommon.h"

//-----------------------------------------------------------------------------
// Purpose: interface to accessing information about individual users,
//			that can be a friend, in a group, on a game server or in a lobby with the local user
//-----------------------------------------------------------------------------
class CSteamFriends : public ISteamFriends
{
public:
	CSteamFriends();
	~CSteamFriends();

	// returns the local players name - guaranteed to not be NULL.
	virtual const char *GetPersonaName();

	// Sets the player name, stores it on the server and publishes the changes to all friends who are online.
	virtual SteamAPICall_t SetPersonaName( const char *pchPersonaName );

	// gets the status of the current user
	virtual EPersonaState GetPersonaState();

	// friend iteration
	virtual int GetFriendCount( int iFriendFlags );

	// returns the steamID of a user
	// iFriend is a index of range [0, GetFriendCount())
	// iFriendsFlags must be the same value as used in GetFriendCount()
	// the returned CSteamID can then be used by all the functions below to access details about the user
	virtual CSteamID GetFriendByIndex( int iFriend, int iFriendFlags );

	// returns a relationship to a user
	virtual EFriendRelationship GetFriendRelationship( CSteamID steamIDFriend );

	// returns the current status of the specified user
	// this will only be known by the local user if steamIDFriend is in their friends list; on the same game server; in a chat room or lobby; or in a small group with the local user
	virtual EPersonaState GetFriendPersonaState( CSteamID steamIDFriend );

	// returns the name another user - guaranteed to not be NULL.
	virtual const char *GetFriendPersonaName( CSteamID steamIDFriend );

	// returns true if the friend is actually in a game, and fills in pFriendGameInfo with an extra details 
	virtual bool GetFriendGamePlayed( CSteamID steamIDFriend, OUT_STRUCT() FriendGameInfo_t *pFriendGameInfo );
	// accesses old friends names - returns an empty string when their are no more items in the history
	virtual const char *GetFriendPersonaNameHistory( CSteamID steamIDFriend, int iPersonaName );
	// friends steam level
	virtual int GetFriendSteamLevel( CSteamID steamIDFriend );

	// Returns nickname the current user has set for the specified player. Returns NULL if the no nickname has been set for that player.
	virtual const char *GetPlayerNickname( CSteamID steamIDPlayer );

	// friend grouping (tag) apis
	// returns the number of friends groups
	virtual int GetFriendsGroupCount();
	// returns the friends group ID for the given index (invalid indices return k_FriendsGroupID_Invalid)
	virtual FriendsGroupID_t GetFriendsGroupIDByIndex( int iFG );
	// returns the name for the given friends group (NULL in the case of invalid friends group IDs)
	virtual const char *GetFriendsGroupName( FriendsGroupID_t friendsGroupID );
	// returns the number of members in a given friends group
	virtual int GetFriendsGroupMembersCount( FriendsGroupID_t friendsGroupID );
	// gets up to nMembersCount members of the given friends group, if fewer exist than requested those positions' SteamIDs will be invalid
	virtual void GetFriendsGroupMembersList( FriendsGroupID_t friendsGroupID, OUT_ARRAY_CALL(nMembersCount, GetFriendsGroupMembersCount, friendsGroupID ) CSteamID *pOutSteamIDMembers, int nMembersCount );

	// returns true if the specified user meets any of the criteria specified in iFriendFlags
	virtual bool HasFriend( CSteamID steamIDFriend, int iFriendFlags );

	// clan (group) iteration and access functions
	virtual int GetClanCount();
	virtual CSteamID GetClanByIndex( int iClan );
	virtual const char *GetClanName( CSteamID steamIDClan );
	virtual const char *GetClanTag( CSteamID steamIDClan );
	// returns the most recent information we have about what's happening in a clan
	virtual bool GetClanActivityCounts( CSteamID steamIDClan, int *pnOnline, int *pnInGame, int *pnChatting );
	// for clans a user is a member of, they will have reasonably up-to-date information, but for others you'll have to download the info to have the latest
	virtual SteamAPICall_t DownloadClanActivityCounts( ARRAY_COUNT(cClansToRequest) CSteamID *psteamIDClans, int cClansToRequest );

	// iterators for getting users in a chat room, lobby, game server or clan
	virtual int GetFriendCountFromSource( CSteamID steamIDSource );
	virtual CSteamID GetFriendFromSourceByIndex( CSteamID steamIDSource, int iFriend );

	// returns true if the local user can see that steamIDUser is a member or in steamIDSource
	virtual bool IsUserInSource( CSteamID steamIDUser, CSteamID steamIDSource );

	// User is in a game pressing the talk button (will suppress the microphone for all voice comms from the Steam friends UI)
	virtual void SetInGameVoiceSpeaking( CSteamID steamIDUser, bool bSpeaking );

	// activates the game overlay, with an optional dialog to open 
	// valid options are "Friends", "Community", "Players", "Settings", "OfficialGameGroup", "Stats", "Achievements"
	virtual void ActivateGameOverlay( const char *pchDialog );

	// activates game overlay to a specific place
	virtual void ActivateGameOverlayToUser( const char *pchDialog, CSteamID steamID );

	// activates game overlay web browser directly to the specified URL
	// full address with protocol type is required, e.g. http://www.steamgames.com/
	virtual void ActivateGameOverlayToWebPage( const char *pchURL );

	// activates game overlay to store page for app
	virtual void ActivateGameOverlayToStore( AppId_t nAppID, EOverlayToStoreFlag eFlag );

	// Mark a target user as 'played with'. This is a client-side only feature that requires that the calling user is 
	// in game 
	virtual void SetPlayedWith( CSteamID steamIDUserPlayedWith );

	// activates game overlay to open the invite dialog. Invitations will be sent for the provided lobby.
	virtual void ActivateGameOverlayInviteDialog( CSteamID steamIDLobby );

	// gets the small (32x32) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
	virtual int GetSmallFriendAvatar( CSteamID steamIDFriend );

	// gets the medium (64x64) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
	virtual int GetMediumFriendAvatar( CSteamID steamIDFriend );

	// gets the large (184x184) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
	// returns -1 if this image has yet to be loaded, in this case wait for a AvatarImageLoaded_t callback and then call this again
	virtual int GetLargeFriendAvatar( CSteamID steamIDFriend );

	// requests information about a user - persona name & avatar
	virtual bool RequestUserInformation( CSteamID steamIDUser, bool bRequireNameOnly );

	// requests information about a clan officer list
	virtual SteamAPICall_t RequestClanOfficerList( CSteamID steamIDClan );

	// iteration of clan officers - can only be done when a RequestClanOfficerList() call has completed
	
	// returns the steamID of the clan owner
	virtual CSteamID GetClanOwner( CSteamID steamIDClan );
	// returns the number of officers in a clan (including the owner)
	virtual int GetClanOfficerCount( CSteamID steamIDClan );
	// returns the steamID of a clan officer, by index, of range [0,GetClanOfficerCount)
	virtual CSteamID GetClanOfficerByIndex( CSteamID steamIDClan, int iOfficer );
	// if current user is chat restricted, he can't send or receive any text/voice chat messages.
	virtual uint32 GetUserRestrictions();

	// Rich Presence data is automatically shared between friends who are in the same game
	virtual bool SetRichPresence( const char *pchKey, const char *pchValue );
	virtual void ClearRichPresence();
	virtual const char *GetFriendRichPresence( CSteamID steamIDFriend, const char *pchKey );
	virtual int GetFriendRichPresenceKeyCount( CSteamID steamIDFriend );
	virtual const char *GetFriendRichPresenceKeyByIndex( CSteamID steamIDFriend, int iKey );
	// Requests rich presence for a specific user.
	virtual void RequestFriendRichPresence( CSteamID steamIDFriend );

	// rich invite support
	virtual bool InviteUserToGame( CSteamID steamIDFriend, const char *pchConnectString );

	// recently-played-with friends iteration
	virtual int GetCoplayFriendCount();
	virtual CSteamID GetCoplayFriend( int iCoplayFriend );
	virtual int GetFriendCoplayTime( CSteamID steamIDFriend );
	virtual AppId_t GetFriendCoplayGame( CSteamID steamIDFriend );

	// chat interface for games
	virtual SteamAPICall_t JoinClanChatRoom( CSteamID steamIDClan );
	virtual bool LeaveClanChatRoom( CSteamID steamIDClan );
	virtual int GetClanChatMemberCount( CSteamID steamIDClan );
	virtual CSteamID GetChatMemberByIndex( CSteamID steamIDClan, int iUser );
	virtual bool SendClanChatMessage( CSteamID steamIDClanChat, const char *pchText );
	virtual int GetClanChatMessage( CSteamID steamIDClanChat, int iMessage, void *prgchText, int cchTextMax, EChatEntryType *peChatEntryType, OUT_STRUCT() CSteamID *psteamidChatter );
	virtual bool IsClanChatAdmin( CSteamID steamIDClanChat, CSteamID steamIDUser );

	// interact with the Steam (game overlay / desktop)
	virtual bool IsClanChatWindowOpenInSteam( CSteamID steamIDClanChat );
	virtual bool OpenClanChatWindowInSteam( CSteamID steamIDClanChat );
	virtual bool CloseClanChatWindowInSteam( CSteamID steamIDClanChat );

	// peer-to-peer chat interception
	virtual bool SetListenForFriendsMessages( bool bInterceptEnabled );
	virtual bool ReplyToFriendMessage( CSteamID steamIDFriend, const char *pchMsgToSend );
	virtual int GetFriendMessage( CSteamID steamIDFriend, int iMessageID, void *pvData, int cubData, EChatEntryType *peChatEntryType );

	// following apis
	virtual SteamAPICall_t GetFollowerCount( CSteamID steamID );
	virtual SteamAPICall_t IsFollowing( CSteamID steamID );
	virtual SteamAPICall_t EnumerateFollowingList( uint32 unStartIndex );
};

extern CSteamFriends* g_pSteamFriends;
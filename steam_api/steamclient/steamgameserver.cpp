#include "steamgameserver.h"
#include "useridvalidation.h"
#include "murmur32.h"

extern CSteamID g_uSteamID;

static CSteamGameServer s_steamgameserver;
CSteamGameServer* g_pSteamGameServer = &s_steamgameserver;

CSteamGameServer::CSteamGameServer()
{
	pr_unClientIP = 0;
	pr_pSteamID = 0;
	pr_hValidationHandle = 0;
	m_uSteamID = k_steamIDNil;
}

CSteamGameServer::~CSteamGameServer()
{

}


// custom functions, implemented by RuSHeRR
void CSteamGameServer::RunCallbacks() {

}

//
// Basic server data.  These properties, if set, must be set before before calling LogOn.  They
// may not be changed after logged in.
//

/// This is called by SteamGameServer_Init, and you will usually not need to call it directly
bool CSteamGameServer::InitGameServer(uint32 unIP, uint16 usGamePort, uint16 usQueryPort, uint32 unFlags, AppId_t nGameAppId, const char* pchVersionString) {
	return true;
}

/// Game product identifier.  This is currently used by the master server for version checking purposes.
/// It's a required field, but will eventually will go away, and the AppID will be used for this purpose.
void CSteamGameServer::SetProduct(const char* pszProduct) {}

/// Description of the game.  This is a required field and is displayed in the steam server browser....for now.
/// This is a required field, but it will go away eventually, as the data should be determined from the AppID.
void CSteamGameServer::SetGameDescription(const char* pszGameDescription) {}

/// If your game is a "mod," pass the string that identifies it.  The default is an empty string, meaning
/// this application is the original game, not a mod.
///
/// @see k_cbMaxGameServerGameDir
void CSteamGameServer::SetModDir(const char* pszModDir) {}

/// Is this is a dedicated server?  The default value is false.
void CSteamGameServer::SetDedicatedServer(bool bDedicated) {}

//
// Login
//

/// Begin process to login to a persistent game server account
///
/// You need to register for callbacks to determine the result of this operation.
/// @see SteamServersConnected_t
/// @see SteamServerConnectFailure_t
/// @see SteamServersDisconnected_t
void CSteamGameServer::LogOn(const char* pszToken) {
	LogOnAnonymous();
}

/// Login to a generic, anonymous account.
///
/// Note: in previous versions of the SDK, this was automatically called within SteamGameServer_Init,
/// but this is no longer the case.
void CSteamGameServer::LogOnAnonymous() {
	m_uSteamID.CreateBlankAnonLogon(k_EUniversePublic);
	//m_uSteamID.SetAccountID(g_uSteamID.GetAccountID() + 3);
	byte key[32] = "__REV_ANONONYMOUS_GAME_SERVER__";
	uint32 accountID = murmur3_32(key, sizeof(key), rand());
	m_uSteamID.SetAccountID(accountID);
}

/// Begin process of logging game server out of steam
void CSteamGameServer::LogOff() {
	m_uSteamID.Clear();
}

// status functions
bool CSteamGameServer::BLoggedOn() {
	return m_uSteamID.BAnonGameServerAccount();
}

bool CSteamGameServer::BSecure() {
	return true;
}

CSteamID CSteamGameServer::GetSteamID() {
	return m_uSteamID;
}

/// Returns true if the master server has requested a restart.
/// Only returns true once per request.
bool CSteamGameServer::WasRestartRequested() {
	return false;
}

//
// Server state.  These properties may be changed at any time.
//

/// Max player count that will be reported to server browser and client queries
void CSteamGameServer::SetMaxPlayerCount(int cPlayersMax) {}

/// Number of bots.  Default value is zero
void CSteamGameServer::SetBotPlayerCount(int cBotplayers) {}

/// Set the name of server as it will appear in the server browser
///
/// @see k_cbMaxGameServerName
void CSteamGameServer::SetServerName(const char* pszServerName) {}

/// Set name of map to report in the server browser
///
/// @see k_cbMaxGameServerName
void CSteamGameServer::SetMapName(const char* pszMapName) {}

/// Let people know if your server will require a password
void CSteamGameServer::SetPasswordProtected(bool bPasswordProtected) {}

/// Spectator server.  The default value is zero, meaning the service
/// is not used.
void CSteamGameServer::SetSpectatorPort(uint16 unSpectatorPort) {}

/// Name of the spectator server.  (Only used if spectator port is nonzero.)
///
/// @see k_cbMaxGameServerMapName
void CSteamGameServer::SetSpectatorServerName(const char* pszSpectatorServerName) {}

/// Call this to clear the whole list of key/values that are sent in rules queries.
void CSteamGameServer::ClearAllKeyValues() {}

/// Call this to add/update a key/value pair.
void CSteamGameServer::SetKeyValue(const char* pKey, const char* pValue) {}

/// Sets a string defining the "gametags" for this server, this is optional, but if it is set
/// it allows users to filter in the matchmaking/server-browser interfaces based on the value
///
/// @see k_cbMaxGameServerTags
void CSteamGameServer::SetGameTags(const char* pchGameTags) {}

/// Sets a string defining the "gamedata" for this server, this is optional, but if it is set
/// it allows users to filter in the matchmaking/server-browser interfaces based on the value
/// don't set this unless it actually changes, its only uploaded to the master once (when
/// acknowledged)
///
/// @see k_cbMaxGameServerGameData
void CSteamGameServer::SetGameData(const char* pchGameData) {}

/// Region identifier.  This is an optional field, the default value is empty, meaning the "world" region
void CSteamGameServer::SetRegion(const char* pszRegion) {}

//
// Player list management / authentication
//

// Handles receiving a new connection from a Steam user.  This call will ask the Steam
// servers to validate the users identity, app ownership, and VAC status.  If the Steam servers 
// are off-line, then it will validate the cached ticket itself which will validate app ownership 
// and identity.  The AuthBlob here should be acquired on the game client using SteamUser()->InitiateGameConnection()
// and must then be sent up to the game server for authentication.
//
// Return Value: returns true if the users ticket passes basic checks. pSteamIDUser will contain the Steam ID of this user. pSteamIDUser must NOT be NULL
// If the call succeeds then you should expect a GSClientApprove_t or GSClientDeny_t callback which will tell you whether authentication
// for the user has succeeded or failed (the steamid in the callback will match the one returned by this call)
bool CSteamGameServer::SendUserConnectAndAuthenticate(uint32 unIPClient, const void* pvAuthBlob, uint32 cubAuthBlobSize, CSteamID* pSteamIDUser) {
	if (!pSteamIDUser)
		return false;

	pr_unClientIP = unIPClient;
	pr_pSteamID = pSteamIDUser;

	return true;
}

// Creates a fake user (ie, a bot) which will be listed as playing on the server, but skips validation.  
// 
// Return Value: Returns a SteamID for the user to be tracked with, you should call HandleUserDisconnect()
// when this user leaves the server just like you would for a real user.
CSteamID CSteamGameServer::CreateUnauthenticatedUserConnection() {
	return k_steamIDNil;
}

// Should be called whenever a user leaves our game server, this lets Steam internally
// track which users are currently on which servers for the purposes of preventing a single
// account being logged into multiple servers, showing who is currently on a server, etc.
void CSteamGameServer::SendUserDisconnect(CSteamID steamIDUser) {}

// Update the data to be displayed in the server browser and matchmaking interfaces for a user
// currently connected to the server.  For regular users you must call this after you receive a
// GSUserValidationSuccess callback.
// 
// Return Value: true if successful, false if failure (ie, steamIDUser wasn't for an active player)
bool CSteamGameServer::BUpdateUserData(CSteamID steamIDUser, const char* pchPlayerName, uint32 uScore) {
	return true;
}

// New auth system APIs - do not mix with the old auth system APIs.
// ----------------------------------------------------------------

// Retrieve ticket to be sent to the entity who wishes to authenticate you ( using BeginAuthSession API ). 
// pcbTicket retrieves the length of the actual ticket.
HAuthTicket CSteamGameServer::GetAuthSessionTicket(void* pTicket, int cbMaxTicket, uint32* pcbTicket) {
	memset(pTicket, 1, 152);
	*pcbTicket = 152;
	return 47;
}

// Authenticate ticket ( from GetAuthSessionTicket ) from entity steamID to be sure it is valid and isnt reused
// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
EBeginAuthSessionResult CSteamGameServer::BeginAuthSession(const void* pAuthTicket, int cbAuthTicket, CSteamID steamID) {
	if (pr_pSteamID != 0)
	{
		TRevUserValidationHandle* handle = nullptr;
		SteamStartValidatingUserIDTicket((void*)pAuthTicket, cbAuthTicket, pr_unClientIP, &handle);
		ESteamError err = 
			SteamProcessOngoingUserIDTicketValidation(&handle, (void*)pAuthTicket, cbAuthTicket);

		if (err == eSteamErrorCorruptEncryptedUserIDTicket || err == eSteamErrorInvalidUserIDTicket)
			return k_EBeginAuthSessionResultInvalidTicket;

		pr_pSteamID->SetFromUint64(handle->uSteamID.ConvertToUint64());

		LogStats(true, false, handle);

		//pr_hValidationHandle = handle;
		pr_pSteamID = 0;
		pr_unClientIP = 0;
		return k_EBeginAuthSessionResultOK;
	}

	// i did a little gimmick so now SendUserConnectAndAuthenticate MUST BE called before any authsession calls
	// why? because we need to manually set client's SteamID from the ticket one.
	// if we didn't get SendUserConnectAndAuthenticate called, return InvalidTicket
	return k_EBeginAuthSessionResultInvalidTicket;
}

// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
void CSteamGameServer::EndAuthSession(CSteamID steamID) {}

// Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
void CSteamGameServer::CancelAuthTicket(HAuthTicket hAuthTicket) {}

// After receiving a user's authentication data, and passing it to SendUserConnectAndAuthenticate, use this function
// to determine if the user owns downloadable content specified by the provided AppID.
EUserHasLicenseForAppResult CSteamGameServer::UserHasLicenseForApp(CSteamID steamID, AppId_t appID) {
	return k_EUserHasLicenseResultHasLicense;
}

// Ask if a user in in the specified group, results returns async by GSUserGroupStatus_t
// returns false if we're not connected to the steam servers and thus cannot ask
bool CSteamGameServer::RequestUserGroupStatus(CSteamID steamIDUser, CSteamID steamIDGroup) { return 1; }


// these two functions s are deprecated, and will not return results
// they will be removed in a future version of the SDK
void CSteamGameServer::GetGameplayStats() {}
SteamAPICall_t CSteamGameServer::GetServerReputation() { return 1; }

// Returns the public IP of the server according to Steam, useful when the server is 
// behind NAT and you want to advertise its IP in a lobby for other clients to directly
// connect to
uint32 CSteamGameServer::GetPublicIP() { return 1; }

// Call this when a packet that starts with 0xFFFFFFFF comes in. That means
// it's for us.
bool CSteamGameServer::HandleIncomingPacket(const void* pData, int cbData, uint32 srcIP, uint16 srcPort) { return 1; }

// AFTER calling HandleIncomingPacket for any packets that came in that frame, call this.
// This gets a packet that the master server updater needs to send out on UDP.
// It returns the length of the packet it wants to send, or 0 if there are no more packets to send.
// Call this each frame until it returns 0.
int CSteamGameServer::GetNextOutgoingPacket(void* pOut, int cbMaxOut, uint32* pNetAdr, uint16* pPort) { return 1; }

//
// Control heartbeats / advertisement with master server
//

// Call this as often as you like to tell the master server updater whether or not
// you want it to be active (default: off).
void CSteamGameServer::EnableHeartbeats(bool bActive) {}

// You usually don't need to modify this.
// Pass -1 to use the default value for iHeartbeatInterval.
// Some mods change this.
void CSteamGameServer::SetHeartbeatInterval(int iHeartbeatInterval) {}

// Force a heartbeat to steam at the next opportunity
void CSteamGameServer::ForceHeartbeat() {}

// associate this game server with this clan for the purposes of computing player compat
SteamAPICall_t CSteamGameServer::AssociateWithClan(CSteamID steamIDClan) { return 1; }

// ask if any of the current players dont want to play with this new player - or vice versa
SteamAPICall_t CSteamGameServer::ComputeNewPlayerCompatibility(CSteamID steamIDNewPlayer) { return 1; }

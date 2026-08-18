#pragma once

#include "revCommon.h"

//-----------------------------------------------------------------------------
// Purpose: Functions for accessing and manipulating a steam account
//			associated with one client instance
//-----------------------------------------------------------------------------
class CSteamUser : public ISteamUser
{
public:
	CSteamUser();
	~CSteamUser();
	// returns the HSteamUser this interface represents
	// this is only used internally by the API, and by a few select interfaces that support multi-user
	virtual HSteamUser GetHSteamUser();

	// returns true if the Steam client current has a live connection to the Steam servers. 
	// If false, it means there is no active connection due to either a networking issue on the local machine, or the Steam server is down/busy.
	// The Steam client will automatically be trying to recreate the connection as often as possible.
	virtual bool BLoggedOn();

	// returns the CSteamID of the account currently logged into the Steam client
	// a CSteamID is a unique identifier for an account, and used to differentiate users in all parts of the Steamworks API
	virtual CSteamID GetSteamID();

	// Multiplayer Authentication functions
	
	// InitiateGameConnection() starts the state machine for authenticating the game client with the game server
	// It is the client portion of a three-way handshake between the client, the game server, and the steam servers
	virtual int InitiateGameConnection(void* pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure);

	// notify of disconnect
	// needs to occur when the game client leaves the specified game server, needs to match with the InitiateGameConnection() call
	virtual void TerminateGameConnection( uint32 unIPServer, uint16 usPortServer ) { /*legacy*/ }

	// Legacy functions

	// used by only a few games to track usage events
	virtual void TrackAppUsageEvent( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo = "" ) {}

	// get the local storage folder for current Steam account to write application data, e.g. save games, configs etc.
	// this will usually be something like "C:\Progam Files\Steam\userdata\<SteamID>\<AppID>\local"
	virtual bool GetUserDataFolder( char *pchBuffer, int cubBuffer ) { return false;}

	virtual void StartVoiceRecording( ) {}
	virtual void StopVoiceRecording( ) {}
	virtual EVoiceResult GetAvailableVoice( 
		uint32 *pcbCompressed, uint32 *pcbUncompressed, uint32 nUncompressedVoiceDesiredSampleRate ) {return k_EVoiceResultOK;}
	virtual EVoiceResult GetVoice( 
		bool bWantCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, 
		bool bWantUncompressed, void *pUncompressedDestBuffer, uint32 cbUncompressedDestBufferSize, 
		uint32 *nUncompressBytesWritten, uint32 nUncompressedVoiceDesiredSampleRate ) {return k_EVoiceResultOK;}
	virtual EVoiceResult DecompressVoice( 
		const void *pCompressed, uint32 cbCompressed, void *pDestBuffer, 
		uint32 cbDestBufferSize, uint32 *nBytesWritten, uint32 nDesiredSampleRate ) {return k_EVoiceResultOK;}
	virtual uint32 GetVoiceOptimalSampleRate() { return 0; }

	// Retrieve ticket to be sent to the entity who wishes to authenticate you. 
	// pcbTicket retrieves the length of the actual ticket.
	virtual HAuthTicket GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket );

	// Authenticate ticket from entity steamID to be sure it is valid and isnt reused
	// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
	virtual EBeginAuthSessionResult BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID );

	// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
	virtual void EndAuthSession( CSteamID steamID );

	// Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
	virtual void CancelAuthTicket( HAuthTicket hAuthTicket );

	// After receiving a user's authentication data, and passing it to BeginAuthSession, use this function
	// to determine if the user owns downloadable content specified by the provided AppID.
	virtual EUserHasLicenseForAppResult UserHasLicenseForApp(CSteamID steamID, AppId_t appID) { return k_EUserHasLicenseResultHasLicense; }
	
	// returns true if this users looks like they are behind a NAT device. Only valid once the user has connected to steam 
	// (i.e a SteamServersConnected_t has been issued) and may not catch all forms of NAT.
	virtual bool BIsBehindNAT() { return false; }

	// set data to be replicated to friends so that they can join your game
	// CSteamID steamIDGameServer - the steamID of the game server, received from the game server by the client
	// uint32 unIPServer, uint16 usPortServer - the IP address of the game server
	virtual void AdvertiseGame( CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer ) {}

	// Requests a ticket encrypted with an app specific shared key
	// pDataToInclude, cbDataToInclude will be encrypted into the ticket
	// ( This is asynchronous, you must wait for the ticket to be completed by the server )
	virtual SteamAPICall_t RequestEncryptedAppTicket(void* pDataToInclude, int cbDataToInclude) { return 0; }

	// retrieve a finished ticket
	virtual bool GetEncryptedAppTicket(void* pTicket, int cbMaxTicket, uint32* pcbTicket) { return false; }

	virtual int GetGameBadgeLevel(int nSeries, bool bFoil) { return 0; }
	virtual int GetPlayerSteamLevel() { return 0; }
	virtual SteamAPICall_t RequestStoreAuthURL(const char* pchRedirectURL) { return 0; }
};

extern CSteamUser* g_pSteamUser;
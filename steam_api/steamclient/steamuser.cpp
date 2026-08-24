#include "steamuser.h"
#include "useridvalidation.h"
#include "logging.h"
#include "tier0/dbg.h"

extern CLoggingSystem* Logger;
extern CSteamID g_uSteamID;
static CSteamUser s_steamuser;
CSteamUser* g_pSteamUser = &s_steamuser;

static HAuthTicket g_hAuthTicket = k_HAuthTicketInvalid;

// Constructor
CSteamUser::CSteamUser()
{
}

// Destructor
CSteamUser::~CSteamUser()
{
}

// returns the HSteamUser this interface represents
// this is only used internally by the API, and by a few select interfaces that support multi-user
HSteamUser CSteamUser::GetHSteamUser(void)
{
	return 1;
}

// returns true if the Steam client current has a live connection to the Steam servers. 
// If false, it means there is no active connection due to either a networking issue on the local machine, or the Steam server is down/busy.
// The Steam client will automatically be trying to recreate the connection as often as possible.
bool CSteamUser::BLoggedOn(void)
{
	if (!g_uSteamID.IsValid())
		return false;

	return true;
}

// returns the CSteamID of the account currently logged into the Steam client
// a CSteamID is a unique identifier for an account, and used to differentiate users in all parts of the Steamworks API
CSteamID CSteamUser::GetSteamID(void)
{
	if (!g_uSteamID.IsValid())
	{
		// try to update some values in steam id... idk why it is invalid sometimes
		g_uSteamID.Set(g_uSteamID.GetAccountID(), k_EUniversePublic, k_EAccountTypeIndividual);
	}

	return g_uSteamID;
}

// InitiateGameConnection() starts the state machine for authenticating the game client with the game server
// It is the client portion of a three-way handshake between the client, the game server, and the steam servers
int CSteamUser::InitiateGameConnection(void* pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure)
{
	uint32 len;
	GetAuthSessionTicket(pAuthBlob, cbMaxAuthBlob, &len);
	return (int)len;
}

// Retrieve ticket to be sent to the entity who wishes to authenticate you. 
// pcbTicket retrieves the length of the actual ticket.
HAuthTicket CSteamUser::GetAuthSessionTicket(void* pTicket, int cbMaxTicket, uint32* pcbTicket) {
	ESteamError status = SteamGetEncryptedUserIDTicket(pTicket, 2048, pcbTicket);

	switch (status)
	{

	case eSteamErrorBadArg: {
		Logger->Write("Invalid pTicket in CSteamUser::GetAuthSessionTicket\n");
		Warning("Invalid pTicket in CSteamUser::GetAuthSessionTicket\n");
		return 0;
	}
	case eSteamErrorLoginFailed: {
		Logger->Write("SteamUser Logon failed\n");
		Warning("SteamUser Logon failed\n");
		return 0;
	}
	case eSteamErrorNone: {
		break;
	}
	default:
		return 0;

	}

	// write some debug logs
	Logger->Write("CSteamUser::GetAuthSessionTicket: ticketlen %u, steamID <%s> %s\n", *pcbTicket, GetUserIDString(g_uSteamID), g_uSteamID.Render());

	g_hAuthTicket++;

	if (g_hAuthTicket == 0xFFFFFFFE)
		g_hAuthTicket = 1;

	return g_hAuthTicket;
}

// Authenticate ticket from entity steamID to be sure it is valid and isnt reused
// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
EBeginAuthSessionResult CSteamUser::BeginAuthSession(const void* pAuthTicket, int cbAuthTicket, CSteamID steamID) {
	return k_EBeginAuthSessionResultOK;
}

// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
void CSteamUser::EndAuthSession(CSteamID steamID) {

}

// Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
void CSteamUser::CancelAuthTicket(HAuthTicket hAuthTicket) {
	if (g_hAuthTicket != hAuthTicket)
		return;

	if (g_hAuthTicket > 0)
		g_hAuthTicket--;
}
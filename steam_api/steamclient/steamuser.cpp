#include "steamuser.h"
#include "useridvalidation.h"

static CSteamUser s_steamuser;
CSteamUser* g_pSteamUser = &s_steamuser;

static HAuthTicket g_hAuthTicket = k_HAuthTicketInvalid;

CSteamUser::CSteamUser()
{
}

CSteamUser::~CSteamUser()
{
}

HSteamUser CSteamUser::GetHSteamUser(void)
{
	return 1;
}

bool CSteamUser::BLoggedOn(void)
{
	return true;
}

CSteamID CSteamUser::GetSteamID(void)
{
	CSteamID a;
	a.FullSet(5, k_EUniversePublic, k_EAccountTypeIndividual);
	return a;
}
int CSteamUser::InitiateGameConnection(void * a1, int a2, CSteamID a3, unsigned int a4, unsigned short a5, bool a6)
{
	return 152;
}

CSteamUser* GSteamUser(void)
{
	static CSteamUser g_SteamUser;
	return &g_SteamUser;
}

// Retrieve ticket to be sent to the entity who wishes to authenticate you. 
// pcbTicket retrieves the length of the actual ticket.
HAuthTicket CSteamUser::GetAuthSessionTicket(void* pTicket, int cbMaxTicket, uint32* pcbTicket) {
	ESteamError status = SteamGetEncryptedUserIDTicket(pTicket, 2048, pcbTicket);

	if (status == eSteamErrorNone)
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
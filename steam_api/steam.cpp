#pragma once

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#undef SetPort
#undef CreateEvent
#else
#include <unistd.h>
#endif

#include "revCommon.h"
#include "useridvalidation.h"
#include "logging.h"
#include "tier0/dbg.h"

char g_chName[50];
char g_chLang[20];
char g_pchServerBrowser[255];
char g_chMasterServer[255] = "78.154.103.37:10232";
bool g_bLogging = true;
CLoggingSystem* Logger;

CSteamID g_uSteamID;
char g_szHostID[128];

BOOL bSteamStartup = FALSE;

/*
static char masterServers[][37] =
{
	"78.154.103.37:10232", // nttnmDev (https://github.com/nttnmDev/cssv34masterserver)
	"91.218.230.217:27011", // reserved
};
*/

unsigned int getHostID()
{
#ifdef _WIN32
	DWORD serialNumber = 0;
	GetVolumeInformationA("C:\\", NULL, 0, &serialNumber, NULL, NULL, NULL, 0);
	return serialNumber;
#else
	unsigned int id = 0;
	char hostname[256] = { 0 };

	if (gethostname(hostname, sizeof(hostname)) == 0)
	{
		return JSHash(hostname, strlen(hostname));
	}
#endif
	return 0;
}

/*
* Initialization
*/
int S_CALLTYPE steamclient_startup()
{
	if (!bSteamStartup)
		bSteamStartup = TRUE;
	else
		return 0;

	uint32 hostID = getHostID();

	snprintf(g_szHostID, 16 * sizeof(char), "%u", hostID);

	uint32 accountID = JSHash(g_szHostID, strlen(g_szHostID));
	accountID *= 2;

	g_uSteamID.Set(accountID, k_EUniversePublic, k_EAccountTypeIndividual);

	char szCurrentDir[260];
	getcwd(szCurrentDir, sizeof(szCurrentDir));

	strcpy(g_pchServerBrowser, szCurrentDir);
	strcat(g_pchServerBrowser, "\\platform\\config\\serverbrowser.vdf");

	V_FixSlashes(g_pchServerBrowser);

	strcpy(g_chLang, "English");
	strcpy(g_chName, "revCrew");

	// logging dir
	char szLoggingDir[260];
	strcpy(szLoggingDir, szCurrentDir);
	strcat(szLoggingDir, "\\rev-client.log");
	
	V_FixSlashes(szLoggingDir);

	/*if (g_bLogging)*/
	Logger = new CLoggingSystem(szLoggingDir);
	Logger->Write("Startup\n");
	Logger->Write("Logged on as %s: %s <%s>\n", g_chName, g_uSteamID.Render(), GetUserIDString(g_uSteamID));
	return 1;
}

/*
* Shutdown
*/
int S_CALLTYPE steamclient_shutdown()
{
	if (!bSteamStartup)
		return 0;
	
	Logger->Write("Shutdown\n");
	if (Logger)
		delete Logger;

	bSteamStartup = FALSE;
	return 1;
}
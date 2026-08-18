#include "revCommon.h"
#include "SteamApps.h"

static CSteamApps s_steamapps;
CSteamApps* g_pSteamApps = &s_steamapps;

extern char g_chLang[20];
extern CSteamID g_uSteamID;

CSteamApps::CSteamApps()
{
	// voided
}

CSteamApps::~CSteamApps()
{
	// voided
}

bool CSteamApps::BIsSubscribed(void)
{
	return true;
}
bool CSteamApps::BIsLowViolence(void)
{
	return false;
}
bool CSteamApps::BIsCybercafe(void)
{
	return false;
}
bool CSteamApps::BIsVACBanned(void)
{
	return false;
}
const char* CSteamApps::GetCurrentGameLanguage(void)
{
	return g_chLang;
}
const char* CSteamApps::GetAvailableGameLanguages(void)
{
	return g_chLang;
}

// only use this member if you need to check ownership of another game related to yours, a demo for example
bool CSteamApps::BIsSubscribedApp(AppId_t appID) {
	return true;
}

// Takes AppID of DLC and checks if the user owns the DLC & if the DLC is installed
bool CSteamApps::BIsDlcInstalled(AppId_t appID) {
	return false;
}

// returns the Unix time of the purchase of the app
uint32 CSteamApps::GetEarliestPurchaseUnixTime(AppId_t nAppID) {
	return 0;
}

// Checks if the user is subscribed to the current app through a free weekend
// This function will return false for users who have a retail or other type of license
// Before using, please ask your Valve technical contact how to package and secure your free weekened
bool CSteamApps::BIsSubscribedFromFreeWeekend() {
	return false;
}

// Returns the number of DLC pieces for the running app
int CSteamApps::GetDLCCount() {
	return 0;
}

// Returns metadata for DLC by index, of range [0, GetDLCCount()]
bool CSteamApps::BGetDLCDataByIndex(
	int iDLC, AppId_t* pAppID, bool* pbAvailable, char* pchName, int cchNameBufferSize) {
	*pAppID = 0;
	*pbAvailable = 0;
	return false;
}

// Install/Uninstall control for optional DLC
void CSteamApps::InstallDLC(AppId_t nAppID) {}
void CSteamApps::UninstallDLC(AppId_t nAppID) {}

// Request cd-key for yourself or owned DLC. If you are interested in this
// data then make sure you provide us with a list of valid keys to be distributed
// to users when they purchase the game, before the game ships.
// You'll receive an AppProofOfPurchaseKeyResponse_t callback when
// the key is available (which may be immediately).
void CSteamApps::RequestAppProofOfPurchaseKey(AppId_t nAppID) {}

// returns current beta branch name, 'public' is the default branch
bool CSteamApps::GetCurrentBetaName(char* pchName, int cchNameBufferSize) {
	strncpy(pchName, "public", cchNameBufferSize);
	return true;
}

// signal Steam that game files seems corrupt or missing
bool CSteamApps::MarkContentCorrupt(bool bMissingFilesOnly) {
	return false;
}

// return installed depots in mount order
uint32 CSteamApps::GetInstalledDepots(AppId_t appID, DepotId_t* pvecDepots, uint32 cMaxDepots) {
	pvecDepots[0] = 240;
	pvecDepots[1] = 241;
	pvecDepots[2] = 242;
	return 3;
} 

// returns current app install folder for AppID, returns folder name length
uint32 CSteamApps::GetAppInstallDir(AppId_t appID, char* pchFolder, uint32 cchFolderBufferSize) {
	return 0;
}

// returns true if that app is installed (not necessarily owned)
bool CSteamApps::BIsAppInstalled(AppId_t appID) {
	return true;
}

// returns the SteamID of the original owner. If different from current user, it's borrowed
CSteamID CSteamApps::GetAppOwner() {
	return g_uSteamID;
}

// Returns the associated launch param if the game is run via steam://run/<appid>//?param1=value1{}param2=value2{}param3=value3 etc.
// Parameter names starting with the character '@' are reserved for internal use and will always return and empty string.
// Parameter names starting with an underscore '_' are reserved for steam features -- they can be queried by the game,
// but it is advised that you not param names beginning with an underscore for your own features.
const char* CSteamApps::GetLaunchQueryParam(const char* pchKey) {
	return "";
}

// get download progress for optional DLC
bool CSteamApps::GetDlcDownloadProgress(AppId_t nAppID, uint64* punBytesDownloaded, uint64* punBytesTotal) 
{
	*punBytesDownloaded = 0;
	*punBytesTotal = 0;
	return false;
}

// return the buildid of this app, may change at any time based on backend updates to the game
int CSteamApps::GetAppBuildId() {
	return 4100;
}
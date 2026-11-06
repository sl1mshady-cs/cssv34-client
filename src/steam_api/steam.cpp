#include <locale>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#undef SetPort
#undef CreateEvent
#else
#include <unistd.h>
#endif
////////////////////////////////

#include "auth.h"
#include "./logging.h"
#include "tier0/dbg.h"

////////////////////////////////////////////////////////////////
/* GLOBALS */

char g_chName[50];
char g_chLang[20];
char g_pchServerBrowser[255];
char g_chMasterServer[255] = "78.154.103.37:10232";
bool g_bLogging = true;
CLoggingFile* Logger;

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

////////////////////////////////

/*
* Get host identificator (TODO: rework for android!)
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

std::string getSystemLanguage()
{
#ifdef _WIN32

    LANGID langId = GetUserDefaultUILanguage();

    switch (PRIMARYLANGID(langId))
    {
    case LANG_ENGLISH:    return "English";
    case LANG_SPANISH:    return "Spanish";
    case LANG_FRENCH:     return "French";
    case LANG_GERMAN:     return "German";
    case LANG_ITALIAN:    return "Italian";
    case LANG_PORTUGUESE:return "Portuguese";
    case LANG_RUSSIAN:    return "Russian";
    case LANG_UKRAINIAN:  return "Ukrainian";
    case LANG_POLISH:     return "Polish";
    case LANG_CZECH:      return "Czech";
    case LANG_JAPANESE:   return "Japanese";
    case LANG_KOREAN:     return "Korean";
    case LANG_CHINESE:    return "Chinese";
    }

#else

    const char* langEnv = std::getenv("LC_ALL");

    if (!langEnv || *langEnv == '\0')
        langEnv = std::getenv("LC_MESSAGES");

    if (!langEnv || *langEnv == '\0')
        langEnv = std::getenv("LANG");

    if (!langEnv || *langEnv == '\0')
        return "Unknown";

    std::string locale(langEnv);

    if (locale == "C" || locale == "POSIX")
        return "English";

    if (locale.length() < 2)
        return "Unknown";

    std::string langCode = locale.substr(0, 2);

    static const std::unordered_map<std::string, std::string> langMap =
    {
        {"en", "English"},
        {"es", "Spanish"},
        {"fr", "French"},
        {"de", "German"},
        {"it", "Italian"},
        {"ja", "Japanese"},
        {"zh", "Chinese"},
        {"cs", "Czech"},
        {"pt", "Portuguese"},
        {"ru", "Russian"},
        {"uk", "Ukrainian"},
        {"pl", "Polish"},
        {"ko", "Korean"}
    };

    auto it = langMap.find(langCode);

    if (it != langMap.end())
        return it->second;

#endif

    return "Unknown";
}

// Xoshiro256**
static uint64 xoshiro_state[4] = { 0x12345, 0x6789A, 0xBCDEF, 0x13579 };

static inline uint64 rotl(const uint64 x, int k) {
    return (x << k) | (x >> (64 - k));
}

void randombytes(uint8* buf, size_t size)
{
    size_t i = 0;

    // generate in portions of 8 bytes (size uint64_t), as long as space allows
    while (i + 8 <= size) {
        // Calculating a random 64-bit number using the Xoshiro256**
        const uint64_t result = rotl(xoshiro_state[1] * 5, 7) * 9;

        // Write 8 bytes to the buffer
        *(uint64_t*)(buf + i) = result;

        // Updating the internal state of the generator
        const uint64_t t = xoshiro_state[1] << 17;
        xoshiro_state[2] ^= xoshiro_state[0];
        xoshiro_state[3] ^= xoshiro_state[1];
        xoshiro_state[1] ^= xoshiro_state[2];
        xoshiro_state[0] ^= xoshiro_state[3];
        xoshiro_state[2] ^= t;
        xoshiro_state[3] = rotl(xoshiro_state[3], 45);

        i += 8;
    }

    // If the buffer size is not a multiple of 8, we finish off the remaining bytes
    if (i < size) {
        uint64_t result = rotl(xoshiro_state[1] * 5, 7) * 9;
        while (i < size) {
            buf[i++] = (uint8_t)result;
            result >>= 8;
        }
    }
}

unsigned generateAccountID()
{
    int a = 0;
    randombytes((uint8*)&a, sizeof(a));
    a = abs(a);
    if (!a) ++a;
    return a;
}

SteamAPICall_t generateSteamAPICallID() {
    static SteamAPICall_t a;
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    randombytes((uint8*)&a, sizeof(a));
    ++a;
    if (a == 0) ++a;
    return a;
}

CSteamID generateSteamIDUser()
{
    return CSteamID(generateAccountID(), k_unSteamUserDesktopInstance, k_EUniversePublic, k_EAccountTypeIndividual);
}

CSteamID generateSteamIDAnonUser()
{
    return CSteamID(generateAccountID(), k_unSteamUserDesktopInstance, k_EUniversePublic, k_EAccountTypeAnonUser);
}


CSteamID generateSteamIDGameServer()
{
    return CSteamID(generateAccountID(), k_unSteamUserDesktopInstance, k_EUniversePublic, k_EAccountTypeGameServer);
}

CSteamID generateSteamIDAnonServer()
{
    return CSteamID(generateAccountID(), k_unSteamUserDesktopInstance, k_EUniversePublic, k_EAccountTypeAnonGameServer);
}

CSteamID generateSteamIDLobby()
{
    return CSteamID(generateAccountID(), k_EChatInstanceFlagLobby | k_EChatInstanceFlagMMSLobby, k_EUniversePublic, k_EAccountTypeChat);
}

CSteamID generateSteamIDClan()
{
    return CSteamID(generateAccountID(), k_EChatInstanceFlagClan, k_EUniversePublic, k_EAccountTypeChat);
}


bool check_timedout(std::chrono::high_resolution_clock::time_point old, double timeout,
	std::chrono::high_resolution_clock::time_point now)
{
	if (timeout == 0.0) return true;

	if (std::chrono::duration_cast<std::chrono::duration<double>>(now - old).count() > timeout) {
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////

/*
* Initialization
*/
int S_CALLTYPE steam_startup()
{
	if (!bSteamStartup)
		bSteamStartup = TRUE;
	else
		return 0;

	uint32 hostID = getHostID();

	snprintf(g_szHostID, 16 * sizeof(char), "%u", hostID);

	uint32 accountID = JSHash(g_szHostID, strlen(g_szHostID));
	accountID *= 2;

	// initialize steamid
	g_uSteamID = CSteamID(accountID, k_unSteamUserDesktopInstance, k_EUniversePublic, k_EAccountTypeIndividual);

	// store cwd
	char szCurrentDir[260];
	getcwd(szCurrentDir, sizeof(szCurrentDir));

	strcpy(g_pchServerBrowser, szCurrentDir);
	strcat(g_pchServerBrowser, "\\platform\\config\\serverbrowser.vdf");

	V_FixSlashes(g_pchServerBrowser);

	// default values, todo rev.ini
	strcpy(g_chLang, getSystemLanguage().c_str());
	strcpy(g_chName, "revCrew");

	// logging dir
	char szLoggingDir[260];
	strcpy(szLoggingDir, szCurrentDir);
	strcat(szLoggingDir, "\\rev-client.log");
	V_FixSlashes(szLoggingDir);
    
	Logger = new CLoggingFile(szLoggingDir);
	Logger->Write("Startup\n");
	Logger->Write("Logged on as %s: %s <%s>\n", g_chName, g_uSteamID.Render(), g_uSteamID.RenderAsSteam2String());
	return 1;
}

/*
* Shutdown
*/
int S_CALLTYPE steam_shutdown()
{
	if (!bSteamStartup)
		return 0;
	
	Logger->Write("Shutdown\n");
	if (Logger)
		delete Logger;

	bSteamStartup = FALSE;
	return 1;
}

////////////////////////////////////////////////////////////////
#include "callback_system.h"
#include "steamclient.h"
#include "logging.h"

CSteamApps* g_pSteamApps = 0;
CSteamGameServer* g_pSteamGameServer = 0;
CSteamMatchMaking* g_pSteamMatchMaking = 0;
CSteamMatchMakingServers* g_pSteamMatchMakingServers = 0;
CSteamUser* g_pSteamUser = 0;

bool cb_run_active = false;
uint64 last_cb_run = 0;

extern CLoggingFile g_CallbackLog;
#define PRINT_DEBUG(msg, ...) g_CallbackLog.Write(msg "\n", __VA_ARGS__)

CSteamClient::CSteamClient()
{
	m_interfaceStub = new interface_stub_t();

    run_every_runcb = new RunEveryRunCB();

    callback_results_client = new SteamCallResults();
    callbacks_client = new SteamCallbacks(callback_results_client);

    callback_results_server = new SteamCallResults();
    callbacks_server = new SteamCallbacks(callback_results_server);

	g_pSteamApps = new CSteamApps();
	g_pSteamGameServer = new CSteamGameServer(callbacks_server);
	g_pSteamMatchMaking = new CSteamMatchMaking();
	g_pSteamMatchMakingServers = new CSteamMatchMakingServers(callbacks_client);
	g_pSteamUser = new CSteamUser(callbacks_client);

	m_pszVersion = 0;
	m_bInitialized = true;
}

CSteamClient::~CSteamClient() 
{
    #define DEL_INST(_obj_ins) do if (_obj_ins) { delete _obj_ins; _obj_ins = nullptr; } while(0)

    DEL_INST(m_interfaceStub);
    DEL_INST(g_pSteamApps);
    DEL_INST(g_pSteamGameServer);
    DEL_INST(g_pSteamMatchMaking);
    DEL_INST(g_pSteamMatchMakingServers);
    DEL_INST(g_pSteamUser);

    DEL_INST(callbacks_server);
    DEL_INST(callback_results_server);
    DEL_INST(callbacks_client);
    DEL_INST(callback_results_client);

    DEL_INST(run_every_runcb);

	m_pszVersion = 0;
	m_bInitialized = false;

    #undef DEL_INST
}

void CSteamClient::RegisterCallback(class CCallbackBase* pCallback, int iCallback)
{
    CCallbackBase* callback_to_add = pCallback;

    int base_callback = (iCallback / 100) * 100;
    int callback_id = iCallback % 100;
    bool isGameServer = CCallbackMgr::IsServer(callback_to_add);
    PRINT_DEBUG("isGameServer %u %i %i", isGameServer, iCallback, base_callback);

    switch (base_callback) {
    case k_iSteamUserCallbacks:
        PRINT_DEBUG("k_iSteamUserCallbacks %i", callback_id);
        break;

    case k_iSteamGameServerCallbacks:
        PRINT_DEBUG("k_iSteamGameServerCallbacks %i", callback_id);
        break;

    case k_iSteamFriendsCallbacks:
        PRINT_DEBUG("k_iSteamFriendsCallbacks %i", callback_id);
        break;

    case k_iSteamBillingCallbacks:
        PRINT_DEBUG("k_iSteamBillingCallbacks %i", callback_id);
        break;

    case k_iSteamMatchmakingCallbacks:
        PRINT_DEBUG("k_iSteamMatchmakingCallbacks %i", callback_id);
        break;

    case k_iSteamContentServerCallbacks:
        PRINT_DEBUG("k_iSteamContentServerCallbacks %i", callback_id);
        break;

    case k_iSteamUtilsCallbacks:
        PRINT_DEBUG("k_iSteamUtilsCallbacks %i", callback_id);
        break;

    case k_iClientFriendsCallbacks:
        PRINT_DEBUG("k_iClientFriendsCallbacks %i", callback_id);
        break;

    case k_iClientUserCallbacks:
        PRINT_DEBUG("k_iClientUserCallbacks %i", callback_id);
        break;

    case k_iSteamAppsCallbacks:
        PRINT_DEBUG("k_iSteamAppsCallbacks %i", callback_id);
        break;

    case k_iSteamUserStatsCallbacks:
        PRINT_DEBUG("k_iSteamUserStatsCallbacks %i", callback_id);
        break;

    case k_iSteamNetworkingCallbacks:
        PRINT_DEBUG("k_iSteamNetworkingCallbacks %i", callback_id);
        break;

    case k_iClientRemoteStorageCallbacks:
        PRINT_DEBUG("k_iClientRemoteStorageCallbacks %i", callback_id);
        break;

    case k_iClientDepotBuilderCallbacks:
        PRINT_DEBUG("k_iClientDepotBuilderCallbacks %i", callback_id);
        break;

    case k_iSteamGameServerItemsCallbacks:
        PRINT_DEBUG("k_iSteamGameServerItemsCallbacks %i", callback_id);
        break;

    case k_iClientUtilsCallbacks:
        PRINT_DEBUG("k_iClientUtilsCallbacks %i", callback_id);
        break;

    case k_iSteamGameCoordinatorCallbacks:
        PRINT_DEBUG("k_iSteamGameCoordinatorCallbacks %i", callback_id);
        break;

    case k_iSteamGameServerStatsCallbacks:
        PRINT_DEBUG("k_iSteamGameServerStatsCallbacks %i", callback_id);
        break;

    case k_iSteam2AsyncCallbacks:
        PRINT_DEBUG("k_iSteam2AsyncCallbacks %i", callback_id);
        break;

    case k_iSteamGameStatsCallbacks:
        PRINT_DEBUG("k_iSteamGameStatsCallbacks %i", callback_id);
        break;

    case k_iClientHTTPCallbacks:
        PRINT_DEBUG("k_iClientHTTPCallbacks %i", callback_id);
        break;

    case k_iClientScreenshotsCallbacks:
        PRINT_DEBUG("k_iClientScreenshotsCallbacks %i", callback_id);
        break;

    case k_iSteamScreenshotsCallbacks:
        PRINT_DEBUG("k_iSteamScreenshotsCallbacks %i", callback_id);
        break;

    case k_iClientAudioCallbacks:
        PRINT_DEBUG("k_iClientAudioCallbacks %i", callback_id);
        break;

    case k_iClientUnifiedMessagesCallbacks:
        PRINT_DEBUG("k_iClientUnifiedMessagesCallbacks %i", callback_id);
        break;

    case k_iSteamStreamLauncherCallbacks:
        PRINT_DEBUG("k_iSteamStreamLauncherCallbacks %i", callback_id);
        break;

    case k_iClientControllerCallbacks:
        PRINT_DEBUG("k_iClientControllerCallbacks %i", callback_id);
        break;

    case k_iSteamControllerCallbacks:
        PRINT_DEBUG("k_iSteamControllerCallbacks %i", callback_id);
        break;

    case k_iClientParentalSettingsCallbacks:
        PRINT_DEBUG("k_iClientParentalSettingsCallbacks %i", callback_id);
        break;

    case k_iClientDeviceAuthCallbacks:
        PRINT_DEBUG("k_iClientDeviceAuthCallbacks %i", callback_id);
        break;

    case k_iClientNetworkDeviceManagerCallbacks:
        PRINT_DEBUG("k_iClientNetworkDeviceManagerCallbacks %i", callback_id);
        break;

    case k_iClientMusicCallbacks:
        PRINT_DEBUG("k_iClientMusicCallbacks %i", callback_id);
        break;

    case k_iClientRemoteClientManagerCallbacks:
        PRINT_DEBUG("k_iClientRemoteClientManagerCallbacks %i", callback_id);
        break;

    case k_iClientUGCCallbacks:
        PRINT_DEBUG("k_iClientUGCCallbacks %i", callback_id);
        break;

    case k_iSteamStreamClientCallbacks:
        PRINT_DEBUG("k_iSteamStreamClientCallbacks %i", callback_id);
        break;

    case k_IClientProductBuilderCallbacks:
        PRINT_DEBUG("k_IClientProductBuilderCallbacks %i", callback_id);
        break;

    case k_iClientShortcutsCallbacks:
        PRINT_DEBUG("k_iClientShortcutsCallbacks %i", callback_id);
        break;

    case k_iClientRemoteControlManagerCallbacks:
        PRINT_DEBUG("k_iClientRemoteControlManagerCallbacks %i", callback_id);
        break;

    case k_iSteamAppListCallbacks:
        PRINT_DEBUG("k_iSteamAppListCallbacks %i", callback_id);
        break;

    case k_iSteamMusicCallbacks:
        PRINT_DEBUG("k_iSteamMusicCallbacks %i", callback_id);
        break;

    case k_iSteamMusicRemoteCallbacks:
        PRINT_DEBUG("k_iSteamMusicRemoteCallbacks %i", callback_id);
        break;

    case k_iClientVRCallbacks:
        PRINT_DEBUG("k_iClientVRCallbacks %i", callback_id);
        break;

    case k_iSteamHTMLSurfaceCallbacks:
        PRINT_DEBUG("k_iSteamHTMLSurfaceCallbacks %i", callback_id);
        break;

    case k_iClientVideoCallbacks:
        PRINT_DEBUG("k_iClientVideoCallbacks %i", callback_id);
        break;

    case k_iClientInventoryCallbacks:
        PRINT_DEBUG("k_iClientInventoryCallbacks %i", callback_id);
        break;

    default:
        PRINT_DEBUG("Unknown callback base %i", base_callback);
    };

    if (isGameServer) {
        callbacks_server->AddCallback(iCallback, callback_to_add);
    }
    else {
        callbacks_client->AddCallback(iCallback, callback_to_add);
    }
}

void CSteamClient::UnregisterCallback(class CCallbackBase* pCallback)
{
    CCallbackBase* callback_to_rm = pCallback;

    int iCallback = callback_to_rm->GetICallback();
    int base_callback = (iCallback / 100) * 100;
    int callback_id = iCallback % 100;
    bool isGameServer = CCallbackMgr::IsServer(callback_to_rm);
    PRINT_DEBUG("isGameServer %u %i", isGameServer, base_callback);

    switch (base_callback) {
    case k_iSteamUserCallbacks:
        PRINT_DEBUG("k_iSteamUserCallbacks %i", callback_id);
        break;

    case k_iSteamGameServerCallbacks:
        PRINT_DEBUG("k_iSteamGameServerCallbacks %i", callback_id);
        break;

    case k_iSteamFriendsCallbacks:
        PRINT_DEBUG("k_iSteamFriendsCallbacks %i", callback_id);
        break;

    case k_iSteamBillingCallbacks:
        PRINT_DEBUG("k_iSteamBillingCallbacks %i", callback_id);
        break;

    case k_iSteamMatchmakingCallbacks:
        PRINT_DEBUG("k_iSteamMatchmakingCallbacks %i", callback_id);
        break;

    case k_iSteamContentServerCallbacks:
        PRINT_DEBUG("k_iSteamContentServerCallbacks %i", callback_id);
        break;

    case k_iSteamUtilsCallbacks:
        PRINT_DEBUG("k_iSteamUtilsCallbacks %i", callback_id);
        break;

    case k_iClientFriendsCallbacks:
        PRINT_DEBUG("k_iClientFriendsCallbacks %i", callback_id);
        break;

    case k_iClientUserCallbacks:
        PRINT_DEBUG("k_iClientUserCallbacks %i", callback_id);
        break;

    case k_iSteamAppsCallbacks:
        PRINT_DEBUG("k_iSteamAppsCallbacks %i", callback_id);
        break;

    case k_iSteamUserStatsCallbacks:
        PRINT_DEBUG("k_iSteamUserStatsCallbacks %i", callback_id);
        break;

    case k_iSteamNetworkingCallbacks:
        PRINT_DEBUG("k_iSteamNetworkingCallbacks %i", callback_id);
        break;

    case k_iClientRemoteStorageCallbacks:
        PRINT_DEBUG("k_iClientRemoteStorageCallbacks %i", callback_id);
        break;

    case k_iClientDepotBuilderCallbacks:
        PRINT_DEBUG("k_iClientDepotBuilderCallbacks %i", callback_id);
        break;

    case k_iSteamGameServerItemsCallbacks:
        PRINT_DEBUG("k_iSteamGameServerItemsCallbacks %i", callback_id);
        break;

    case k_iClientUtilsCallbacks:
        PRINT_DEBUG("k_iClientUtilsCallbacks %i", callback_id);
        break;

    case k_iSteamGameCoordinatorCallbacks:
        PRINT_DEBUG("k_iSteamGameCoordinatorCallbacks %i", callback_id);
        break;

    case k_iSteamGameServerStatsCallbacks:
        PRINT_DEBUG("k_iSteamGameServerStatsCallbacks %i", callback_id);
        break;

    case k_iSteam2AsyncCallbacks:
        PRINT_DEBUG("k_iSteam2AsyncCallbacks %i", callback_id);
        break;

    case k_iSteamGameStatsCallbacks:
        PRINT_DEBUG("k_iSteamGameStatsCallbacks %i", callback_id);
        break;

    case k_iClientHTTPCallbacks:
        PRINT_DEBUG("k_iClientHTTPCallbacks %i", callback_id);
        break;

    case k_iClientScreenshotsCallbacks:
        PRINT_DEBUG("k_iClientScreenshotsCallbacks %i", callback_id);
        break;

    case k_iSteamScreenshotsCallbacks:
        PRINT_DEBUG("k_iSteamScreenshotsCallbacks %i", callback_id);
        break;

    case k_iClientAudioCallbacks:
        PRINT_DEBUG("k_iClientAudioCallbacks %i", callback_id);
        break;

    case k_iClientUnifiedMessagesCallbacks:
        PRINT_DEBUG("k_iClientUnifiedMessagesCallbacks %i", callback_id);
        break;

    case k_iSteamStreamLauncherCallbacks:
        PRINT_DEBUG("k_iSteamStreamLauncherCallbacks %i", callback_id);
        break;

    case k_iClientControllerCallbacks:
        PRINT_DEBUG("k_iClientControllerCallbacks %i", callback_id);
        break;

    case k_iSteamControllerCallbacks:
        PRINT_DEBUG("k_iSteamControllerCallbacks %i", callback_id);
        break;

    case k_iClientParentalSettingsCallbacks:
        PRINT_DEBUG("k_iClientParentalSettingsCallbacks %i", callback_id);
        break;

    case k_iClientDeviceAuthCallbacks:
        PRINT_DEBUG("k_iClientDeviceAuthCallbacks %i", callback_id);
        break;

    case k_iClientNetworkDeviceManagerCallbacks:
        PRINT_DEBUG("k_iClientNetworkDeviceManagerCallbacks %i", callback_id);
        break;

    case k_iClientMusicCallbacks:
        PRINT_DEBUG("k_iClientMusicCallbacks %i", callback_id);
        break;

    case k_iClientRemoteClientManagerCallbacks:
        PRINT_DEBUG("k_iClientRemoteClientManagerCallbacks %i", callback_id);
        break;

    case k_iClientUGCCallbacks:
        PRINT_DEBUG("k_iClientUGCCallbacks %i", callback_id);
        break;

    case k_iSteamStreamClientCallbacks:
        PRINT_DEBUG("k_iSteamStreamClientCallbacks %i", callback_id);
        break;

    case k_IClientProductBuilderCallbacks:
        PRINT_DEBUG("k_IClientProductBuilderCallbacks %i", callback_id);
        break;

    case k_iClientShortcutsCallbacks:
        PRINT_DEBUG("k_iClientShortcutsCallbacks %i", callback_id);
        break;

    case k_iClientRemoteControlManagerCallbacks:
        PRINT_DEBUG("k_iClientRemoteControlManagerCallbacks %i", callback_id);
        break;

    case k_iSteamAppListCallbacks:
        PRINT_DEBUG("k_iSteamAppListCallbacks %i", callback_id);
        break;

    case k_iSteamMusicCallbacks:
        PRINT_DEBUG("k_iSteamMusicCallbacks %i", callback_id);
        break;

    case k_iSteamMusicRemoteCallbacks:
        PRINT_DEBUG("k_iSteamMusicRemoteCallbacks %i", callback_id);
        break;

    case k_iClientVRCallbacks:
        PRINT_DEBUG("k_iClientVRCallbacks %i", callback_id);
        break;

    case k_iSteamHTMLSurfaceCallbacks:
        PRINT_DEBUG("k_iSteamHTMLSurfaceCallbacks %i", callback_id);
        break;

    case k_iClientVideoCallbacks:
        PRINT_DEBUG("k_iClientVideoCallbacks %i", callback_id);
        break;

    case k_iClientInventoryCallbacks:
        PRINT_DEBUG("k_iClientInventoryCallbacks %i", callback_id);
        break;

    default:
        PRINT_DEBUG("Unknown callback base %i", base_callback);
    };

    if (isGameServer) {
        callbacks_server->RemoveCallback(iCallback, callback_to_rm);
    }
    else {
        callbacks_client->RemoveCallback(iCallback, callback_to_rm);
    }
}

void CSteamClient::RegisterCallResult(class CCallbackBase* pCallback, SteamAPICall_t hAPICall)
{
    PRINT_DEBUG("%llu %i", hAPICall, pCallback->GetICallback());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    callback_results_client->AddCallback(hAPICall, pCallback);
    callback_results_server->AddCallback(hAPICall, pCallback);

}

void CSteamClient::UnregisterCallResult(class CCallbackBase* pCallback, SteamAPICall_t hAPICall)
{
    PRINT_DEBUG("%llu %i", hAPICall, pCallback->GetICallback());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    callback_results_client->RemoveCallback(hAPICall, pCallback);
    callback_results_server->RemoveCallback(hAPICall, pCallback);
}

void CSteamClient::RunCallbacks(bool runClientCB, bool runGameserverCB)
{
    PRINT_DEBUG("begin ------------------------------------------------------");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    cb_run_active = true;

    // PRINT_DEBUG("steam_matchmaking_servers *********");
    g_pSteamMatchMakingServers->RunCallbacks();

    // PRINT_DEBUG("run_every_runcb *********");
    run_every_runcb->Run();

    // PRINT_DEBUG("steam_gameserver *********");
    g_pSteamGameServer->RunCallbacks();

    // PRINT_DEBUG("steam_user *********");
    g_pSteamUser->RunCallbacks();

    if (runClientCB && g_pSteamUser->BLoggedOn()) {
        // PRINT_DEBUG("callback_results_client *********");
        callback_results_client->RunCallResults();
    }

    if (runGameserverCB && g_pSteamGameServer->BLoggedOn()) {
        // PRINT_DEBUG("callback_results_server *********");
        callback_results_server->RunCallResults();
    }

    // PRINT_DEBUG("callbacks_server *********");
    callbacks_server->RunCallbacks();

    // PRINT_DEBUG("callbacks_client *********");
    callbacks_client->RunCallbacks();

    last_cb_run = (unsigned long long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    cb_run_active = false;
    PRINT_DEBUG("done ******************************************************");
}
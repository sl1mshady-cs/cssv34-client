#include <winlite.h>
#include "tier0/basetypes.h"
#include "tier1/interface.h"

#ifdef _WIN32
// Define necessary undocumented NT types and enums
typedef long NTSTATUS;

enum SYSTEM_INFORMATION_CLASS {
    SystemMemoryListInformation = 80 // The class used to manage memory lists
};

enum SYSTEM_MEMORY_LIST_COMMAND {
    MemoryCaptureState,
    MemoryPurgeStandbyList,        // Command to purge the standby list
    MemoryPurgeLowPriorityStandbyList,
    MemoryCommandMax
};

// Helper function to enable privileges
bool SetPrivilege(void* hToken, const char* lpszPrivilege, bool bEnablePrivilege) {
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid)) return false;

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = bEnablePrivilege ? SE_PRIVILEGE_ENABLED : 0;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) return false;
    return GetLastError() != ERROR_NOT_ALL_ASSIGNED;
}

// NtSetSystemInformation function
NTSTATUS __stdcall NtSetSystemInformation(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    LPVOID SystemInformation,
    ULONG SystemInformationLength
) 
{
    // Load ntdll.dll and find NtSetSystemInformation
    HMODULE hNtDll = GetModuleHandle("ntdll.dll");
    if (!hNtDll) return 0xC0000135; // STATUS_DLL_NOT_FOUND

    auto pfnNtSetSystemInformation = (decltype(NtSetSystemInformation)*)GetProcAddress(hNtDll, "NtSetSystemInformation");
    if (!pfnNtSetSystemInformation) return 0xC0000139; // STATUS_ENTRYPOINT_NOT_FOUND
    
    return pfnNtSetSystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength);
}

int ClearStandbyList() 
{
    void* hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return -1;
    }

    // SE_INCREASE_QUOTA_NAME is required to modify system memory parameters
    if (!SetPrivilege(hToken, SE_INCREASE_QUOTA_NAME, true)) {
        CloseHandle(hToken);
        return -2;
    }

    CloseHandle(hToken);

    // Load ntdll.dll and find NtSetSystemInformation
    SYSTEM_MEMORY_LIST_COMMAND command = MemoryPurgeStandbyList;
    NTSTATUS status = NtSetSystemInformation(SystemMemoryListInformation, &command, sizeof(command));

    return status;
}

#endif
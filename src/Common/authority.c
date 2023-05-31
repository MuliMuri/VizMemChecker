#include <Windows.h>

#include "common.h"
#include "win_nt_def.h"

#include "authority.h"

// Try to get the debug privilege
HKSTATUS EnableDebugPrivilege()
{
    HKSTATUS status         = HK_STATUS_SUCCESS;

    HANDLE hToken           = NULL;
    LUID debugLuid          = {0};
    TOKEN_PRIVILEGES tkp    = { 0 };

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return HK_STATUS_FATAL;

    if (!LookupPrivilegeValueA(NULL, SE_DEBUG_NAME, &debugLuid))
        return HK_STATUS_FATAL;

    tkp.PrivilegeCount              = 1;
    tkp.Privileges[0].Luid          = debugLuid;
    tkp.Privileges[0].Attributes    = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
    {
        CloseHandle(hToken);
        return HK_STATUS_FATAL;
    }

    return status;
}

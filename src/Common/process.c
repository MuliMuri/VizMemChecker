#include <Windows.h>
#include <psapi.h>

#include "common.h"
#include "win_nt_def.h"

#include "process.h"

HKSTATUS PROC_FindProcessByName(CHAR *processName, PROCESS_INFO **processInfo)
{
    HKSTATUS status = HK_STATUS_SUCCESS;

    DWORD processes[1024];
    DWORD needed;
    DWORD processCount;
    HANDLE hProcess;
    CHAR szProcessName[MAX_PATH];

    if (!*processInfo)
        return HK_STATUS_FATAL;

    if (!EnumProcesses(processes, sizeof(processes), &needed))
        return 0;

    processCount = needed / sizeof(DWORD);

    for (DWORD i = 0; i < processCount; i++)
    {
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processes[i]);
        if (!hProcess)
            continue;

        if (!GetModuleBaseNameA(hProcess, NULL, szProcessName, sizeof(szProcessName)))
            continue;

        if (!strcmp(szProcessName, processName))
        {
            (*processInfo)->ProcessHandle = hProcess;
            (*processInfo)->ProcessId = processes[i];
            strcpy_s((*processInfo)->ProcessName, strlen(szProcessName) + 1, szProcessName);

            break;
        }
        else
        {
            CloseHandle(hProcess);
        }        
    }

    return status;
}

HKSTATUS PROC_FindProcessById(DWORD pid, PROCESS_INFO **processInfo)
{
    HKSTATUS status = HK_STATUS_SUCCESS;

    HANDLE hProcess;
    CHAR szProcessName[MAX_PATH];

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess)
        return HK_STATUS_FATAL;

    if (!GetModuleBaseNameA(hProcess, NULL, szProcessName, sizeof(szProcessName)))
        return HK_STATUS_FATAL;

    (*processInfo)->ProcessHandle = hProcess;
    (*processInfo)->ProcessId = pid;
    strcpy_s((*processInfo)->ProcessName, strlen(szProcessName) + 1, szProcessName);

    return status;
}

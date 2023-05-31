#include <Windows.h>

#include "common/common.h"
#include "common/win_nt_def.h"

#include "toolkit.h"

HKSTATUS TOOL_Initialize()
{
    HKSTATUS status = HK_STATUS_SUCCESS;

    g_toolHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0x100, 0x1000);
    if (!g_toolHeap)
        return HK_STATUS_FATAL;

    return status;
}

// HKSTATUS TOOL_RunProgram(WCHAR *fileFullPath, PROCESS_INFO *processInfo)
// {
//     HKSTATUS status = HK_STATUS_SUCCESS;

//     if (!processInfo)
//         return HK_STATUS_FATAL;

//     STARTUPINFOW* startupInfo = (STARTUPINFOW*)HeapAlloc(g_toolHeap, HEAP_ZERO_MEMORY, sizeof(STARTUPINFOW));
//     PROCESS_INFORMATION *processInfo = (PROCESS_INFORMATION *)HeapAlloc(g_toolHeap, HEAP_ZERO_MEMORY, sizeof(PROCESS_INFORMATION));

//     if (!CreateProcessW(NULL, fileFullPath, NULL, NULL, FALSE, DEBUG_PROCESS, NULL, NULL, startupInfo, processInfo))
//         return HK_STATUS_FATAL;

//     SuspendThread(processInfo->hThread);

//     progInfo->ProcessInfo = processInfo;
//     progInfo->StartUpInfo = startupInfo;

//     return status;
// }

// TOOL_AttachProgram(WCHAR *processName,  *progInfo)
// {

// }

/*
* In VizMemoryChecker, it should inject target, to run `injector.dll`
* Target need to connect main(this) by pipe
*/
HKSTATUS TOOL_InjectDll(DWORD pid, CHAR *dllName)
{
    HKSTATUS status = HK_STATUS_SUCCESS;

    HANDLE hProcess = NULL;
    SIZE_T size = 0;
    BYTE *pRemoteDllName = NULL;

    HMODULE hKrnl = NULL;
    PTHREAD_START_ROUTINE pfnStartAddr = NULL;

    HANDLE hRemoteThread = NULL;

    // Calc length of dllName
    size = (strlen(dllName) + 1);
    if (size <= 1)
        return HK_STATUS_FATAL;

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == INVALID_HANDLE_VALUE)
        return HK_STATUS_FATAL;

    pRemoteDllName = (BYTE *)VirtualAllocEx(hProcess, NULL, size, MEM_COMMIT, PAGE_READWRITE);
    if (!pRemoteDllName)
    {
        CloseHandle(hProcess);

        return HK_STATUS_FATAL;
    }

    if (!WriteProcessMemory(hProcess, pRemoteDllName, dllName, size, NULL))
    {
        CloseHandle(hProcess);
        VirtualFreeEx(hProcess, pRemoteDllName, size, MEM_DECOMMIT);

        return HK_STATUS_FATAL;
    }

    hKrnl = GetModuleHandleA("Kernel32");
    if (!hKrnl)
    {
        CloseHandle(hProcess);
        VirtualFreeEx(hProcess, pRemoteDllName, size, MEM_DECOMMIT);

        return HK_STATUS_FATAL;
    }

    pfnStartAddr = (PTHREAD_START_ROUTINE)GetProcAddress(hKrnl, "LoadLibraryA");
    if (!pfnStartAddr)
    {
        int a = GetLastError();
        CloseHandle(hProcess);
        VirtualFreeEx(hProcess, pRemoteDllName, size, MEM_DECOMMIT);

        return HK_STATUS_FATAL;
    }

    hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, pfnStartAddr, pRemoteDllName, 0, NULL);
    if (!hRemoteThread)
    {
        int a = GetLastError();
        CloseHandle(hProcess);
        VirtualFreeEx(hProcess, pRemoteDllName, size, MEM_DECOMMIT);

        return HK_STATUS_FATAL;
    }


    CloseHandle(hProcess);
    return status;
}

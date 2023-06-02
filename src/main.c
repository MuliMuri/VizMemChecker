#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Windows.h>

#include "argtable3/argtable3.h"

#include "common/common.h"
#include "common/win_nt_def.h"
#include "common/authority.h"
#include "common/process.h"

#include "core/toolkit.h"

#include "utils/logprint.h"

PROCESS_INFO *g_processInfo = NULL;

int main(int argc, char* argv[])
{
	char *filePath = NULL;

	struct arg_int *detachPid = arg_int0("d", "detach", "<pid>", "Detach a process by pid");
	struct arg_str *detachStr = arg_str0("f", "find", "<process_name>", "Detach a process by its name");
	struct arg_end *end = arg_end(20);

	void *argtable[] = {
		detachPid,
		detachStr,
		end,
	};

	uint8_t nerrors = arg_parse(argc, argv, argtable);
	if (nerrors > 0)
	{
		arg_print_errors(stderr, end, argv[0]);
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return -1;
	}

	// Get execute program path
	// i=1 to skip this(self) program command
	for (uint8_t i = 1; i < argc; i++)
	{
		if (argv[i][0] != '-')
		{
			// Not argtable args
			// Just filePath
			filePath = argv[i];
			break;
		}
	}

	// ========================================== //
	// ========== Initialize arguments ========== //
	// ========================================== //

	// Check arguments
	if (detachStr->count >0 && detachPid->count > 0)
	{
		printf(COLOR_YELLOW("[!] You cannot specify both ") COLOR_PINK("%s") COLOR_YELLOW(" and ") COLOR_PINK("%s") COLOR_YELLOW(" at the same time.\r\n"), detachPid->hdr.datatype, detachStr->hdr.datatype);
		return -1;
	}

	if (detachStr->count == 0 && detachPid->count == 0 && filePath == NULL)
	{
		printf(COLOR_YELLOW("[!] Please specify process by ") COLOR_PINK("%s") COLOR_YELLOW(" , ") COLOR_PINK("%s") COLOR_YELLOW(" or program path.\r\n"), detachPid->hdr.datatype, detachStr->hdr.datatype);
		return -1;
	}

	g_processInfo = malloc(sizeof(PROCESS_INFO));
	RtlZeroMemory(g_processInfo, sizeof(PROCESS_INFO));

	// Use <process_name>
	if (detachStr->count > 0)
	{
		PROC_FindProcessByName(*detachStr->sval, &g_processInfo);
		if (!g_processInfo->ProcessHandle)
		{
			printf(COLOR_YELLOW("[!] Cannot found the ") COLOR_PINK("%s") COLOR_YELLOW(" process, please check it.\r\n"), *detachStr->sval);
			return -1;
		}
	}

	// Use <pid>
	if (detachPid->count > 0)
	{
		PROC_FindProcessById(*detachPid->ival, &g_processInfo);
		if (!g_processInfo->ProcessHandle)
		{
			printf(COLOR_YELLOW("[!] Cannot found the ") COLOR_PINK("%d") COLOR_YELLOW(" process, please check it.\r\n"), *detachPid->ival);
			return -1;
		}
	}

	printf(COLOR_GREEN("[+] Process: ") COLOR_PINK("<%s : %d>\r\n"), g_processInfo->ProcessName, g_processInfo->ProcessId);

	// ========================================== //
	// ========= Initialize environment ========= //
	// ========================================== //

	TOOL_Initialize();
	EnableDebugPrivilege();

	TOOL_InjectDll(g_processInfo->ProcessId, "G:\\SystemTools\\VizMemChecker\\build\\src\\injector\\Debug\\injector.dll");

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}































// #include <Windows.h>
// #include <tlhelp32.h>
// #include <stdio.h>

// typedef SHORT(__stdcall *HK_Initialize)(DWORD pid);
// typedef SHORT(__stdcall *HK_AppendHookNode)(WCHAR* FileName, WCHAR* FuncName);
// typedef SHORT(__stdcall *HK_EnableOnceHook)(WCHAR* FileName, WCHAR* FuncName);

// int GetProcessIdByName(const char* processName) {
//     int pid = -1;
//     HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

//     if (hSnapshot != INVALID_HANDLE_VALUE) {
//         PROCESSENTRY32 pe32;
//         pe32.dwSize = sizeof(PROCESSENTRY32);

//         if (Process32First(hSnapshot, &pe32)) {
//             do {
//                 if (strcmp(pe32.szExeFile, processName) == 0) {
//                     pid = pe32.th32ProcessID;
//                     break;
//                 }
//             } while (Process32Next(hSnapshot, &pe32));
//         }

//         CloseHandle(hSnapshot);
//     }

//     return pid;
// }

// int main(void)
// {
// 	/*
// 	*         [DllImport("HookDLL.dll")]
//         private extern static short HK_Initialize(int pid);

//         [DllImport("HookDLL.dll", CharSet = CharSet.Unicode)]
//         private extern static short HK_AppendHookNode(string FileName, string FuncName);

//         [DllImport("HookDLL.dll", CharSet = CharSet.Unicode)]
//         private extern static short HK_RemoveHookNode(string FileName, string FuncName);

//         [DllImport("HookDLL.dll", CharSet = CharSet.Unicode)]
//         private extern static short HK_EnableOnceHook(string FileName, string FuncName);




//         public MainWin()
//         {
//             InitializeComponent();
//         }

//         private void button1_Click(object sender, EventArgs e)
//         {
//             int pid = Process.GetProcessesByName("notepad")[0].Id;





//             HK_Initialize(pid);
//             HK_AppendHookNode("Kernel32.dll", "VirtualAlloc");
//             HK_EnableOnceHook("Kernel32.dll", "VirtualAlloc");
//         }
// 	*/

//     HMODULE lib = LoadLibraryW(L"HookDLL.dll");
//     HK_Initialize hkinit = (HK_Initialize)GetProcAddress(lib, "HK_Initialize");
//     HK_AppendHookNode hkapd = (HK_AppendHookNode)GetProcAddress(lib, "HK_AppendHookNode");
//     HK_EnableOnceHook hkenb = (HK_EnableOnceHook)GetProcAddress(lib, "HK_EnableOnceHook");


//     while (1)
//     {
//         DWORD pid = GetProcessIdByName("notepad.exe");
//         scanf("%d", &pid);
//         hkinit(pid);
//         hkapd(L"Kernel32.dll", L"HeapCreate");
//         hkenb(L"Kernel32.dll", L"HeapCreate");

//         system("pause");

//     }

// 	return 0;
// }

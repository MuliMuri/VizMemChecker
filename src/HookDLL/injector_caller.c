#include <Windows.h>

#include "common.h"
#include "win_nt_def.h"
#include "injector_caller.h"

HKSTATUS _Pipe_Initialize(DWORD pid)
{
	WCHAR pipeName[64] = INJTOR_PIPE_NAME;
	WCHAR wcPid[8] = { 0 };

	wsprintfW(wcPid, L"%d", pid);
	wcscat(pipeName, wcPid);

	if (!WaitNamedPipeW(pipeName, NMPWAIT_WAIT_FOREVER))
		return HK_STATUS_FATAL;

	g_pipe = CreateFileW(pipeName, GENERIC_WRITE | GENERIC_READ, FALSE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	// bugs?
}

HKSTATUS CALLER_Initialize(DWORD pid)
{
	_Pipe_Initialize(pid);
}

HKSTATUS CALLER_EnableHook(PHOOK_NODE hookNode)
{
	DWORD size = 0;
	CALLER_COMMAND cmd = { 0 };
	cmd.Command = COMMAND_ENABLE_HOOK;
	cmd.CommandUnion.HookNode = *hookNode;

	WriteFile(g_pipe, &cmd, sizeof(CALLER_COMMAND), &size, NULL);
}

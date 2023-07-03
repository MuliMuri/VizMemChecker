#include <Windows.h>

#include "common/common.h"
#include "common/win_nt_def.h"
#include "code_decoder.h"
#include "injector_caller.h"

HKSTATUS _Pipe_Initialize(DWORD pid)
{
	WCHAR pipeName[64] = INJTOR_PIPE_NAME;
	WCHAR wcPid[8] = { 0 };

	wsprintfW(wcPid, L"%d", pid);
	wcscat(pipeName, wcPid);

	for (size_t i = 0; i < 5; i++)
	{
		Sleep(1000);
		if (!WaitNamedPipeW(pipeName, NMPWAIT_USE_DEFAULT_WAIT))
		{
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
				continue;
			
			return HK_STATUS_FATAL;
		}
		else break;
	}	

	g_pipe = CreateFileW(pipeName, GENERIC_WRITE | GENERIC_READ, FALSE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

HKSTATUS CALLER_Initialize(DWORD pid)
{
	_Pipe_Initialize(pid);
}

HKSTATUS CALLER_EnableHook(PHOOK_NODE hookNode)
{
	DWORD size = 0;
	CALLER_COMMAND cmd = { 0 };
	INSN_LIST insnList = { 0 };
	USHORT codeSize = 0;

	cmd.Command = COMMAND_HOOK_ENABLE;
	cmd.Context.HookNode = *hookNode;

	WriteFile(g_pipe, &cmd, sizeof(CALLER_COMMAND), &size, NULL);
	if (!ReadFile(g_pipe, &cmd, sizeof(CALLER_COMMAND), &size, NULL))
		return HK_STATUS_FATAL;

	BYTE* asmCode = cmd.Context.AsmCode;
	DECODER_Decode(asmCode, size, &insnList);		// This size must greater than real size of asmCode, but we just need first few asmCodes' size
	
	for (size_t i = 0; i < insnList.Count; i++)
	{
		codeSize += insnList.Context[i].Size;
		if (codeSize >= HOOK_STUB_LENGTH)
			break;
	}

	cmd.Context.HookSize = codeSize;
	WriteFile(g_pipe, &cmd, sizeof(CALLER_COMMAND), &size, NULL);

	return HK_STATUS_SUCCESS;
}

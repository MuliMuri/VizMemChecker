#include <Windows.h>

#include "common.h"
#include "win_nt_def.h"
#include "injector.h"

/*
* 
* 
*		==WARRING====WARRING====WARRING====WARRING====WARRING====WARRING==
* 
*		This function need to run in debug's program
*		Need to inject into program
* 
*		==WARRING====WARRING====WARRING====WARRING====WARRING====WARRING==
* 
*/

CHAR* _TryFindMatchFuncAddr(WCHAR* FileName, WCHAR* FuncName)
{
	CHAR funcName[64] = { 0 };

	HMODULE hDll = LoadLibraryW(FileName);
	if (!hDll)
		return NULL;

	wcstombs(&funcName, FuncName, 64);
	FARPROC func = GetProcAddress(hDll, funcName);

	return (CHAR*)func;
}

HKSTATUS _CreateDebuggerByPipe(DWORD pid)
{
	DebugBreak();

	WCHAR pipeName[64] = INJTOR_PIPE_NAME;
	WCHAR wcPid[8] = { 0 };

	wsprintfW(wcPid, L"%d", pid);
	wcscat(pipeName, wcPid);

	g_pipe = CreateNamedPipeW(pipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_WAIT, 1, 0, 0, 0, NULL);
	if (g_pipe == INVALID_HANDLE_VALUE)
		return HK_STATUS_FATAL;

	return HK_STATUS_SUCCESS;
}

HKSTATUS _TaskLoop()
{
	g_buffer = HeapAlloc(g_hookNodeHeap, 0, BUFFER_LENGTH);
	DWORD size = 0;
	HKSTATUS status = HK_STATUS_SUCCESS;

	ConnectNamedPipe(g_pipe, NULL);	// Wait for client connected.

	while (TRUE)
	{
		// :Use blocking IO
		if (!ReadFile(g_pipe, g_buffer, BUFFER_LENGTH, &size, NULL) || g_buffer == NULL)
			return HK_STATUS_FATAL;

		CALLER_COMMAND* cmdStruct = (CALLER_COMMAND*)g_buffer;

		switch (cmdStruct->Command)
		{
		case COMMAND_ENABLE_HOOK:
			status = INJTOR_EnableHook(&cmdStruct->CommandUnion.HookNode);

			break;
		case COMMAND_DISABLE_HOOK:
			status = INJTOR_DisableHook(&cmdStruct->CommandUnion.HookNode);

			break;
		default:
			break;
		}

		// CallBack
		if (status != HK_STATUS_SUCCESS)
			cmdStruct->Command = COMMAND_ERR;

		if (!WriteFile(g_pipe, g_buffer, BUFFER_LENGTH, &size, NULL))
			break;/*return HK_STATUS_FATAL;*/
	}
}

HKSTATUS _Instruction_Initialize()
{
	g_instLengthTable = HeapAlloc(g_runtimeHeap, 0, sizeof(INSTRUCTION) * 0xFF);
	if (!g_instLengthTable)
		return HK_STATUS_FATAL;

	/*
	* 0x0F: 扩展指令集 SSE/AVX等
	* 0x66: 操作数大小覆盖前缀
	* 0x67: 地址大小覆盖前缀
	* 0xF0: LOCK前缀
	* 0xF2: REPNE/REPNZ前缀
	* 0xF3: REP/REPE/REPZ前缀
	* 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65: 分段寻址前缀
	*/

	g_instLengthTable[0x0F].Opcode = 0x0F;
	g_instLengthTable[0x0F].Length = 1;

	g_instLengthTable[0x66].Opcode = 0x66;
	g_instLengthTable[0x66].Length = 1;

	g_instLengthTable[0x67].Opcode = 0x67;
	g_instLengthTable[0x67].Length = 1;

	g_instLengthTable[0xF0].Opcode = 0xF0;
	g_instLengthTable[0xF0].Length = 1;

	g_instLengthTable[0xF2].Opcode = 0xF2;
	g_instLengthTable[0xF2].Length = 1;

	g_instLengthTable[0xF3].Opcode = 0xF3;
	g_instLengthTable[0xF3].Length = 1;

	g_instLengthTable[0x2E].Opcode = 0x2E;
	g_instLengthTable[0x2E].Length = 1;

	g_instLengthTable[0x36].Opcode = 0x36;
	g_instLengthTable[0x36].Length = 1;

	g_instLengthTable[0x3E].Opcode = 0x3E;
	g_instLengthTable[0x3E].Length = 1;

	g_instLengthTable[0x26].Opcode = 0x26;
	g_instLengthTable[0x26].Length = 1;

	g_instLengthTable[0x64].Opcode = 0x64;
	g_instLengthTable[0x64].Length = 1;

	g_instLengthTable[0x65].Opcode = 0x65;
	g_instLengthTable[0x65].Length = 1;
}

// EntryPoint
// EntryPoint
// EntryPoint
HKSTATUS INJTOR_Initialize()
{
	HKSTATUS status = HK_STATUS_SUCCESS;

	g_runtimeHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0x1000, 0x10000);
	if (!g_runtimeHeap)
		return HK_STATUS_FATAL;

	g_hookNodeHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0x1000, 0x10000);
	if (!g_hookNodeHeap)
	{
		HeapDestroy(g_runtimeHeap);
		return HK_STATUS_FATAL;
	}

	DWORD pid = GetCurrentProcessId();

	status = _CreateDebuggerByPipe(pid);
	if (status != HK_STATUS_SUCCESS)
		return status;

	status = _TaskLoop();


	return status;
}

/*
*	Hook process
* 
*	1. Copy something source byte code
*	2. Edit to jmp to handler function
*	3. Run handler
*	4. Recover change
*	5. Jmp back to source
* 
*/

HKSTATUS INJTOR_EnableHook(PHOOK_NODE hookInfo)
{
	DebugBreak();

	WCHAR* fileName = &hookInfo->Data[hookInfo->Match.FileNameOffset];
	CHAR* funcName = &hookInfo->Data[hookInfo->Match.FuncNameOffset];
	CHAR code[5] = { 0xE9 };

	if (!hookInfo->HookAddress)
		hookInfo->HookAddress = _TryFindMatchFuncAddr(fileName, funcName);

	if (!hookInfo->HandlerAddress)
		hookInfo->HandlerAddress = &HANDLER_CountFuncion;

	if (!hookInfo->HookAddress || !hookInfo->HandlerAddress)
		return HK_STATUS_FATAL;

	CHAR* rawCode = &hookInfo->Data[hookInfo->HookFuncRawCodeOffset];
	CHAR* hookAddr = hookInfo->HookAddress;
	CHAR* handlerAddr = hookInfo->HandlerAddress;
	DWORD jmpAddr = handlerAddr - hookAddr - 0x5;

	CHAR* p = &code[1];
	RtlCopyMemory(p, &jmpAddr, 0x4);

	VirtualProtect(hookAddr, 0x10, PAGE_EXECUTE_READWRITE, &hookInfo->Data[hookInfo->HookPageProtect]);
	RtlCopyMemory(rawCode, hookAddr, 0x5);
	RtlCopyMemory(hookAddr, &code, 0x5);

	hookInfo->HookState = TRUE;

	HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0x1000, 0x10000);		// test call

	return HK_STATUS_SUCCESS;
}

HKSTATUS INJTOR_DisableHook(PHOOK_NODE hookInfo)
{
	if (!hookInfo->HookAddress || !hookInfo->HandlerAddress)
		return HK_STATUS_FATAL;

	CHAR* rawCode = &hookInfo->Data[hookInfo->HookFuncRawCodeOffset];
	CHAR* hookAddr = hookInfo->HookAddress;

	RtlCopyMemory(rawCode, hookAddr, 0x5);

	hookInfo->HookState = FALSE;

	return HK_STATUS_SUCCESS;
}

VOID HANDLER_CountFuncion()
{
	__asm pushad;
	cnt++;
}

BOOL WINAPI DllMain(HINSTANCE const instance, DWORD const reason, LPVOID const reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		INJTOR_Initialize();

		break;
	case DLL_THREAD_ATTACH:

		break;
	case DLL_THREAD_DETACH:

		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

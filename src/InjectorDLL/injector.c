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

BYTE* _TryFindMatchFuncAddr(WCHAR* FileName, WCHAR* FuncName)
{
	CHAR funcName[64] = { 0 };

	HMODULE hDll = LoadLibraryW(FileName);
	if (!hDll)
		return NULL;

	wcstombs(&funcName, FuncName, 64);
	FARPROC func = GetProcAddress(hDll, funcName);

	return (BYTE*)func;
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
	g_buffer = HeapAlloc(g_runtimeHeap, 0, BUFFER_LENGTH);
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
		case COMMAND_HOOK_ENABLE:
			status = INJTOR_EnableHook(&cmdStruct->Context.HookNode);

			break;
		case COMMAND_HOOK_DISABLE:
			status = INJTOR_DisableHook(&cmdStruct->Context.HookNode);

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

SHORT _CalcHookSize(BYTE* asmCode)
{
	DWORD size = 0;
	CALLER_COMMAND cmd = { 0 };

	cmd.Command = COMMAND_HOOK_CALCSTUB;
	RtlCopyMemory(cmd.Context.AsmCode, asmCode, DISASM_LENGTH);

	if (!WriteFile(g_pipe, &cmd, sizeof(CALLER_COMMAND), &size, NULL))
		return NULL;

	if (!ReadFile(g_pipe, g_buffer, sizeof(CALLER_COMMAND), &size, NULL))
		return NULL;

	return ((CALLER_COMMAND*)g_buffer)->Context.HookSize;
}

BYTE* _CalcE9JmpAddress(BYTE* dstAddr, BYTE* srcAddr)
{
	return dstAddr - srcAddr - 0x5;
}

VOID _jmpBack()
{
	DebugBreak();

	SHORT size = g_node->HookFuncRawCodeSize;
	BYTE stubCode[5] = { 0 };
	stubCode[0] = '\xE9';

	BYTE* addr = _CalcE9JmpAddress(g_node->HookAddress, g_execBuffer + size);
	RtlCopyMemory(&stubCode[1], &addr, 0x4);
	RtlCopyMemory(g_execBuffer, &g_node->Data[g_node->HookFuncRawCodeOffset], size);
	RtlCopyMemory(g_execBuffer + size, &stubCode, 0x5);


	__asm
	{
		mov eax, g_regs.eax
		mov ecx, g_regs.ecx
		mov edx, g_regs.edx
		mov ebx, g_regs.ebx
		mov esp, g_regs.esp
		mov ebp, g_regs.ebp
		mov esi, g_regs.esi
		mov edi, g_regs.edi

		jmp g_execBuffer
	}
}

VOID  __declspec(naked) HANDLER_PreCall()
{
	/*
	* stack map
	*
	* push &node
	* push address of HANDLER_xxx
	*
	*/
	__asm
	{
		/*
		* To have a reg to pop stack, 
		* other wise need to add esp, 0x8 
		* because have two push
		*/
		mov g_regs.eax, eax

		pop eax		// address of HANDLER_xxx
		pop g_node

		mov g_regs.ecx, ecx
		mov g_regs.edx, edx
		mov g_regs.ebx, ebx
		mov g_regs.esp, esp		// now in here, the address is same to raw
		mov g_regs.ebp, ebp
		mov g_regs.esi, esi
		mov g_regs.edi, edi

		jmp eax
	}
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

	g_hookListHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0x1000, 0x10000);
	if (!g_hookListHeap)
	{
		HeapDestroy(g_runtimeHeap);
		return HK_STATUS_FATAL;
	}

	g_hookList = HeapAlloc(g_hookListHeap, HEAP_ZERO_MEMORY, sizeof(HOOK_NODE));
	if (!g_hookList)
		return HK_STATUS_FATAL;
	InitializeListHead(&g_hookList->ListEntry);

	g_execBuffer = HeapAlloc(g_runtimeHeap, HEAP_ZERO_MEMORY, 0x100);
	if (!g_execBuffer)
		return HK_STATUS_FATAL;
	DWORD temp = 0;
	VirtualProtect(g_execBuffer, 0x100, PAGE_EXECUTE_READWRITE, &temp);

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
	HOOK_NODE* node = HeapAlloc(g_hookListHeap, HEAP_ZERO_MEMORY, sizeof(HOOK_NODE));
	if (!node)
		return HK_STATUS_FATAL;

	RtlCopyMemory(node, hookInfo, sizeof(HOOK_NODE));

	WCHAR* fileName = &node->Data[node->Match.FileNameOffset];
	CHAR* funcName = &node->Data[node->Match.FuncNameOffset];

	if (!node->HookAddress)
		node->HookAddress = _TryFindMatchFuncAddr(fileName, funcName);

	if (!node->HandlerAddress)
		node->HandlerAddress = &HANDLER_CountFuncion;	// TODO: Custom in the future

	if (!node->HookAddress || !node->HandlerAddress)
		return HK_STATUS_FATAL;

	BYTE* rawCode = &node->Data[node->HookFuncRawCodeOffset];
	BYTE* hookAddr = node->HookAddress;
	BYTE* handlerAddr = node->HandlerAddress;

	RtlCopyMemory(rawCode, hookAddr, DISASM_LENGTH);

	SHORT size = _CalcHookSize(rawCode);
	if (size < HOOK_STUB_LENGTH)
		return HK_STATUS_FATAL;

	BYTE* stubCode = HeapAlloc(g_runtimeHeap, HEAP_ZERO_MEMORY, size);
	if (!stubCode)
		return HK_STATUS_FATAL;
	node->HookFuncRawCodeSize = size;
	RtlFillMemory(stubCode, size, '\x90');		// Use nop to fill this heap
	RtlFillMemory(&rawCode[size], DISASM_LENGTH - size, 0x00);		// Remove surpuls code

	DWORD preCallerAddr = _CalcE9JmpAddress(&HANDLER_PreCall, hookAddr + 0xA);	// hookAddr is push, need to add 5

	// Fill address into stub array
	stubCode[0] = '\x68';			// push &node
	stubCode[5] = '\x68';			// push address of HANDLER_xxx
	stubCode[10] = '\xE9';			// jmp HANDLER_PreCall (naked)

	RtlCopyMemory(&stubCode[1], &node, 0x4);
	RtlCopyMemory(&stubCode[6], &handlerAddr, 0x4);
	RtlCopyMemory(&stubCode[11], &preCallerAddr, 0x4);

	VirtualProtect(hookAddr, size, PAGE_EXECUTE_READWRITE, &node->Data[node->HookPageProtect]);

	RtlCopyMemory(hookAddr, stubCode, size);

	node->HookState = TRUE;

	InsertTailList(&g_hookList->ListEntry, &node->ListEntry);

	HeapFree(g_runtimeHeap, NULL, stubCode);

	DebugBreak();
	HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0x1000, 0x10000);		// test call

	return HK_STATUS_SUCCESS;
}

HKSTATUS INJTOR_DisableHook(PHOOK_NODE hookInfo)
{
	if (!hookInfo->HookAddress || !hookInfo->HandlerAddress)
		return HK_STATUS_FATAL;

	BYTE* rawCode = &hookInfo->Data[hookInfo->HookFuncRawCodeOffset];
	BYTE* hookAddr = hookInfo->HookAddress;

	RtlCopyMemory(rawCode, hookAddr, 0x5);

	hookInfo->HookState = FALSE;

	return HK_STATUS_SUCCESS;
}

VOID HANDLER_CountFuncion()
{
	cnt++;
	_jmpBack();
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

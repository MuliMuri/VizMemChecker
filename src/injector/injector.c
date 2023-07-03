#include <Windows.h>
#include <psapi.h>

#include "common/common.h"
#include "common/win_nt_def.h"

#include "handlers.h"
#include "stack.h"
#include "memory.h"
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

PHOOK_NODE g_node; // Current hook_node

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
	g_bufferHandle = MEM_Allocate(BUFFER_LENGTH);
	g_buffer = MEM_GetAddress(g_bufferHandle);

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
	SHORT size = g_node->HookFuncRawCodeSize;
	BYTE stubCode[5] = { 0 };
	stubCode[0] = '\xE9';

	BYTE *addr = _CalcE9JmpAddress(g_node->HookAddress + size, g_execBuffer + size);
	RtlCopyMemory(&stubCode[1], &addr, 0x4);
	RtlCopyMemory(g_execBuffer, &g_node->Data[g_node->HookFuncRawCodeOffset], size);
	RtlCopyMemory(g_execBuffer + size, &stubCode, 0x5);

	__asm
	{
		call STACK_Pop

		mov eax, g_context.eax
    	mov ecx, g_context.ecx
    	mov edx, g_context.edx
    	mov ebx, g_context.ebx
    	mov esp, g_context.esp
    	mov ebp, g_context.ebp
    	mov esi, g_context.esi
    	mov edi, g_context.edi
    	
		jmp g_execBuffer
	}
}

VOID CheckCaller()
{
	DWORD start = g_myselfInfo.BaseAddress;
	DWORD stop = g_myselfInfo.BaseAddress + g_myselfInfo.SizeOfImage;

	if (g_context.ret >= start && g_context.ret <= stop)
	{
		// myself call hooked function
		_jmpBack();
	}

	// other wise call handler
}

VOID __declspec(naked) HANDLER_PreCall()
{
	__asm
	{
		/*
		stack map

		push &node
		push address of HANDLER_xxx
		*/

		pop g_context.HandlerAddress // address of HANDLER_xxx
		pop g_node

    	mov g_context.eax, eax
		mov g_context.ecx, ecx

		mov ecx, ss:[esp]
		mov g_context.ret, ecx // address of caller

		mov g_context.edx, edx
		mov g_context.ebx, ebx
		mov g_context.esp, esp // now in here, the address is same to raw
		mov g_context.ebp, ebp
		mov g_context.esi, esi
		mov g_context.edi, edi

		call STACK_Push

		//call CheckCaller

		mov eax, g_context.HandlerAddress
		jmp eax
	}
}

// EntryPoint
// EntryPoint
// EntryPoint
HKSTATUS INJTOR_Initialize(HANDLE hMyself)
{	
	HKSTATUS status = HK_STATUS_SUCCESS;

	MEM_Initialize();
	STACK_Initialize();

	g_hookListHandle = MEM_Allocate(sizeof(HOOK_NODE));
	g_hookList = MEM_GetAddress(g_hookListHandle);
	if (!g_hookList)
		return HK_STATUS_FATAL;
	InitializeListHead(&g_hookList->ListEntry);

	g_execBufferHandle = MEM_Allocate(0x100);
	g_execBuffer = MEM_GetAddress(g_execBufferHandle);
	if (!g_execBuffer)
		return HK_STATUS_FATAL;
	DWORD temp = 0;
	VirtualProtect(g_execBuffer, 0x100, PAGE_EXECUTE_READWRITE, &temp);

	MODULEINFO tempMyself;
	GetModuleInformation(GetCurrentProcess(), hMyself, &tempMyself, sizeof(MODULEINFO));
	g_myselfInfo.BaseAddress = (DWORD)tempMyself.lpBaseOfDll;
	g_myselfInfo.SizeOfImage = (DWORD)tempMyself.SizeOfImage;

	DWORD pid = GetCurrentProcessId();

	status = _CreateDebuggerByPipe(pid);
	if (status != HK_STATUS_SUCCESS)
		return status;

	status = HANDLER_Initialize();
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
*	4. Run raw function
* 
*/

HKSTATUS INJTOR_EnableHook(PHOOK_NODE hookInfo)
{
	PVOID nodeHandle = MEM_Allocate(sizeof(HOOK_NODE));
	HOOK_NODE *node = MEM_GetAddress(nodeHandle);
	if (!node)
		return HK_STATUS_FATAL;

	RtlCopyMemory(node, hookInfo, sizeof(HOOK_NODE));

	WCHAR* fileName = &node->Data[node->Match.FileNameOffset];
	CHAR* funcName = &node->Data[node->Match.FuncNameOffset];

	if (!node->HookAddress)
		node->HookAddress = _TryFindMatchFuncAddr(fileName, funcName);

	if (!node->HandlerAddress)
		node->HandlerAddress = &HANDLER_Count;	// TODO: Custom in the future

	if (!node->HookAddress || !node->HandlerAddress)
		return HK_STATUS_FATAL;

	BYTE* rawCode = &node->Data[node->HookFuncRawCodeOffset];
	BYTE* hookAddr = node->HookAddress;
	BYTE* handlerAddr = node->HandlerAddress;

	RtlCopyMemory(rawCode, hookAddr, DISASM_LENGTH);

	SHORT size = _CalcHookSize(rawCode);
	if (size < HOOK_STUB_LENGTH)
		return HK_STATUS_FATAL;

	PVOID stubCodeHandle = MEM_Allocate(size);
	BYTE *stubCode = MEM_GetAddress(stubCodeHandle);
	if (!stubCode)
		return HK_STATUS_FATAL;
	node->HookFuncRawCodeSize = size;
	RtlFillMemory(stubCode, size, '\x90');		// Use nop to fill this heap
	RtlFillMemory(&rawCode[size], DISASM_LENGTH - size, 0x00);		// Remove surpuls code

	DWORD preCallerAddr = (DWORD)_CalcE9JmpAddress(&HANDLER_PreCall, hookAddr + 0xA);	// hookAddr is push, need to add 5

	// Fill address into stub array
	stubCode[0] = '\x68';			// push &node
	stubCode[5] = '\x68';			// push address of HANDLER_xxx
	stubCode[10] = '\xE9';			// jmp HANDLER_PreCall (naked)

	RtlCopyMemory(&stubCode[1], &node, 0x4);
	RtlCopyMemory(&stubCode[6], &handlerAddr, 0x4);
	RtlCopyMemory(&stubCode[11], &preCallerAddr, 0x4);

	VirtualProtect(hookAddr, size, PAGE_EXECUTE_READWRITE, &node->Data[node->HookPageProtect]);

	RtlCopyMemory(hookAddr, stubCode, size);	// Hook done

	node->HookState = TRUE;
	node->UID = 1;	//TODO:
	InsertTailList(&g_hookList->ListEntry, &node->ListEntry);

	HANDLER_AppendFuncChain(node);

	MEM_Free(stubCodeHandle);

	DebugBreak();
	HANDLE a = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0x100, 0x1000);
	HeapAlloc(a, HEAP_ZERO_MEMORY, 0x100);		// Test call <<<<=============================================

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

BOOL APIENTRY DllMain(HINSTANCE const instance, DWORD const reason, LPVOID const reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		INJTOR_Initialize(instance);

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

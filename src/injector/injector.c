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

BYTE* _TryFindMatchFuncAddr(CHAR* fileName, CHAR* funcName)
{

	HMODULE hDll = LoadLibraryA(fileName);
	if (!hDll)
		return NULL;

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
	g_buffer = MEM_Allocate(BUFFER_LENGTH);

	DWORD size = 0;
	HKSTATUS status = HK_STATUS_SUCCESS;

	ConnectNamedPipe(g_pipe, NULL);	// Wait for client connected.

	while (TRUE)
	{
		// :Use blocking IO
		if (!ReadFile(g_pipe, g_buffer, BUFFER_LENGTH, &size, NULL) || g_buffer == NULL)
			return HK_STATUS_FATAL;

		CALLER_COMMAND* cmd = (CALLER_COMMAND*)g_buffer;

		switch (cmd->Command)
		{
		case COMMAND_HOOK_APPEND:
			status = INJTOR_AppendHook(&cmd->Context.HookNode);

			break;
		case COMMAND_HOOK_REMOVE:

			break;
		case COMMAND_HANDLER_APPEND:
			status = INJTOR_AppendHandler(&cmd->Context.HookNode, &cmd->ExtraContext.HandlerName);

			break;
		case COMMAND_HOOK_ENABLE:
			status = INJTOR_EnableHook(&cmd->Context.HookNode);

			break;
		case COMMAND_HOOK_DISABLE:
			status = INJTOR_DisableHook(&cmd->Context.HookNode);

			break;
		default:
			break;
		}

		// CallBack
		if (status != HK_STATUS_SUCCESS)
			cmd->Command = COMMAND_ERR;

		if (!WriteFile(g_pipe, cmd, sizeof(CALLER_COMMAND), &size, NULL))
			break;/*return HK_STATUS_FATAL;*/
	}
}

HKSTATUS _StubCodeInitialize()
{
	g_stubCodeBuffer = MEM_Allocate(DISASM_LENGTH); // To allocate maximum possible memory for disassembly
	if (!g_stubCodeBuffer)
		return HK_STATUS_FATAL;

	RtlFillMemory(g_stubCodeBuffer, DISASM_LENGTH, '\x90'); // Use nop to fill this

	// Fill address into stub array
	g_stubCodeBuffer[0] = '\x68'; // push &node
	g_stubCodeBuffer[5] = '\xE9'; // jmp HANDLER_PreCall (naked)

	return HK_STATUS_SUCCESS;
}

USHORT _CalcHookSize(BYTE* asmCode)
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

VOID __declspec(naked) HANDLER_PreCall()
{
	/*
	* Here, if caller is myself, call the function by proxy
	* Attention: 	Keep stack balance
	* 				Keep 'g_context' switch to correct context
	*/
	__asm
	{
		/*
		* stack map

		* push &node
		* caller return address
		*/

		pop g_node

		call STACK_CheckProxy
		test al, al
		jnz HANDLER_ContinueInProxy

    	mov g_context.eax, eax
		mov g_context.ecx, ecx

		mov ecx, ss:[esp]
		mov g_context.ret, ecx // address of caller

		mov g_context.edx, edx
		mov g_context.ebx, ebx
		mov g_context.esp, esp // Now in here, the esp is same as raw
		mov g_context.ebp, ebp
		mov g_context.esi, esi
		mov g_context.edi, edi

		call STACK_Push

		call CheckCaller

		jmp HANDLER_PreChain
	}
}

VOID CheckCaller()
{
	DWORD ret = 0;
	DWORD start = g_myselfInfo.BaseAddress;
	DWORD stop = g_myselfInfo.BaseAddress + g_myselfInfo.SizeOfImage;

	__asm
	{
		mov eax, ss:[esp + 0x4 + 0x4 + 0xC]
		mov ret, eax
	}

	if (ret >= start && ret <= stop)
	{
		/*
		* Proxy caller, in here need to call the raw function
		*/
		STACK_Pop();	// Clear myself regs context
		HANDLER_ProxyCaller();

		// Not run here
	}

	// other wise call handler
}

// EntryPoint
// EntryPoint
// EntryPoint
HKSTATUS INJTOR_Initialize(HANDLE hMyself)
{	
	HKSTATUS status = HK_STATUS_SUCCESS;

	MEM_Initialize();
	STACK_Initialize();
	_StubCodeInitialize();

	g_hookList = MEM_Allocate(sizeof(HOOK_NODE));
	if (!g_hookList)
		return HK_STATUS_FATAL;
	InitializeListHead(&g_hookList->ListEntry);

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

HKSTATUS INJTOR_AppendHook(HOOK_NODE* hookNode)
{
	HOOK_NODE* node = MEM_Allocate(sizeof(HOOK_NODE));
	if (!node)
		return HK_STATUS_FATAL;

	RtlCopyMemory(node, hookNode, sizeof(HOOK_NODE));

	CHAR *fileName = &node->Data[node->Match.FileNameOffset];
	CHAR *funcName = &node->Data[node->Match.FuncNameOffset];

	if (!node->HookAddress)
		node->HookAddress = _TryFindMatchFuncAddr(fileName, funcName);

	if (!node->HookAddress)
		return HK_STATUS_FATAL;

	BYTE *rawCode = &node->Data[node->HookFuncRawCodeOffset];

	RtlCopyMemory(rawCode, node->HookAddress, DISASM_LENGTH);

	USHORT size = _CalcHookSize(rawCode);
	if (size < HOOK_STUB_LENGTH)
		return HK_STATUS_FATAL;

	node->HookSize = size;
	RtlFillMemory(&rawCode[size], DISASM_LENGTH - size, 0x00); // Remove surpuls code

	VirtualProtect(node->HookAddress, size, PAGE_EXECUTE_READWRITE, &node->Data[node->HookPageProtect]); // Adjust privilege

	InsertTailList(&g_hookList->ListEntry, &node->ListEntry);

	RtlCopyMemory(hookNode, node, sizeof(HOOK_NODE));

	return HK_STATUS_SUCCESS;
}

HKSTATUS INJTOR_AppendHandler(HOOK_NODE* hookNode, CHAR* handlerName)
{
	HANDLER_CHAIN *handlerChain = hookNode->HandlerChain;
	HANDLER_TABLE *handlerTable = SearchHandler(handlerName);

	for (size_t i = 0; i < hookNode->HandlerCount; i++)
	{
		if (handlerChain[i].HandlerAddress == handlerTable->HandlerAddress)
			return HK_STATUS_SUCCESS;
	}

	handlerChain = &handlerChain[hookNode->HandlerCount];
	handlerChain->HandlerAddress = handlerTable->HandlerAddress;
	hookNode->HandlerCount++;

	return HK_STATUS_SUCCESS;
}

HKSTATUS INJTOR_EnableHook(HOOK_NODE* hookNode)
{
	DWORD preHandlerAddr = &HANDLER_PreCall;
	DWORD hookAddr = hookNode->HookAddress;
	DWORD preCallerAddr = (DWORD)CalcE9JmpAddress(preHandlerAddr, (hookAddr + 0x5)); // hookAddr is push, need to add 0x5

	RtlCopyMemory(&g_stubCodeBuffer[1], &hookNode, 0x4);		// push &hookNode
	RtlCopyMemory(&g_stubCodeBuffer[6], &preCallerAddr, 0x4);	// jmp HANDLER_PreCall (naked)

	RtlCopyMemory(hookNode->HookAddress, g_stubCodeBuffer, hookNode->HookSize); // Hook done

	hookNode->Status = TRUE;

	return HK_STATUS_SUCCESS;
}

HKSTATUS INJTOR_DisableHook(HOOK_NODE* hookNode)
{
	// if (!hookInfo->HookAddress || !hookInfo->HandlerAddress)
	// 	return HK_STATUS_FATAL;

	// BYTE* rawCode = &hookInfo->Data[hookInfo->HookFuncRawCodeOffset];
	// BYTE* hookAddr = hookInfo->HookAddress;

	// RtlCopyMemory(rawCode, hookAddr, 0x5);

	// hookInfo->HookState = FALSE;

	// return HK_STATUS_SUCCESS;
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

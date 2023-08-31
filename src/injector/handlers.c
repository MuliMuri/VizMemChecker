#include <Windows.h>

#include "common/common.h"
#include "common/win_nt_def.h"

#include "injector.h"
#include "memory.h"
#include "stack.h"

#include "handlers.h"

PFUNC_CHAIN _TryFindFuncStatusByUID(USHORT uid)
{
	PLIST_ENTRY Entry = g_funcChainList->ListEntry.Flink;

	while (Entry != &g_funcChainList->ListEntry)
	{
		FUNC_CHAIN *pFunc = CONTAINING_RECORD(Entry, FUNC_CHAIN, ListEntry);

		if (pFunc->UID == uid)
			return pFunc;

		Entry = Entry->Flink;
	}

	return NULL;
}

PSTACK_CONTEXT GetCurrentThreadStack()
{
	DWORD tid = GetCurrentThreadId();

	THREAD_STACK* tsk = SearchStackGroupByTid(tid);

	return CONTAINING_RECORD(tsk->StackList.Next, STACK_CONTEXT, entry);
}

VOID HandlerRegister(CHAR* handlerName, VOID* handler)
{
	HANDLER_TABLE *table = MEM_Allocate(sizeof(HANDLER_TABLE));
	table->HandlerAddress = handler;

	char* handlerBuffer = MEM_Allocate(strlen(handlerName) + 1);
	strcpy_s(handlerBuffer, strlen(handlerName) + 1, handlerName);
	table->HandlerName = handlerBuffer;

	PushEntryList(&g_handlerTable, &table->Entry);
}

PHANDLER_TABLE SearchHandler(CHAR* handlerName)
{
	SINGLE_LIST_ENTRY *current = g_handlerTable.Next;
	HANDLER_TABLE *table = NULL;

	while (current != &g_handlerTable)
	{
		table = CONTAINING_RECORD(current, HANDLER_TABLE, Entry);
		if (!strcmp(table->HandlerName, handlerName))
			return table;

		current = current->Next;
	}

	return NULL;
}

HKSTATUS HANDLER_Initialize()
{
	HKSTATUS status = HK_STATUS_SUCCESS;

	g_funcChainList = MEM_Allocate(sizeof(FUNC_CHAIN));
	if (!g_funcChainList)
		return HK_STATUS_FATAL;

	g_execBuffer = MEM_Allocate(0x100);
	if (!g_execBuffer)
		return HK_STATUS_FATAL;
	DWORD temp = 0;
	VirtualProtect(g_execBuffer, 0x100, PAGE_EXECUTE_READWRITE, &temp);

	InitializeListHead(&g_funcChainList->ListEntry);

	g_handlerTable.Next = &g_handlerTable;

	return status;
}

HKSTATUS HANDLER_AppendFuncChain(HOOK_NODE* currNode)
{
	HKSTATUS status = HK_STATUS_SUCCESS;

	FUNC_CHAIN *fc = MEM_Allocate(sizeof(FUNC_CHAIN));
	if (!fc)
		return HK_STATUS_FATAL;

	fc->UID = currNode->UID;

	InsertTailList(&g_funcChainList->ListEntry, &fc->ListEntry);

	return status;
}

// VOID HANDLER_HeapCreate()
// {

// }

// VOID HANDLER_HeapAlloc()
// {
// 	STACK_CONTEXT* stack = GetCurrentThreadStack();
// 	DWORD* esp = stack->esp;
// 	DWORD handle = *(esp + 1);
// 	DWORD flag = *(esp + 2);
// 	DWORD size = *(esp + 3);

// 	DebugBreak();
// }

VOID PrepareExecBuffer()
{
	USHORT size = g_node->HookSize;
	BYTE stubCode[5] = {0};
	stubCode[0] = '\xE9';

	BYTE *addr = CalcE9JmpAddress((g_node->HookAddress + size), (g_execBuffer + size));
	RtlCopyMemory(&stubCode[1], &addr, 0x4);
	RtlCopyMemory(g_execBuffer, &g_node->Data[g_node->HookFuncRawCodeOffset], size);
	RtlCopyMemory(g_execBuffer + size, &stubCode, 0x5);
}

VOID __stdcall HANDLER_ContinueInProxy()
{
	__asm
	{
		pop ebp
		jmp g_execBuffer
	}
}

DWORD testEax = 0;
VOID __declspec(naked) HANDLER_ProxyCallback()
{
	/*
	* HANDLER_ProxyCaller call the function will be run here
	* In here, need to save 'eax', then return raw caller's ret
	*/
	__asm
	{
		int 3
		mov testEax, eax
		call STACK_UnProxy
		mov eax, testEax
		jmp g_context.ret
	}
}

VOID HANDLER_ProxyCaller()
{
	STACK_Pop(); // Recover g_context, now 'g_context' is correct g_context
	STACK_Proxy(); // Notic current thread in proxyCaller, to solve recursive call
	PrepareExecBuffer();

	DWORD callbackAddress = (DWORD)&HANDLER_ProxyCallback;

	__asm
	{
		mov eax, callbackAddress		// This value stone in stack, move it before change 'esp'
    	mov ecx, g_context.ecx
    	mov edx, g_context.edx
    	mov ebx, g_context.ebx
    	mov esp, g_context.esp
    	mov ebp, g_context.ebp
    	mov esi, g_context.esi
    	mov edi, g_context.edi
    	
		mov ss:[esp], eax				// callbackAddress -> ss:[esp] --- (ret address)
		mov eax, g_context.eax

		jmp g_execBuffer
	}
}

VOID HANDLER_PreChain()
{
	HANDLER_CHAIN *chain = (HANDLER_CHAIN *)&g_node->HandlerChain;

	for (size_t i = 0; i < g_node->HandlerCount; i++)
	{
		CallAddress(chain[i].HandlerAddress);
	}

	/*
	* Enter hooked function twice
	* Recover regs
	* To call the raw function by proxy
	* Ret to raw return address
	*/
	CallAddress(g_node->HookAddress);
}


/*Test Function*/
VOID HANDLER_Count()
{
	FUNC_CHAIN *funcStatus = _TryFindFuncStatusByUID(g_node->UID);
	if (!funcStatus)
		return;

	funcStatus->Count++;

	// _jmpBack();
}

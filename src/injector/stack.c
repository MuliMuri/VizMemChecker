#include <Windows.h>

#include "common/common.h"
#include "common/win_nt_def.h"

#include "memory.h"
#include "stack.h"

STACK_CONTEXT g_context;

PTHREAD_STACK SearchStackGroupByTid(DWORD tid)
{
	THREAD_STACK *tsk = CONTAINING_RECORD(g_threadStackList.Next, THREAD_STACK, Entry);

	while (&tsk->Entry != &g_threadStackList)
	{
		if (tsk->Tid == tid)
			return tsk;

		tsk = CONTAINING_RECORD(tsk->Entry.Next, THREAD_STACK, Entry);
	}

	return NULL;
}

VOID STACK_Initialize()
{
	g_threadStackList.Next = &g_threadStackList;
}

VOID STACK_Push()
{
	DWORD tid = GetCurrentThreadId();

	THREAD_STACK* tsk = SearchStackGroupByTid(tid);
	if (!tsk)
	{
		tsk = MEM_Allocate(sizeof(THREAD_STACK));
		tsk->Tid = tid;

		PushEntryList(&g_threadStackList, &tsk->Entry);
	}

	STACK_CONTEXT *stack = MEM_Allocate(sizeof(STACK_CONTEXT));

	RtlCopyMemory(stack, &g_context, sizeof(STACK_CONTEXT));

	PushEntryList(&tsk->StackList, &stack->entry);
}

VOID STACK_Pop()
{
	DWORD tid = GetCurrentThreadId();

	THREAD_STACK *tsk = SearchStackGroupByTid(tid);
	SINGLE_LIST_ENTRY *entry = PopEntryList(&tsk->StackList);
	STACK_CONTEXT *stack = CONTAINING_RECORD(entry, STACK_CONTEXT, entry);
	RtlCopyMemory(&g_context, stack, sizeof(STACK_CONTEXT));

	MEM_Free(stack);
}

VOID STACK_Proxy()
{
	DWORD tid = GetCurrentThreadId();

	THREAD_STACK *tsk = SearchStackGroupByTid(tid);
	tsk->IsProxyCall = TRUE;
}

VOID STACK_UnProxy()
{
	DWORD tid = GetCurrentThreadId();

	THREAD_STACK *tsk = SearchStackGroupByTid(tid);
	tsk->IsProxyCall = FALSE;
}

BYTE STACK_CheckProxy()
{
	DWORD tid = GetCurrentThreadId();

	THREAD_STACK *tsk = SearchStackGroupByTid(tid);
	if (!tsk)
		return FALSE;		// First trigger

	return tsk->IsProxyCall;
}

// VOID STACK_Reload(PSTACK_CONTEXT stack)
// {
// 	__asm
// 	{
// 		mov eax, stack.eax
//     	mov ecx, stack.ecx
//     	mov edx, stack.edx
//     	mov ebx, stack.ebx
//     	mov esp, stack.esp
//     	mov ebp, stack.ebp
//     	mov esi, stack.esi
//     	mov edi, stack.edi
// 	}
// }

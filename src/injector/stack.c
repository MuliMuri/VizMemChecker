#include <Windows.h>

#include "common/common.h"
#include "common/win_nt_def.h"

#include "memory.h"
#include "stack.h"

STACK_CONTEXT g_context;

VOID STACK_Initialize()
{
	
}

VOID STACK_Push()
{
	PVOID stackHandle = MEM_Allocate(sizeof(STACK_CONTEXT));
	STACK_CONTEXT *stack = MEM_GetAddress(stackHandle);

	RtlCopyMemory(stack, &g_context, sizeof(STACK_CONTEXT));

	PushEntryList(&g_contextListHead, &stack->entry);
}

VOID STACK_Pop()
{
	SINGLE_LIST_ENTRY *entry = PopEntryList(&g_contextListHead);
	STACK_CONTEXT *stack = CONTAINING_RECORD(entry, STACK_CONTEXT, entry);
	RtlCopyMemory(&g_context, stack, sizeof(STACK_CONTEXT));

	// MEM_Free(stack);
}

#pragma once
#ifndef __STACK_H__
#define __STACK_H__

typedef struct _STACK_CONTEXT
{
	SINGLE_LIST_ENTRY entry;

	DWORD eax;
	DWORD ecx;
	DWORD edx;
	DWORD ebx;
	DWORD esp;
	DWORD ebp;
	DWORD esi;
	DWORD edi;

	DWORD ret;

} STACK_CONTEXT, *PSTACK_CONTEXT;

typedef struct _THREAD_STACK
{
	SINGLE_LIST_ENTRY Entry;

	DWORD Tid;
	BYTE IsProxyCall;

	SINGLE_LIST_ENTRY StackList;

}THREAD_STACK, *PTHREAD_STACK;

static SINGLE_LIST_ENTRY g_threadStackList;

extern STACK_CONTEXT g_context;

PTHREAD_STACK SearchStackGroupByTid(DWORD tid);

VOID STACK_Initialize();
VOID STACK_Push();
VOID STACK_Pop();
VOID STACK_Proxy();
VOID STACK_UnProxy();
BYTE STACK_CheckProxy();
VOID STACK_Reload(PSTACK_CONTEXT stack);

#endif

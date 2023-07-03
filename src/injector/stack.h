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

	DWORD HandlerAddress;

} STACK_CONTEXT, *PSTACK_CONTEXT;


static SINGLE_LIST_ENTRY g_contextListHead;

extern STACK_CONTEXT g_context;

VOID STACK_Initialize();
VOID STACK_Push();
VOID STACK_Pop();

#endif

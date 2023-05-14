#pragma once
#ifndef __INJECTOR_H__
#define __INJECTOR_H__

typedef struct _REGS
{
	DWORD eax;
	DWORD ecx;
	DWORD edx;
	DWORD ebx;
	DWORD esp;
	DWORD ebp;
	DWORD esi;
	DWORD edi;

}REGS;



static HANDLE	g_hookListHeap;
static HANDLE	g_runtimeHeap;

static BYTE* g_execBuffer;

static PHOOK_NODE g_hookList;
static PHOOK_NODE	g_node;
static REGS		g_regs;

static HANDLE	g_pipe;

static BYTE*	g_buffer;



int cnt = 0;


HKSTATUS INJTOR_Initialize();
HKSTATUS INJTOR_EnableHook(PHOOK_NODE hookInfo);
HKSTATUS INJTOR_DisableHook(PHOOK_NODE hookInfo);
VOID HANDLER_CountFuncion();

BOOL WINAPI DllMain(
	HINSTANCE const instance,  // handle to DLL module
	DWORD     const reason,    // reason for calling function
	LPVOID    const reserved);  // reserved

#endif // !__INJECTOR_H__

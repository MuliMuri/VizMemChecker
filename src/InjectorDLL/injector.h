#pragma once
#ifndef __INJECTOR_H__
#define __INJECTOR_H__

static HANDLE	g_hookNodeHeap;
static HANDLE	g_runtimeHeap;
static HANDLE	g_pipe;

static CHAR*	g_buffer;
static INSTRUCTION* g_instLengthTable;



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

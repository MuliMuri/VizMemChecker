#pragma once
#ifndef __INJECTOR_H__
#define __INJECTOR_H__

#define CallAddress(address) (*(void (*)(void))address)()

typedef struct _MEMORY_REGION
{
	DWORD BaseAddress;
	DWORD SizeOfImage;

} MEMORY_REGION;

static BYTE *g_stubCodeBuffer;

static PHOOK_NODE g_hookList;

extern PHOOK_NODE	g_node;			// Current hook_node

static HANDLE	g_pipe;

static BYTE*	g_buffer;

static MEMORY_REGION g_myselfInfo;

HKSTATUS INJTOR_Initialize(HANDLE hMyself);
HKSTATUS INJTOR_AppendHook(HOOK_NODE *hookNode);
HKSTATUS INJTOR_AppendHandler(HOOK_NODE *hookNode, CHAR *handlerName);
HKSTATUS INJTOR_EnableHook(HOOK_NODE *hookNode);
HKSTATUS INJTOR_DisableHook(HOOK_NODE *hookNode);

VOID CheckCaller();

BOOL WINAPI DllMain(
	HINSTANCE const instance, // handle to DLL module
	DWORD const reason,		  // reason for calling function
	LPVOID const reserved);	  // reserved

#endif // !__INJECTOR_H__

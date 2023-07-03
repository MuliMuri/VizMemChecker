#pragma once
#ifndef __INJECTOR_H__
#define __INJECTOR_H__

typedef struct _MEMORY_REGION
{
	DWORD BaseAddress;
	DWORD SizeOfImage;

} MEMORY_REGION;

static PVOID g_execBufferHandle;
static BYTE* g_execBuffer;

static PVOID g_hookListHandle;
static PHOOK_NODE g_hookList;

extern PHOOK_NODE	g_node;			// Current hook_node

static HANDLE	g_pipe;

static PVOID g_bufferHandle;
static BYTE*	g_buffer;

static MEMORY_REGION g_myselfInfo;

HKSTATUS INJTOR_Initialize(HANDLE hMyself);
HKSTATUS INJTOR_EnableHook(PHOOK_NODE hookInfo);
HKSTATUS INJTOR_DisableHook(PHOOK_NODE hookInfo);

VOID _jmpBack();

BOOL WINAPI DllMain(
	HINSTANCE const instance, // handle to DLL module
	DWORD const reason,		  // reason for calling function
	LPVOID const reserved);	  // reserved

#endif // !__INJECTOR_H__

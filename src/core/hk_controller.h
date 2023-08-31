#pragma once
#ifndef __HK_CONTROLLER_H__
#define __HK_CONTROLLER_H__

static HANDLE			g_hookListHeap;			// A heap be save nodes of hooks structs

static PHOOK_NODE		g_hookList;					// HOOK_NODE ListEntry

HKSTATUS EXPORT HK_Initialize(DWORD pid);
HKSTATUS EXPORT HK_AppendHook(CHAR *FileName, CHAR *FuncName, USHORT* uid);
HKSTATUS EXPORT HK_AppendHandler(USHORT uid, CHAR *handlerName);
HKSTATUS EXPORT HK_RemoveHook(CHAR *FileName, CHAR *FuncName);
HKSTATUS EXPORT HK_EnableHook(USHORT uid);

PHOOK_NODE HK_EnumHook();

#endif // !__HK_CONTROLLER_H__

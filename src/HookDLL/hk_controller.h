#pragma once
#ifndef __HK_CONTROLLER_H__
#define __HK_CONTROLLER_H__

static HANDLE			g_hooksListHeap;			// A heap be save nodes of hooks structs
static HANDLE			g_hooksRawHeap;				// A head be save hooks raw function code
static PHOOK_NODE		g_hookList;					// HOOK_NODE ListEntry


HKSTATUS EXPORT HK_Initialize();
HKSTATUS EXPORT HK_AppendHookNode(WCHAR* FileName, WCHAR* FuncName);
HKSTATUS EXPORT HK_RemoveHookNode(WCHAR* FileName, WCHAR* FuncName);
HKSTATUS EXPORT HK_EnableOnceHook(WCHAR* FileName, WCHAR* FuncName);


#endif // !__HK_CONTROLLER_H__

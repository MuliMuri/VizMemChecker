#pragma once
#ifndef __HANDLERS_H__
#define __HANDLERS_H__

typedef struct _FUNC_CHAIN
{
	LIST_ENTRY ListEntry;

	USHORT UID;

	UINT Count;

} FUNC_CHAIN, *PFUNC_CHAIN;

typedef struct _HANDLER_CHAIN
{
	// struct _HANDLER_CHAIN *Next;
	VOID *HandlerAddress;

} HANDLER_CHAIN, *PHANDLER_CHAIN;

typedef struct _HANDLER_TABLE
{
	SINGLE_LIST_ENTRY Entry;

	CHAR* HandlerName;
	VOID *HandlerAddress;

} HANDLER_TABLE, *PHANDLER_TABLE;

static PFUNC_CHAIN g_funcChainList;
static SINGLE_LIST_ENTRY g_handlerTable;

static BYTE *g_execBuffer;

HKSTATUS HANDLER_Initialize();
HKSTATUS HANDLER_AppendFuncChain(HOOK_NODE *currNode);
PHANDLER_TABLE SearchHandler(CHAR *handlerName);
VOID __stdcall HANDLER_ContinueInProxy();
VOID HANDLER_ProxyCaller();
VOID HANDLER_PreChain();
VOID HANDLER_Count();

#endif	// !___HANDLERS_H__
#pragma once
#ifndef __HANDLERS_H__
#define __HANDLERS_H__

typedef struct _FUNC_CHAIN
{
	LIST_ENTRY ListEntry;

	USHORT UID;

	UINT Count;

} FUNC_CHAIN, *PFUNC_CHAIN;

static PVOID g_funcChainListHandle;
static PFUNC_CHAIN g_funcChainList;

HKSTATUS HANDLER_Initialize();
HKSTATUS HANDLER_AppendFuncChain(HOOK_NODE *currNode);
VOID HANDLER_Count();

#endif	// !___HANDLERS_H__
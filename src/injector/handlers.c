#include <Windows.h>

#include "common/common.h"
#include "common/win_nt_def.h"

#include "injector.h"
#include "memory.h"
#include "handlers.h"

PFUNC_CHAIN _TryFindFuncStatusByUID(USHORT uid)
{
	PLIST_ENTRY Entry = g_funcChainList->ListEntry.Flink;

	while (Entry != &g_funcChainList->ListEntry)
	{
		FUNC_CHAIN *pFunc = CONTAINING_RECORD(Entry, FUNC_CHAIN, ListEntry);

		if (pFunc->UID == uid)
			return pFunc;

		Entry = Entry->Flink;
	}

	return NULL;
}

HKSTATUS HANDLER_Initialize()
{
	HKSTATUS status = HK_STATUS_SUCCESS;

	g_funcChainListHandle = MEM_Allocate(sizeof(FUNC_CHAIN));
	g_funcChainList = MEM_GetAddress(g_funcChainListHandle);
	if (!g_funcChainList)
		return HK_STATUS_FATAL;

	InitializeListHead(&g_funcChainList->ListEntry);

	return status;
}

HKSTATUS HANDLER_AppendFuncChain(HOOK_NODE* currNode)		//TODO: custom chain
{
	HKSTATUS status = HK_STATUS_SUCCESS;

	PVOID fcHandle = MEM_Allocate(sizeof(FUNC_CHAIN));
	FUNC_CHAIN *fc = MEM_GetAddress(fcHandle);
	if (!fc)
		return HK_STATUS_FATAL;

	fc->UID = currNode->UID;

	InsertTailList(&g_funcChainList->ListEntry, &fc->ListEntry);

	return status;
}

VOID HANDLER_Count()
{
	FUNC_CHAIN *funcStatus = _TryFindFuncStatusByUID(g_node->UID);
	if (!funcStatus)
		return;

	funcStatus->Count++;

	_jmpBack();
}

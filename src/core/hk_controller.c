#include <Windows.h>

#include "common/common.h"
#include "common/win_nt_def.h"
#include "hk_controller.h"
#include "code_decoder.h"
#include "injector_caller.h"


PHOOK_NODE _SearchHookByName(CHAR* FileName, CHAR* FuncName)
{
	PLIST_ENTRY Entry = g_hookList->ListEntry.Flink;
	CHAR* hookFileName = NULL;
	CHAR* hookFuncName = NULL;

	while (Entry != &g_hookList->ListEntry)
	{
		HOOK_NODE* pHook = CONTAINING_RECORD(Entry, HOOK_NODE, ListEntry);

		hookFileName = &pHook->Data[pHook->Match.FileNameOffset];
		hookFuncName = &pHook->Data[pHook->Match.FuncNameOffset];

		if (!strcmp(hookFileName, FileName) && !strcmp(hookFuncName, FuncName))
			return pHook;

		Entry = Entry->Flink;
	}

	return NULL;
}

PHOOK_NODE _SearchHookByUid(USHORT uid)
{
	LIST_ENTRY *entry = g_hookList->ListEntry.Flink;

	while (entry != &g_hookList->ListEntry)
	{
		HOOK_NODE *current = CONTAINING_RECORD(entry, HOOK_NODE, ListEntry);
		if (current->UID == uid)
			return current;
	}

	return NULL;
}

HKSTATUS EXPORT HK_Initialize(DWORD pid)
{
	// Create a heap to be save MATCH_LIB_FILES node

	g_hookListHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0x1000, 0x10000);		// 4KB -> 64KB
	if (!g_hookListHeap)
		return HK_STATUS_FATAL;

	g_hookList = HeapAlloc(g_hookListHeap, HEAP_ZERO_MEMORY, sizeof(HOOK_NODE));
	if (!g_hookList)
		return HK_STATUS_FATAL;

	InitializeListHead(&g_hookList->ListEntry);

	DECODER_Initialize();
	CALLER_Initialize(pid);

	return HK_STATUS_SUCCESS;
}

HKSTATUS EXPORT HK_AppendHook(CHAR* FileName, CHAR* FuncName, USHORT* uid)
{
	// Maybe allocate char memory?
	// sometimes the chars will be free in caller (like stack free)

	HOOK_NODE* hookNode = HeapAlloc(g_hookListHeap, HEAP_ZERO_MEMORY, sizeof(HOOK_NODE));
	if (!hookNode)
		return HK_STATUS_FATAL;

	*uid = (CONTAINING_RECORD(g_hookList->ListEntry.Flink, HOOK_NODE, ListEntry)->UID) + 1;
	USHORT fileNameLength = strlen(FileName) * sizeof(CHAR);
	USHORT funcNameLength = strlen(FuncName) * sizeof(CHAR);

	hookNode->UID = *uid;
	hookNode->Match.FileNameOffset = 0;
	hookNode->Match.FuncNameOffset = fileNameLength + 0x2;
	hookNode->HookPageProtect = hookNode->Match.FuncNameOffset + funcNameLength + 0x2;
	hookNode->HookFuncRawCodeOffset = hookNode->HookPageProtect + sizeof(DWORD) + 0x2;

	RtlCopyMemory(&hookNode->Data[hookNode->Match.FileNameOffset], FileName, fileNameLength);
	RtlCopyMemory(&hookNode->Data[hookNode->Match.FuncNameOffset], FuncName, funcNameLength);

	CALLER_Call(COMMAND_HOOK_APPEND, hookNode);

	InsertTailList(&g_hookList->ListEntry, &hookNode->ListEntry);
	
	return HK_STATUS_SUCCESS;
}

HKSTATUS EXPORT HK_RemoveHook(CHAR* FileName, CHAR* FuncName)
{
	PHOOK_NODE node = _SearchHookByName(FileName, FuncName);
	if (!node)
	{
		RemoveEntryList(&node->ListEntry);
		HeapFree(g_hookListHeap, 0, node);
	}

	return HK_STATUS_SUCCESS;
}

HKSTATUS EXPORT HK_AppendHandler(USHORT uid, CHAR *handlerName)
{
	HOOK_NODE *node = _SearchHookByUid(uid);

	CALLER_Call(COMMAND_HANDLER_APPEND, node, handlerName);

	return HK_STATUS_SUCCESS;
}

HKSTATUS EXPORT HK_RemoveFunction(USHORT uid, CHAR* funcName)
{

}

HKSTATUS EXPORT HK_EnableHook(USHORT uid)
{
	HOOK_NODE *hookNode = _SearchHookByUid(uid);

	CALLER_Call(COMMAND_HOOK_ENABLE, hookNode);
}

HKSTATUS HK_DisableHook(CHAR* FileName, CHAR* FuncName)
{

}

PHOOK_NODE HK_EnumHook()
{
	return g_hookList;
}

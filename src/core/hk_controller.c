#include <Windows.h>

#include "common/common.h"
#include "common/win_nt_def.h"
#include "hk_controller.h"
#include "code_decoder.h"
#include "injector_caller.h"


PLIST_ENTRY _SearchHookNode(WCHAR* FileName, WCHAR* FuncName)
{
	PLIST_ENTRY Entry = g_hookList->ListEntry.Flink;
	WCHAR* hookFileName = NULL;
	WCHAR* hookFuncName = NULL;

	while (Entry != &g_hookList->ListEntry)
	{
		HOOK_NODE* pHook = CONTAINING_RECORD(Entry, HOOK_NODE, ListEntry);

		hookFileName = &pHook->Data[pHook->Match.FileNameOffset];
		hookFuncName = &pHook->Data[pHook->Match.FuncNameOffset];

		if (!wcscmp(hookFileName, FileName) && !wcscmp(hookFuncName, FuncName))
			return &pHook->ListEntry;

		Entry = Entry->Flink;
	}

	return NULL;
}

HKSTATUS EXPORT HK_Initialize(DWORD pid)//TODO: change
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

HKSTATUS EXPORT HK_AppendHookNode(WCHAR* FileName, WCHAR* FuncName)
{
	// Maybe allocate char memory?
	// sometimes the chars will be free in caller (like stack free)

	HOOK_NODE* hookNode = HeapAlloc(g_hookListHeap, HEAP_ZERO_MEMORY, sizeof(HOOK_NODE));
	if (!hookNode)
		return HK_STATUS_FATAL;

	SHORT fileNameLength = wcslen(FileName) * sizeof(WCHAR);
	SHORT funcNameLength = wcslen(FuncName) * sizeof(WCHAR);

	hookNode->Match.FileNameOffset = 0;
	hookNode->Match.FuncNameOffset = fileNameLength + 0x2;
	hookNode->HookPageProtect = hookNode->Match.FuncNameOffset + funcNameLength + 0x2;
	hookNode->HookFuncRawCodeOffset = hookNode->HookPageProtect + sizeof(DWORD) + 0x2;

	RtlCopyMemory(&hookNode->Data[hookNode->Match.FileNameOffset], FileName, fileNameLength);
	RtlCopyMemory(&hookNode->Data[hookNode->Match.FuncNameOffset], FuncName, funcNameLength);

	InsertTailList(&g_hookList->ListEntry, &hookNode->ListEntry);
	
	return HK_STATUS_SUCCESS;
}

HKSTATUS EXPORT HK_RemoveHookNode(WCHAR* FileName, WCHAR* FuncName)
{
	PLIST_ENTRY Entry = _SearchHookNode(FileName, FuncName);
	if (!Entry)
	{
		RemoveEntryList(Entry);
		HeapFree(g_hookListHeap, 0, CONTAINING_RECORD(Entry, HOOK_NODE, ListEntry));
	}

	return HK_STATUS_SUCCESS;
}

HKSTATUS EXPORT HK_EnableOnceHook(WCHAR* FileName, WCHAR* FuncName)
{
	LIST_ENTRY* entry = _SearchHookNode(FileName, FuncName);
	HOOK_NODE* hookNode = CONTAINING_RECORD(entry, HOOK_NODE, ListEntry);
	
	CALLER_EnableHook(hookNode);
}

HKSTATUS HK_DisableOnceHook(WCHAR* FileName, WCHAR* FuncName)
{

}

HKSTATUS HK_EnableAllHook(WCHAR* FileName, WCHAR* FuncName)
{

}

HKSTATUS HK_DisableAllHook(WCHAR* FileName, WCHAR* FuncName)
{

}

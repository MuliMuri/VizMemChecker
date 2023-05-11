#include <Windows.h>

#include "common.h"
#include "win_nt_def.h"

HKSTATUS InitializeListHead(PLIST_ENTRY ListHead)
{
    if (ListHead == NULL)
        return HK_STATUS_FATAL;

	ListHead->Blink = ListHead->Flink = ListHead;

    return HK_STATUS_SUCCESS;
}

HKSTATUS RemoveEntryList(PLIST_ENTRY Entry)
{
    PLIST_ENTRY PrevEntry;
    PLIST_ENTRY NextEntry;

    NextEntry = Entry->Flink;
    PrevEntry = Entry->Blink;
    if ((NextEntry->Blink != Entry) || (PrevEntry->Flink != Entry))
        return HK_STATUS_FATAL;

    PrevEntry->Flink = NextEntry;
    NextEntry->Blink = PrevEntry;
    return (BOOLEAN)(PrevEntry == NextEntry);
}

PLIST_ENTRY RemoveHeadList(PLIST_ENTRY ListHead)
{
    PLIST_ENTRY Entry;
    PLIST_ENTRY NextEntry;

    Entry = ListHead->Flink;

    NextEntry = Entry->Flink;
    if ((Entry->Blink != ListHead) || (NextEntry->Blink != Entry))
        return NULL;

    ListHead->Flink = NextEntry;
    NextEntry->Blink = ListHead;

    return Entry;
}

PLIST_ENTRY RemoveTailList(PLIST_ENTRY ListHead)
{
    PLIST_ENTRY Entry;
    PLIST_ENTRY PrevEntry;

    Entry = ListHead->Blink;

    PrevEntry = Entry->Blink;
    if ((Entry->Flink != ListHead) || (PrevEntry->Flink != Entry))
        return NULL;

    ListHead->Blink = PrevEntry;
    PrevEntry->Flink = ListHead;

    return Entry;
}

VOID InsertTailList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry)
{
    PLIST_ENTRY PrevEntry;

    PrevEntry = ListHead->Blink;
    if (PrevEntry->Flink != ListHead) {
        return;
    }

    Entry->Flink = ListHead;
    Entry->Blink = PrevEntry;
    PrevEntry->Flink = Entry;
    ListHead->Blink = Entry;
    return;
}

VOID InsertHeadList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry)
{
    PLIST_ENTRY NextEntry;

    NextEntry = ListHead->Flink;
    if (NextEntry->Blink != ListHead) {
        return;
    }

    Entry->Flink = NextEntry;
    Entry->Blink = ListHead;
    NextEntry->Blink = Entry;
    ListHead->Flink = Entry;
    return;
}

VOID AppendTailList(PLIST_ENTRY ListHead, PLIST_ENTRY ListToAppend)
{
    PLIST_ENTRY ListEnd = ListHead->Blink;

    ListHead->Blink->Flink = ListToAppend;
    ListHead->Blink = ListToAppend->Blink;
    ListToAppend->Blink->Flink = ListHead;
    ListToAppend->Blink = ListEnd;
    return;
}

// PushEntry
// PopEntry

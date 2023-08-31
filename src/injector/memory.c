#include <Windows.h>

#include "common/common.h"
#include "common/win_nt_def.h"

#include "memory.h"

CRITICAL_SECTION g_criticalSection;

inline MEM_HANDLE GenerateHandle(DWORD StartAddress, DWORD size)
{
	return ((StartAddress << 16) | size);
}

VOID MEM_Initialize()
{
	// 1MB
	BYTE* memoryBaseAddress = VirtualAlloc(NULL, 0x100000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	MEM_BLOCK *block = (MEM_BLOCK*)memoryBaseAddress;
	block->NextBlock = block->PrevBlock = block;
	block->Size = 0x100000 - sizeof(MEM_BLOCK);
	block->StartAddress = (DWORD)block + sizeof(MEM_BLOCK);

	memoryBlockList = block;

	totalFreeSize = block->Size;

	InitializeCriticalSection(&g_criticalSection);
}

PVOID MEM_Allocate(DWORD size)
{
	EnterCriticalSection(&g_criticalSection);

	if (size > totalFreeSize)
		return NULL;	// Another: try to realloc

	MEM_BLOCK *current = memoryBlockList;
	MEM_BLOCK *allocate = NULL;
	do
	{
		if (size <= current->Size && current->Handle == 0)
		{
			allocate = current;
			MEM_BLOCK *remain = current->StartAddress + size;

			remain->Size = current->Size - size - sizeof(MEM_BLOCK);
			remain->StartAddress = (DWORD)remain + sizeof(MEM_BLOCK);

			allocate->Size = size;

			MEM_BLOCK *nextBlock = current->NextBlock;

			remain->PrevBlock = current;
			remain->NextBlock = nextBlock;
			nextBlock->PrevBlock = remain;
			current->NextBlock = remain;

			break;

			// if (current->NextBlock == current)
			// {
			// 	// Only have one node
			// 	remain->NextBlock = remain->PrevBlock = remain;
			// 	memoryBlockList->NextBlock = remain;
			// 	break;
			// }
			// else
			// {
			// 	((MEM_BLOCK *)(current->PrevBlock))->NextBlock = remain;
			// 	remain->NextBlock = current->NextBlock;
			// 	remain->PrevBlock = current->PrevBlock;
			// 	break;
			// }
		}

		current = current->NextBlock;
	} while (current != memoryBlockList);

	allocate->Handle = GenerateHandle(allocate->StartAddress, allocate->Size);

	totalFreeSize -= (size + sizeof(MEM_BLOCK));

	LeaveCriticalSection(&g_criticalSection);

	return allocate->StartAddress;
}

VOID MEM_Free(PVOID handle)
{
	EnterCriticalSection(&g_criticalSection);

	MEM_BLOCK *current = memoryBlockList;

	if (current == NULL)
		return;

	do
	{
		if (current->Handle == handle)
		{
			current->Handle = 0;

			break;
		}

		current = current->NextBlock;
	} while (current != memoryBlockList);

	MEM_BLOCK *prevBlock = current->PrevBlock;
	MEM_BLOCK *nextBlock = current->NextBlock;

	if (prevBlock->Handle == 0 && prevBlock < current)	// Make sure the direction
	{
		prevBlock->Size += (current->Size + sizeof(MEM_BLOCK));
		prevBlock->NextBlock = current->NextBlock;
		((MEM_BLOCK *)(current->NextBlock))->PrevBlock = prevBlock;

		RtlZeroMemory(current, sizeof(MEM_BLOCK));

		current = prevBlock;
	}
	if (nextBlock->Handle == 0 && current < nextBlock)
	{
		current->Size += (nextBlock->Size + sizeof(MEM_BLOCK));
		current->NextBlock = nextBlock->NextBlock;
		((MEM_BLOCK *)(nextBlock->NextBlock))->PrevBlock = current;

		RtlZeroMemory(nextBlock, sizeof(MEM_BLOCK));
	}

	LeaveCriticalSection(&g_criticalSection);
}

VOID *MEM_GetAddress(PVOID handle)
{
	MEM_BLOCK *current = memoryBlockList;

	do
	{
		if (current->Handle == handle)
			return current->StartAddress;

		current = current->NextBlock;
	} while (current != memoryBlockList);

	return NULL;
}

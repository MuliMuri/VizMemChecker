#pragma once
#ifndef __MEMORY_H__
#define __MEMORY_H__

typedef PVOID MEM_HANDLE;

typedef struct _MEM_BLOCK
{
	struct MEM_BLOCK *PrevBlock;
	struct MEM_BLOCK *NextBlock;
	DWORD StartAddress;
	DWORD Size;
	MEM_HANDLE Handle;

} MEM_BLOCK, *PMEM_BLOCK;

DWORD totalFreeSize;

MEM_BLOCK* memoryBlockList;

VOID MEM_Initialize();
PVOID MEM_Allocate(DWORD size);
VOID MEM_Free(PVOID handle);
VOID *MEM_GetAddress(PVOID handle);

#endif

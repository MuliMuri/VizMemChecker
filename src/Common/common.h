#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__

typedef SHORT HKSTATUS;

#define EXPORT		__stdcall

#define BITWIDE_X86	0
#define BITWIDE_X64	1

#define HK_STATUS_SUCCESS	0x0000
#define HK_STATUS_FATAL		0xC000

/*
* 0x68 0xXXXXXXXX		push &node							0x5
* 0x68 0xXXXXXXXX		push address of HANDLER_xxx			0xA
* 0xE9 0xXXXXXXXX		jmp HANDLER_PreCall (naked)			0xF
*/
#define HOOK_STUB_LENGTH	0xA

/*************		INJTOR CONFIG		*************/

#define BUFFER_LENGTH	0x1000

#define DISASM_LENGTH	0x20

#define INJTOR_PIPE_NAME		L"\\\\.\\pipe\\MemLeakChecker\\"

#define COMMAND_HOOK_APPEND		0x00
#define COMMAND_HOOK_REMOVE		0x01
#define COMMAND_HANDLER_APPEND	0x02
#define COMMAND_HANDLER_REMOVE	0x03
#define COMMAND_HOOK_ENABLE		0x04
#define COMMAND_HOOK_DISABLE	0x05
#define COMMAND_HOOK_CALCSTUB	0x06

#define COMMAND_DEBUG_DETACH	0xC0

#define COMMAND_ERR				0xFF

#define CalcE9JmpAddress(dstAddr, srcAddr) (dstAddr - srcAddr - 0x5)

/*************		INJTOR CONFIG		*************/

//typedef struct _MEM_ALLOC_INFO_OPTIONAL
//{
//	DWORD
//
//}MEM_ALLOC_INFO_OPTIONAL, *PMEM_ALLOC_INFO_OPTIONAL;

typedef struct _PROCESS_INFO
{
	HANDLE ProcessHandle;
	CHAR 	ProcessName[64];
	DWORD		ProcessId;

} PROCESS_INFO, *PPROCESS_INFO;

typedef struct _MEM_ALLOC_INFO
{
	LIST_ENTRY ListEntry;

	PVOID 	HeapHandle;

	PVOID	CallerAddress;

	PVOID	MemoryAddress;
	ULONG64 MemorySize;

}MEM_ALLOC_INFO, *PMEM_ALLOC_INFO;

typedef struct _MATCH_LIB_FILES
{
	USHORT	FileNameOffset;
	USHORT	FuncNameOffset;

}MATCH_LIB_FILES, * PMATCH_LIB_FILES;

typedef struct _HOOK_NODE
{
	LIST_ENTRY			ListEntry;
	USHORT 				UID;						// UID
	MATCH_LIB_FILES		Match;						// Match Dll and Function
	BYTE				Status;						// Enable or disable
	USHORT 				HookSize;					// The size of Hook's impact
	BYTE*				HookAddress;				// Hook program address
	UCHAR				HandlerCount;				// Count of handler
	BYTE				HandlerChain[0xFF];			// Handler chain
	USHORT				HookFuncRawCodeOffset;		// Raw code before hook
	USHORT				HookPageProtect;			// Old page protect
	BOOLEAN				BitWide;					// x86 or x64

	BYTE				Data[0xFF];

}HOOK_NODE, * PHOOK_NODE;

typedef struct _CALLER_COMMAND
{
	BYTE Command;

	union
	{
		HOOK_NODE HookNode;
		BYTE	AsmCode[DISASM_LENGTH];
		USHORT	HookSize;

	}Context;

	union
	{
		CHAR HandlerName[64];
	}ExtraContext;

}CALLER_COMMAND, * PCALLER_COMMAND;

#endif // !__COMMON_H__

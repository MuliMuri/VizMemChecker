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
#define HOOK_STUB_LENGTH	0xF

/*************		INJTOR CONFIG		*************/

#define BUFFER_LENGTH	0x1000

#define DISASM_LENGTH	0x20

#define INJTOR_PIPE_NAME		L"\\\\.\\pipe\\MemLeakChecker\\"

#define COMMAND_HOOK_ENABLE		0x00
#define COMMAND_HOOK_DISABLE	0x01
#define COMMAND_HOOK_CALCSTUB	0x02

#define COMMAND_DEBUG_DETACH	0xC0

#define COMMAND_ERR				0xFF

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

typedef struct _MEM_ALLOC_INFO_NODE
{
	LIST_ENTRY ListEntry;

	PVOID	CallerAddress;

	PVOID	MemoryAddress;
	ULONG64 MemorySize;

}MEM_ALLOC_INFO_NODE, *PMEM_ALLOC_INFO_NODE;

typedef struct _MATCH_LIB_FILES
{
	SHORT	FileNameOffset;
	SHORT	FuncNameOffset;

}MATCH_LIB_FILES, * PMATCH_LIB_FILES;

typedef struct _HOOK_NODE
{
	LIST_ENTRY			ListEntry;
	MATCH_LIB_FILES		Match;						// Match Dll and Function
	BYTE				HookState;					// Enable or disable
	BYTE*				HookAddress;				// Hook program address
	BYTE*				HandlerAddress;				// Jmp to my hook function
	SHORT				HookFuncRawCodeOffset;		// Raw code before hook
	SHORT				HookFuncRawCodeSize;		// Raw code size
	SHORT				HookPageProtect;			// Old page protect
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
		SHORT	HookSize;

	}Context;

}CALLER_COMMAND, * PCALLER_COMMAND;

#endif // !__COMMON_H__

#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__

typedef SHORT HKSTATUS;

#define EXPORT		__stdcall

#define BITWIDE_X86	0
#define BITWIDE_X64	1

#define HK_STATUS_SUCCESS	0x0000
#define HK_STATUS_FATAL		0xC000



/*************		INJTOR CONFIG		*************/

#define BUFFER_LENGTH	0x1000

#define INJTOR_PIPE_NAME		L"\\\\.\\pipe\\MemLeakChecker\\"

#define COMMAND_ENABLE_HOOK		0x00
#define COMMAND_DISABLE_HOOK	0x01

#define COMMAND_DEBUG_DETACH	0xC0

#define COMMAND_ERR				0xFF

/*************		INJTOR CONFIG		*************/

typedef struct _INSTRUCTION
{
	UCHAR	Opcode;
	SHORT	Length;

}INSTRUCTION, *PINSTRUCTION;

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
	CHAR*				HookAddress;				// Hook program address
	CHAR*				HandlerAddress;				// Jmp to my hook function
	SHORT				HookFuncRawCodeOffset;		// Raw code before hook
	SHORT				HookPageProtect;			// Old page protect
	BOOLEAN				BitWide;					// x86 or x64

	CHAR				Data[0xFF];

}HOOK_NODE, * PHOOK_NODE;

typedef struct _CALLER_COMMAND
{
	BYTE Command;
	union _CommandUnion
	{
		HOOK_NODE HookNode;

	}CommandUnion;

}CALLER_COMMAND, * PCALLER_COMMAND;

void inline jmp_to_86(unsigned int address)
{
	((void(*)())address)();
}

void inline jmp_to_64()
{

}

#endif // !__COMMON_H__

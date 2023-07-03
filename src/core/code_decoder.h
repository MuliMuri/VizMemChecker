#pragma once
#ifndef __CODE_DECODER_H__
#define __CODE_DECODER_H__

#include "capstone/capstone.h"
#include "capstone/x86.h"

typedef struct _INSN
{
	uint64_t Address;
	size_t	Size;

}INSN, *PINSN;

typedef struct _INSN_LIST
{
	size_t Count;
	INSN* Context;

}INSN_LIST, *PINSN_LIST;

static csh g_capstoneHandle;
static HANDLE	g_insnHeap;

HKSTATUS DECODER_Initialize();
HKSTATUS DECODER_Decode(BYTE *asmCode, SIZE_T size, INSN_LIST *insnList);

#endif // !__CODE_DECODER_H__

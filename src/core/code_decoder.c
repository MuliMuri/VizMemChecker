#include <Windows.h>

#include "capstone/capstone.h"
#include "capstone/x86.h"

#include "common/common.h"
#include "common/win_nt_def.h"
#include "code_decoder.h"

HKSTATUS DECODER_Initialize()
{
	g_insnHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0x1000, 0x10000);
	if (!g_insnHeap)
		return HK_STATUS_FATAL;

	if (cs_open(CS_ARCH_X86, CS_MODE_32, &g_capstoneHandle) != CS_ERR_OK)
		return HK_STATUS_FATAL;

	return HK_STATUS_SUCCESS;
}

HKSTATUS DECODER_Decode(BYTE* asmCode, SIZE_T size, INSN_LIST* insnList)
{
	cs_insn* insn = NULL;
	size_t count = 0;

	count = cs_disasm(g_capstoneHandle, asmCode, size, 0x00000000, 0, &insn);
	if (!count)
		return HK_STATUS_FATAL;

	PINSN pInsn = HeapAlloc(g_insnHeap, HEAP_ZERO_MEMORY, count * sizeof(INSN));
	if (!pInsn)
		return HK_STATUS_FATAL;

	for (size_t i = 0; i < count; i++)
	{
		pInsn[i].Address = insn[i].address;
		pInsn[i].Size = insn[i].size;
	}

	insnList->Count = count;
	insnList->Context = pInsn;

	return HK_STATUS_SUCCESS;
}

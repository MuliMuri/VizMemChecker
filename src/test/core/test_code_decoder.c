#include <assert.h>
#include <Windows.h>

#include "common/common.h"

#include "core/code_decoder.h"

void test_DECODER_Initialize()
{
    assert(DECODER_Initialize() == HK_STATUS_SUCCESS);
}

void test_DECODER_Decode()
{
    BYTE *testAsm = "\xB8\x01\x00\x00\x00\xB8\x02\x00\x00\x00\x90\xB8\x03\x00\x00\x00\xB8\x04\x00\x00\x00\x68\x44\x33\x22\x11\x68\x44\x55\x66\x77\x58\x5B\x59\x83\xC0\x01";

    INSN_LIST *insn = malloc(sizeof(INSN_LIST));

    assert(DECODER_Decode(testAsm, 38, insn) != HK_STATUS_FATAL);
    assert(insn->Count == 11);

    free(insn);
}

int main(void)
{
    test_DECODER_Initialize();
    test_DECODER_Decode();

    return 0;
}

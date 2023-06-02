#include <assert.h>
#include <Windows.h>

#include "common/common.h"

#include "common/authority.h"

void test_EnableDebugPrivilege()
{
    assert(EnableDebugPrivilege() == HK_STATUS_SUCCESS);
}

int main(void)
{
    test_EnableDebugPrivilege();

    return 0;
}

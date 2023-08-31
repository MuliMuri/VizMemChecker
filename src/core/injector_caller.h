#pragma once
#ifndef __INJECTOR_CALLER_H__
#define __INJECTOR_CALLER_H__


static HANDLE g_pipe;

HKSTATUS CALLER_Initialize(DWORD pid);
HKSTATUS CALLER_Call(USHORT code, HOOK_NODE *hookNode, ...);

#endif // !__INJECTOR_CALLER_H__

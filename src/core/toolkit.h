#pragma once
#ifndef __TOOLKIT_H__
#define __TOOLKIT_H__

static HANDLE g_toolHeap;

HKSTATUS TOOL_Initialize();
// HKSTATUS TOOL_RunProgram(WCHAR *fileFullPath, PROGRAM_INFO *progInfo);
HKSTATUS TOOL_InjectDll(DWORD pid, CHAR *dllName);

#endif // !__TOOLKIT_H__

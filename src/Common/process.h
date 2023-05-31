#pragma once
#ifndef __PROCESS_H__
#define __PROCESS_H__

HKSTATUS PROC_FindProcessByName(CHAR *processName, PROCESS_INFO **processInfo);
HKSTATUS PROC_FindProcessById(DWORD pid, PROCESS_INFO **processInfo);
HKSTATUS PROC_GetProcessId(PROCESS_INFO **processInfo);

#endif // !__PROCESS_H__

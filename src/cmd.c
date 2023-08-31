#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <Windows.h>

#include "common/common.h"

#include "core/hk_controller.h"

#include "utils/logprint.h"

#include "cmd.h"

void CMD_HookAdd()
{
	char *lib = strtok(NULL, " \t");
	char *func = strtok(NULL, " ,\t");
	uint16_t uid = 0;

	do
	{
		HK_AppendHook(lib, func, &uid);
		HK_EnableHook(uid);
		printf(COLOR_GREEN("[+]") COLOR_PINK(" %s!%s \r\n"), lib, func);

		func = strtok(NULL, " ,\t");
	} while (func != NULL);
	
}

void CMD_HookList()
{
	HOOK_NODE *list = HK_EnumHook();
	printf("UID \t Status \t Library \t Function \t Address \r\n");
	LIST_ENTRY *current = list->ListEntry.Flink;

	while (current != &list->ListEntry)
	{
		HOOK_NODE *node = CONTAINING_RECORD(current, HOOK_NODE, ListEntry);

		printf("%d \t %c \t\t %s \t %s \t %x \r\n",
			   node->UID,
			   (int)(node->Status + 'D'),
			   &node->Data[node->Match.FileNameOffset],
			   &node->Data[node->Match.FuncNameOffset],
			   (uint32_t)node->HookAddress
			   );

		current = current->Flink;
	}
	
}

void CMD_HookHandlers()
{
	uint8_t count = 0;
	char *uid;
	char *handler;
	char *uids = strtok(NULL, " \t");
	char *handlers = strtok(NULL, " \t");
	HOOK_NODE *list = HK_EnumHook();
	HOOK_NODE *current = CONTAINING_RECORD(list->ListEntry.Flink, HOOK_NODE, ListEntry);

	char *handlersBak = malloc(strlen(handlers) + 1);
	strcpy_s(handlersBak, strlen(handlers) + 1, handlers);

	uid = strtok_s(uids, ",", &uids);
	do
	{
		handler = strtok(handlers, ",");
		while (handler != NULL)
		{
			HK_AppendHandler((uint16_t)atoi(uid), handler);
			handler = strtok(NULL, ",");
		}

		strcpy_s(handlers, strlen(handlersBak) + 1, handlersBak);
		uid = strtok_s(uids, ",", &uids);
	} while (uid != NULL);
}

void CMD_HookCommands(char* code)
{
	switch (code[1])
	{
	case 'a':
		CMD_HookAdd();
		break;
	
	case 'l':
		CMD_HookList();
		break;

	case 'f':
		CMD_HookHandlers();
		break;

	default:
		break;
	}
}

void CMD_DecodeAndExec(char *command)
{
	char *code = strtok(command, " \t");
	switch (code[0])
	{
	case 'h':
		CMD_HookCommands(code);
		break;
	
	default:
		break;
	}
}

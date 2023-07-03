#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "logprint.h"


void log_error(char* log)
{
	printf("%s\r\n", log);
}

void log_info(char* log)
{
	printf("%s\r\n", log);
}

// void log_format(char* log, short level, char** buffer)
// {
//     uint32_t size = strlen(log) + 1;
//     char *color_module = NULL;

//     switch (level)
//     {
//     case LEVEL_INFO:
		
//         break;
	
//     case LEVEL_ERROR:
//         color_module = COLOR_RED()
		
//         break;
//     default:
//         break;
//     }

//     if (!buffer)
//         return;

//     *buffer = malloc(size);

// }

// void log_error(char* log)
// {
//     log_format()
//     printf("%s\r\n", buffer);
// }

// void log_info(char* log)
// {
//     printf("%s\r\n", buffer);
// }

// void log_printf(char* log, short level)
// {
//     uint32_t size = strlen(log) + 1;
//     char *color_module = NULL;
//     char *buffer = malloc(size);
//     if (!buffer)
//         return;

//     switch (level)
//     {
//     case LEVEL_INFO:
//         color_module = COLOR

//             break;
//     case LEVEL_WARNING:

//         break;
//     case LEVEL_FATAL:

//         break;
//     default:
//         break;
//     }

//     sprintf_s(buffer, size, COLOR_RED("%s"), log);

//     free(buffer);
// }

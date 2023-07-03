#pragma once
#ifndef __LOGPRINT_H__
#define __LOGPRINT_H__

#define COLOR_RED(str)		"\033[0m\033[1;31m"str"\033[0m"
#define COLOR_YELLOW(str) 	"\033[0m\033[1;33m"str"\033[0m"
#define COLOR_GREEN(str)	"\033[0m\033[1;32m"str"\033[0m"
#define COLOR_PINK(str)		"\033[0m\033[1;35m"str"\033[0m"

#define LEVEL_INFO		0
#define LEVEL_WARNING   1
#define LEVEL_ERROR     2
#define LEVEL_FATAL     3

void log_info(char *log);
void log_error(char *log);

#endif // !__LOGPRINT_H__

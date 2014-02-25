#ifndef _H_TLOG_PRINT_H
#define _H_TLOG_PRINT_H

#include "tlog/tlog_config_types.h"

#include <stdarg.h>
#include <unistd.h>


#ifndef TLOG_PRINT_LEVEL
#define TLOG_PRINT_LEVEL e_tlog_debug
#endif//TLOG_PRINT_LEVEL

#define TLOG_MESSAGE_LENGTH 65536

#define TLOG_ERROR_COLOR "\033[;31m"
#define TLOG_WARN_COLOR "\033[;33m"
#define TLOG_INFO_COLOR "\033[;37m"
#define TLOG_DEBUG_COLOR "\033[;32m"
#define TLOG_COLOR_LEN 6

#define TLOG_RST_COLOR "\033[0m"
#define TLOG_RST_COLOR_LEN 4

void tlog_make_message(char *msg, size_t *msg_len, tlog_level_t level, va_list arglist);


void tlog_print(int fd, tlog_level_t level, ...);



#if TLOG_PRINT_LEVEL <= e_tlog_error
#define ERROR_PRINT(...) tlog_print(STDERR_FILENO, e_tlog_error, __VA_ARGS__)
#else
#define ERROR_PRINT(...)
#endif

#if TLOG_PRINT_LEVEL <= e_tlog_warn
#define WARN_PRINT(...) tlog_print(STDERR_FILENO, e_tlog_warn, __VA_ARGS__)
#else
#define WARN_PRINT(...)
#endif

#if TLOG_PRINT_LEVEL <= e_tlog_info
#define INFO_PRINT(...) tlog_print(STDOUT_FILENO, e_tlog_info, __VA_ARGS__)
#else
#define INFO_PRINT(...)
#endif

#if TLOG_PRINT_LEVEL <= e_tlog_debug
#define DEBUG_PRINT(...) tlog_print(STDOUT_FILENO, e_tlog_debug, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

#endif//_H_TLOG_PRINT_H

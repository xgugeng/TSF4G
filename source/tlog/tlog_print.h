#ifndef _H_TLOG_PRINT_H
#define _H_TLOG_PRINT_H

#include "tlog/tlog_config_types.h"

#include <stdarg.h>
#include <unistd.h>


#define TLOG_MESSAGE_LENGTH 65536


#define TLOG_ERROR_COLOR "\033[;31m"
#define TLOG_WARN_COLOR "\033[;33m"
#define TLOG_INFO_COLOR "\033[;37m"
#define TLOG_DEBUG_COLOR "\033[;32m"
#define TLOG_COLOR_LEN 6

#define TLOG_RST_COLOR "\033[0m"
#define TLOG_RST_COLOR_LEN 4

#ifndef ERROR_PRINT_OFF
#define ERROR_PRINT(...) tlog_print(STDERR_FILENO, e_tlog_error, __VA_ARGS__)
#else
#define ERROR_PRINT(...)
#endif

#ifndef WARN_PRINT_OFF
#define WARN_PRINT(...) tlog_print(STDERR_FILENO, e_tlog_warn, __VA_ARGS__)
#else
#define WARN_PRINT(...)
#endif

#ifndef INFO_PRINT_OFF
#define INFO_PRINT(...) tlog_print(STDOUT_FILENO, e_tlog_info, __VA_ARGS__)
#else
#define INFO_PRINT(...)
#endif

#ifndef DEBUG_PRINT_OFF
#define DEBUG_PRINT(...) tlog_print(STDOUT_FILENO, e_tlog_debug, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif


void tlog_make_message(char *msg, size_t *msg_len, tlog_level_t level, va_list arglist);

void tlog_print(int fd, tlog_level_t level, ...);

#endif//_H_TLOG_PRINT_H


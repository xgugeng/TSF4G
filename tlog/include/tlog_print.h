#ifndef _H_TLOG_PRINT_H
#define _H_TLOG_PRINT_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "tlog_config_types.h"
#include "tlog_message_types.h"

#include <stdarg.h>
#include <unistd.h>
#include <stdint.h>


#define TLOG_ERROR_COLOR "\033[;31m"
#define TLOG_WARN_COLOR "\033[;33m"
#define TLOG_INFO_COLOR "\033[;37m"
#define TLOG_DEBUG_COLOR "\033[;32m"
#define TLOG_COLOR_LEN 6

#define TLOG_RST_COLOR "\033[0m"
#define TLOG_RST_COLOR_LEN 4

#ifndef ERROR_PRINT_OFF
#define ERROR_PRINT(...) tlog_print(STDERR_FILENO, e_tlog_error, __FILE__, __LINE__, __VA_ARGS__)
#else
#define ERROR_PRINT(...)
#endif

#ifndef WARN_PRINT_OFF
#define WARN_PRINT(...) tlog_print(STDERR_FILENO, e_tlog_warn,__FILE__, __LINE__, __VA_ARGS__)
#else
#define WARN_PRINT(...)
#endif

#ifndef INFO_PRINT_OFF
#define INFO_PRINT(...) tlog_print(STDOUT_FILENO, e_tlog_info, __FILE__, __LINE__, __VA_ARGS__)
#else
#define INFO_PRINT(...)
#endif

#ifndef DEBUG_PRINT_OFF
#define DEBUG_PRINT(...) tlog_print(STDOUT_FILENO, e_tlog_debug, __FILE__, __LINE__, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif


void tlog_make_message(tlog_message_t *message, tlog_level_t level,
    const char* file, uint32_t line, va_list arglist);


void tlog_print(int fd, tlog_level_t level, const char* file, uint32_t line, ...);

#ifdef  __cplusplus
}
#endif

#endif//_H_TLOG_PRINT_H


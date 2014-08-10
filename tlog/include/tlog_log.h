#ifndef _H_TLOG_LOG_H
#define _H_TLOG_LOG_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "tlog.h"

#include <stdarg.h>
#include <stdint.h>

#ifndef ERROR_LOG_OFF
#define ERROR_LOG(...) tlog_log(&g_tlog_instance, e_tlog_error, __FILE__, __LINE__, __VA_ARGS__)
#else
#define ERROR_LOG(...)
#endif

#ifndef WARN_LOG_OFF
#define WARN_LOG(...) tlog_log(&g_tlog_instance, e_tlog_warn, __FILE__, __LINE__, __VA_ARGS__)
#else
#define WARN_LOG(...)
#endif

#ifndef INFO_LOG_OFF
#define INFO_LOG(...) tlog_log(&g_tlog_instance, e_tlog_info, __FILE__, __LINE__, __VA_ARGS__)
#else
#define INFO_LOG(...)
#endif


#ifndef DEBUG_LOG_OFF
#define DEBUG_LOG(...) tlog_log(&g_tlog_instance, e_tlog_debug, __FILE__, __LINE__, __VA_ARGS__)
#else
#define DEBUG_LOG(..)
#endif

extern tlog_t g_tlog_instance;

void tlog_log(tlog_t *self, tlog_level_t level, const char* file, uint32_t line, const char* fmt, ...) __attribute__((format(printf, 5,6)));;

#ifdef  __cplusplus
}
#endif

#endif//_H_TLOG_LOG_H


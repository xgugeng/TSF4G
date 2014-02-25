#ifndef _H_TLOG_LOG_H
#define _H_TLOG_LOG_H

#include "tlog/tlog.h"

#include <stdarg.h>

#ifndef ERROR_LOG_OFF
#define ERROR_LOG(...) tlog_log(&g_tlog_instance, e_tlog_error, __VA_ARGS__)
#else
#define ERROR_LOG(...)
#endif

#ifndef WARN_LOG_OFF
#define WARN_LOG(...) tlog_log(&g_tlog_instance, e_tlog_warn, __VA_ARGS__)
#else
#define WARN_LOG(...)
#endif

#ifndef INFO_LOG_OFF
#define INFO_LOG(...) tlog_log(&g_tlog_instance, e_tlog_info, __VA_ARGS__)
#else
#define INFO_LOG(...)
#endif


#ifndef DEBUG_LOG_OFF
#define DEBUG_LOG(...) tlog_log(&g_tlog_instance, e_tlog_debug, __VA_ARGS__)
#else
#define DEBUG_LOG(..)
#endif

extern tlog_t g_tlog_instance;

void tlog_log(tlog_t *self, tlog_level_t level, ...);


#endif//_H_TLOG_LOG_H


#ifndef _H_TLOG_LOG_H
#define _H_TLOG_LOG_H

#include "tlog/tlog.h"
#include "tlog/tlog_print.h"

#include <stdarg.h>


#ifndef TLOG_LOG_LEVEL
#define TLOG_LOG_LEVEL e_tlog_debug
#endif//TLOG_LOG_LEVEL


extern tlog_t g_tlog_instance;

void tlog_log(tlog_t *self, tlog_level_t level, ...);

#if TLOG_LOG_LEVEL <= e_tlog_error
#define ERROR_LOG(...) tlog_log(&g_tlog_instance, e_tlog_error, __VA_ARGS__)
#else
#define ERROR_LOG(...)
#endif

#if TLOG_LOG_LEVEL <= e_tlog_warn
#define WARN_LOG(...) tlog_log(&g_tlog_instance, e_tlog_warn, __VA_ARGS__)
#else
#define WARN_LOG(...)
#endif

#if TLOG_LOG_LEVEL <= e_tlog_info
#define INFO_LOG(...) tlog_log(&g_tlog_instance, e_tlog_info, __VA_ARGS__)
#else
#define INFO_LOG(...)
#endif

#if TLOG_LOG_LEVEL <= e_tlog_debug
#define DEBUG_LOG(...) tlog_log(&g_tlog_instance, e_tlog_debug, __VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif


#endif//_H_TLOG_LOG_H


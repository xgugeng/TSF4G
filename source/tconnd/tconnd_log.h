#ifndef _H_TCONND_LOG_H
#define _H_TCONND_LOG_H

#include "tlog/tlog.h"
#include "tconnd/globals.h"
#include "stdio.h"


#define ERROR_LOG(...) TLOG_HELPER_LOG(&g_tlog_instance, e_tlog_error, __VA_ARGS__)
#define INFO_LOG(...) TLOG_HELPER_LOG(&g_tlog_instance, e_tlog_info, __VA_ARGS__)
#define DEBUG_LOG(...) TLOG_HELPER_LOG(&g_tlog_instance, e_tlog_debug, __VA_ARGS__)


#define INFO_PRINTF(...) TLOG_HELPER_PRINTF(&g_tlog_instance, e_tlog_info, __VA_ARGS__)

#endif//_H_TCONND_LOG_H


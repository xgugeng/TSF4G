#ifndef _H_TLOG_INSTANCE_H
#define _H_TLOG_INSTANCE_H

#include "tlog/tlog.h"
#include "tlog/tlog_print.h"

extern tlog_t g_tlog_instance;

#define TLOG_LOG(inst, lv, ...)\
{\
    if(lv <= (inst)->config.level )\
    {\
        char message[TLOG_MESSAGE_LENGTH];\
        size_t message_len;\
        TLOG_MAKE_MESSAGE(message, TLOG_MESSAGE_LENGTH, message_len, lv, __VA_ARGS__)\
	    tlog_write(inst, message, message_len);\
	}\
}

#define ERROR_LOG(...) TLOG_LOG(&g_tlog_instance, e_tlog_error, __VA_ARGS__)
#define WARN_LOG(...) TLOG_LOG(&g_tlog_instance, e_tlog_warn, __VA_ARGS__)
#define INFO_LOG(...) TLOG_LOG(&g_tlog_instance, e_tlog_info, __VA_ARGS__)
#define DEBUG_LOG(...) TLOG_LOG(&g_tlog_instance, e_tlog_debug, __VA_ARGS__)


#endif//_H_TLOG_INSTANCE_H


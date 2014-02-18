#ifndef _H_TLOG_INSTANCE_H
#define _H_TLOG_INSTANCE_H
#include "tlog/tlog.h"
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <sys/uio.h>


extern tlog_t g_tlog_instance;

#define TLOG_MESSAGE_LENGTH 65536

#define TLOG_MAKE_MESSAGE(msg, msg_limit, msg_len, level, ...)\
{\
    struct timeval timestamp;\
    struct tm   tm;\
    const char* level_name = "";\
    switch(level)\
    {\
    case e_tlog_error:\
        level_name = "error";\
        break;\
    case e_tlog_warn:\
        level_name = "warn";\
        break;\
    case e_tlog_info:\
        level_name = "info";\
        break;\
    case e_tlog_debug:\
        level_name = "debug";\
        break;\
    }\
    gettimeofday(&timestamp, NULL);\
    localtime_r(&timestamp.tv_sec, &tm);\
    msg_len = snprintf(msg, msg_limit,\
        "%04d-%02d-%02d %02d:%02d:%02d [%s] %s:%u : ",\
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,\
        tm.tm_hour, tm.tm_min, tm.tm_sec\
        ,level_name, __FILE__, __LINE__\
        );\
    if(msg_len < msg_limit)\
    {\
        size_t _len = snprintf(message + msg_len, msg_limit - msg_len\
            , __VA_ARGS__);\
        msg_len += _len;\
    }\
    if(msg_len < msg_limit)\
    {\
        message[msg_len] = '\n';\
        ++msg_len;\
    }\
	else\
	{\
		message[msg_len - 1] = '\n';\
	}\
}

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

#define TLOG_ERROR_COLOR "\033[;31m"
#define TLOG_WARN_COLOR "\033[;33m"
#define TLOG_INFO_COLOR "\033[;37m"
#define TLOG_DEBUG_COLOR "\033[;32m"
#define TLOG_COLOR_LEN 6

#define TLOG_RST_COLOR "\033[0m"
#define TLOG_RST_COLOR_LEN 4

#ifndef TLOG_INSTANCE_LEVEL
#define TLOG_INSTANCE_LEVEL e_tlog_debug
#endif//TLOG_INSTANCE_LEVEL

#include <string.h>
#define TLOG_PRINT(fout, lv, ...)\
{\
    if(lv <= TLOG_INSTANCE_LEVEL)\
    {\
        struct iovec iov[3];\
        char message[TLOG_MESSAGE_LENGTH];\
        size_t message_len;\
        TLOG_MAKE_MESSAGE(message, TLOG_MESSAGE_LENGTH, message_len, lv, __VA_ARGS__)\
        switch(lv)\
        {\
        case e_tlog_error:\
            iov[0].iov_base = TLOG_ERROR_COLOR;\
            break;\
        case e_tlog_warn:\
            iov[0].iov_base = TLOG_WARN_COLOR;\
            break;\
        case e_tlog_info:\
            iov[0].iov_base = TLOG_INFO_COLOR;\
            break;\
        case e_tlog_debug:\
            iov[0].iov_base = TLOG_DEBUG_COLOR;\
            break;\
        }\
        iov[0].iov_len = TLOG_COLOR_LEN;\
        iov[1].iov_base = message;\
        iov[1].iov_len = message_len;\
        iov[2].iov_base = TLOG_RST_COLOR;\
        iov[2].iov_len = TLOG_RST_COLOR_LEN;\
        writev(1, iov, 3);\
    }\
}

#define ERROR_PRINT(...) TLOG_PRINT(stderr, e_tlog_error, __VA_ARGS__)
#define WARN_PRINT(...) TLOG_PRINT(stderr, e_tlog_warn, __VA_ARGS__)
#define INFO_PRINT(...) TLOG_PRINT(stdout, e_tlog_info, __VA_ARGS__)
#define DEBUG_PRINT(...) TLOG_PRINT(stdout, e_tlog_debug, __VA_ARGS__)


#endif//_H_TLOG_INSTANCE_H


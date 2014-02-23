#ifndef _H_TLOG_PRINT_H
#define _H_TLOG_PRINT_H

#include "tlog/tlog_config_types.h"

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <sys/uio.h>
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



#define TLOG_MAKE_MESSAGE(msg, msg_limit, msg_len, level, ...)\
{\
    struct timeval timestamp;\
    struct tm   tm;\
    const char* level_name = "";\
    int _len;\
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
    msg_len = 0;\
    _len = snprintf(msg, msg_limit,\
        "%04d-%02d-%02d %02d:%02d:%02d [%s] %s:%u : ",\
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,\
        tm.tm_hour, tm.tm_min, tm.tm_sec\
        ,level_name, __FILE__, __LINE__\
        );\
    if(_len >= 0)\
    {\
        msg_len += (size_t)_len;\
    }\
    if(msg_len < msg_limit)\
    {\
        _len = snprintf(message + msg_len, msg_limit - msg_len\
            , __VA_ARGS__);\
        if(_len >= 0)\
        {\
            msg_len += (size_t)_len;\
        }\
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


#define TLOG_PRINT(fd, lv, ...)\
{\
    if(lv <= TLOG_PRINT_LEVEL)\
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
        writev(fd, iov, 3);\
    }\
}

#if TLOG_PRINT_LEVEL <= e_tlog_error
#define ERROR_PRINT(...) TLOG_PRINT(STDOUT_FILENO, e_tlog_error, __VA_ARGS__)
#else
#define ERROR_PRINT(...)
#endif

#if TLOG_PRINT_LEVEL <= e_tlog_warn
#define WARN_PRINT(...) TLOG_PRINT(STDOUT_FILENO, e_tlog_warn, __VA_ARGS__)
#else
#define WARN_PRINT(...)
#endif

#if TLOG_PRINT_LEVEL <= e_tlog_info
#define INFO_PRINT(...) TLOG_PRINT(STDOUT_FILENO, e_tlog_info, __VA_ARGS__)
#else
#define INFO_PRINT(...)
#endif

#if TLOG_PRINT_LEVEL <= e_tlog_debug
#define DEBUG_PRINT(...) TLOG_PRINT(STDOUT_FILENO, e_tlog_debug, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

#endif//_H_TLOG_PRINT_H

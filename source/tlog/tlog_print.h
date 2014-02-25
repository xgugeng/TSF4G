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


//void tlog_make_message(const char *msg, char* msg_limit, tlog_level_t level, ...);

#define TLOG_MAKE_MESSAGE(msg, msg_limit, msg_len, level, ...)\
{\
    struct timeval _tlog_print_timestamp;\
    struct tm   _tlog_print_tm;\
    const char* _tlog_print_level_name = "";\
    int _tlog_print_len;\
    switch(level)\
    {\
    case e_tlog_error:\
        _tlog_print_level_name = "error";\
        break;\
    case e_tlog_warn:\
        _tlog_print_level_name = "warn";\
        break;\
    case e_tlog_info:\
        _tlog_print_level_name = "info";\
        break;\
    case e_tlog_debug:\
        _tlog_print_level_name = "debug";\
        break;\
    }\
    gettimeofday(&_tlog_print_timestamp, NULL);\
    localtime_r(&_tlog_print_timestamp.tv_sec, &_tlog_print_tm);\
    (msg_len) = 0;\
    _tlog_print_len = snprintf(msg, msg_limit,\
        "%04d-%02d-%02d %02d:%02d:%02d [%s] %s:%u : ",\
        _tlog_print_tm.tm_year + 1900, _tlog_print_tm.tm_mon + 1, _tlog_print_tm.tm_mday,\
        _tlog_print_tm.tm_hour, _tlog_print_tm.tm_min, _tlog_print_tm.tm_sec\
        ,_tlog_print_level_name, __FILE__, __LINE__\
        );\
    if(_tlog_print_len >= 0)\
    {\
        (msg_len) += (size_t)_tlog_print_len;\
    }\
    if((msg_len) < msg_limit)\
    {\
        _tlog_print_len = snprintf(msg + (msg_len), msg_limit - (msg_len)\
            , __VA_ARGS__);\
        if(_tlog_print_len >= 0)\
        {\
            (msg_len) += (size_t)_tlog_print_len;\
        }\
    }\
    if(msg_len < msg_limit)\
    {\
        msg[msg_len] = '\n';\
        ++msg_len;\
    }\
	else\
	{\
		msg[msg_len - 1] = '\n';\
	}\
}


#define TLOG_PRINT(fd, lv, ...)\
{\
    if(lv <= TLOG_PRINT_LEVEL)\
    {\
        struct iovec _tlog_print_iov[3];\
        char _tlog_print_message[TLOG_MESSAGE_LENGTH];\
        size_t _tlog_print_message_len;\
        TLOG_MAKE_MESSAGE(_tlog_print_message, TLOG_MESSAGE_LENGTH, _tlog_print_message_len, lv, __VA_ARGS__)\
        switch(lv)\
        {\
        case e_tlog_error:\
            _tlog_print_iov[0].iov_base = TLOG_ERROR_COLOR;\
            break;\
        case e_tlog_warn:\
            _tlog_print_iov[0].iov_base = TLOG_WARN_COLOR;\
            break;\
        case e_tlog_info:\
            _tlog_print_iov[0].iov_base = TLOG_INFO_COLOR;\
            break;\
        case e_tlog_debug:\
            _tlog_print_iov[0].iov_base = TLOG_DEBUG_COLOR;\
            break;\
        }\
        _tlog_print_iov[0].iov_len = TLOG_COLOR_LEN;\
        _tlog_print_iov[1].iov_base = _tlog_print_message;\
        _tlog_print_iov[1].iov_len = _tlog_print_message_len;\
        _tlog_print_iov[2].iov_base = TLOG_RST_COLOR;\
        _tlog_print_iov[2].iov_len = TLOG_RST_COLOR_LEN;\
        writev(fd, _tlog_print_iov, 3);\
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

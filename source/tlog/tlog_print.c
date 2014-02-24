#include "tlog/tlog_print.h"

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <sys/uio.h>
#include <unistd.h>


void tlog_make_message(char *msg, size_t *msg_len, tlog_level_t level,
    const char* file, uint32_t line, va_list arglist)
{
    struct timeval timestamp;
    struct tm   tm;
    const char* level_name = "";
    size_t len;
    ssize_t r;
    char *fmt;
    
    switch(level)
    {
    case e_tlog_error:
        level_name = "error";
        break;
    case e_tlog_warn:
        level_name = "warn";
        break;
    case e_tlog_info:
        level_name = "info";
        break;
    case e_tlog_debug:
        level_name = "debug";
        break;
    }
    
    gettimeofday(&timestamp, NULL);    
    localtime_r(&timestamp.tv_sec, &tm);

    len = 0;
    r = snprintf(msg, *msg_len - len,
        "%04d-%02d-%02d %02d:%02d:%02d [%s] %s:%u : ",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec
        ,level_name, file, line);

        
    if(r > 0)
    {
        len += (size_t)r;
        msg += r;
    }

    fmt = va_arg(arglist, char*);

    if(*msg_len >= len)
    {
        r = vsnprintf(msg, *msg_len - len, fmt, arglist);
        if(r > 0)
        {
            len += (size_t)r;
            msg += r;
        }
    }
    
	*msg_len = len;
}



void tlog_print(int fd, tlog_level_t level, const char* file, uint32_t line, ...)
{
    struct iovec iov[4];
    char message[TLOG_MESSAGE_LENGTH];
    size_t message_len;
    va_list arglist;

    message_len = TLOG_MESSAGE_LENGTH;

    va_start(arglist, line);
    tlog_make_message(message, &message_len, level, file, line, arglist);
    va_end(arglist);
    
    switch(level)
    {
    case e_tlog_error:
        iov[0].iov_base = TLOG_ERROR_COLOR;
        break;
    case e_tlog_warn:
        iov[0].iov_base = TLOG_WARN_COLOR;
        break;
    case e_tlog_info:
        iov[0].iov_base = TLOG_INFO_COLOR;
        break;
    case e_tlog_debug:
        iov[0].iov_base = TLOG_DEBUG_COLOR;
        break;
    }

    iov[0].iov_len = TLOG_COLOR_LEN;
    iov[1].iov_base = message;
    iov[1].iov_len = message_len;
    iov[2].iov_base = TLOG_RST_COLOR;
    iov[2].iov_len = TLOG_RST_COLOR_LEN;
    iov[3].iov_base = "\n";
    iov[3].iov_len = 1;
    writev(fd, iov, 4);
    fsync(fd);
    
}


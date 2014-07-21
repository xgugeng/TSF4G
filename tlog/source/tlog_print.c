#include "tlog_print.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>

void tlog_make_message(tlog_message_t *message, tlog_level_t level,
    const char* file, uint32_t line, va_list arglist)
{
    char* msg = message->msg;
    struct timeval timestamp;
    struct tm   tm;
    const char* level_name = "";
    size_t len;
    ssize_t r;
    char *fmt;
	message->level = level;
    
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

	message->year = tm.tm_year + 1900;
	message->month = tm.tm_mon + 1;
	message->day = tm.tm_mday;
	message->hour = tm.tm_hour;
	message->min = tm.tm_min;
	message->sec = tm.tm_sec;

    len = 0;
    r = snprintf(msg, TLOG_MESSAGE_LENGTH - len,
        "%04d-%02d-%02d %02d:%02d:%02d [%s] %s:%u | ",
        message->year, message->month, message->day,
        tm.tm_hour, tm.tm_min, tm.tm_sec
        ,level_name, file, line);

        
    if(r > 0)
    {
        len += (size_t)r;
        msg += r;
    }

    fmt = va_arg(arglist, char*);

    if(TLOG_MESSAGE_LENGTH >= len)
    {
        r = vsnprintf(msg, TLOG_MESSAGE_LENGTH - len, fmt, arglist);
        if(r > 0)
        {
            len += (size_t)r;
            msg += r;
        }
    }
}

void tlog_print(int fd, tlog_level_t level, const char* file, uint32_t line, ...)
{
    struct iovec iov[4];
    tlog_message_t message;

    va_list arglist;
    va_start(arglist, line);
    tlog_make_message(&message, level, file, line, arglist);
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
    iov[1].iov_base = message.msg;
    iov[1].iov_len = strlen(message.msg);
    iov[2].iov_base = TLOG_RST_COLOR;
    iov[2].iov_len = TLOG_RST_COLOR_LEN;
    iov[3].iov_base = "\n";
    iov[3].iov_len = 1;
    writev(fd, iov, 4);
    fsync(fd);
}


#include "tlog_log.h"
#include "tlog_print.h"

#include "tlog/tlog.h"

tlog_t g_tlog_instance;

void tlog_log(tlog_t *self, tlog_level_t level, const char* file, uint32_t line, ...)
{
    char message[TLOG_MESSAGE_LENGTH];
    size_t message_len = TLOG_MESSAGE_LENGTH;
    va_list arglist;


    if(level <= self->config.level)
    {
        va_start(arglist, line);
        tlog_make_message(message, &message_len, level, file, line, arglist);
        va_end(arglist);

        tlog_write(self, message, message_len);
    }
}


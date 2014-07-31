#include "tlog_log.h"
#include "tlog_print.h"

#include "tlog.h"

//.a里面的弱符号会在链接时出错, 编译时候使用-fno-common可以避免放入common段
tlog_t g_tlog_instance = {};

void tlog_log(tlog_t *self, tlog_level_t level, const char* file, uint32_t line, ...)
{
	if(tlog_enable(self, level))
	{
		tlog_message_t message;
		va_list arglist;
		va_start(arglist, line);
		tlog_make_message(&message, level, file, line, arglist);
		va_end(arglist);

		tlog_write(self, &message);
	}
}


#include "tlog/tlog.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>


tlog_t g_tlog_instance;




#define DEBUG_LOG(fmt, ...) TLOG_HELPER_LOG(&g_tlog_instance, e_tlog_debug, fmt, __VA_ARGS__)
#define ERROR_LOG(fmt, ...) TLOG_HELPER_LOG(&g_tlog_instance, e_tlog_error, fmt, __VA_ARGS__)

#define DEBUG_PRINTF(fmt, ...) TLOG_HELPER_PRINTF(&g_tlog_instance, e_tlog_debug, fmt, __VA_ARGS__)
#define ERROR_PRINTF(fmt, ...) TLOG_HELPER_PRINTF(&g_tlog_instance, e_tlog_error, fmt, __VA_ARGS__)



int main()
{
	tuint32 i;
	

	tlog_init(&g_tlog_instance, "tlog_config.xml");

	for(i = 0;;++i)
	{
	    DEBUG_PRINTF("hello debug printf, %u", i);
        ERROR_PRINTF("hello error printf, %u", i);
		DEBUG_LOG("hello debug, %u", i);
		ERROR_LOG("hello error, %u", i);

		usleep(1000000);
	}

	return 0;
}

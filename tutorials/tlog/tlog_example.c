#include "tlog/tlog.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>



tlog_t g_tlog_instance;

int main()
{
	tuint32 i;
	
	tlog_init(&g_tlog_instance, "tlog_config.xml");

	for(i = 0;;++i)
	{
		tlog_log(&g_tlog_instance, e_tlog_debug, "hello, %u\n", i);
	}

	return 0;
}

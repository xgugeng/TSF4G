#include "tlog/tlog_instance.h"
#include "tlog/tlog_print.h"


#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main()
{
	uint32_t i;
	

	tlog_init(&g_tlog_instance, "tlog_config.xml");

	for(i = 0;;++i)
	{
	    ERROR_PRINT("hello error printf, %u", i);
        WARN_PRINT("hello warn printf, %u", i);        
	    INFO_PRINT("hello info printf, %u", i);
        DEBUG_PRINT("hello debug printf, %u", i);
        
		DEBUG_LOG("hello debug, %u", i);
		ERROR_LOG("hello error, %u", i);
		usleep(1000000);
	}
	tlog_fini(&g_tlog_instance);

	return 0;
}

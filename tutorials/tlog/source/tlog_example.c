#include "tlog_log.h"
#include "tlog_print.h"


#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
	uint32_t i;
	

	if(tlog_init_from_file(&g_tlog_instance, argv[1]) != E_TS_NOERROR)
	{
	    return 1;
	}

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

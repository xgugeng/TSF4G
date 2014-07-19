#include "tlog_log.h"
#include "tlog_print.h"
#include "tapp.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static tlog_config_t g_config;

int main(int argc, char *argv[])
{
	uint32_t i;

    tapp_load_config(&g_config, argc, argv, (tapp_xml_reader_t)tlibc_read_tlog_config);

	if(tlog_init(&g_tlog_instance, &g_config) != E_TLIBC_NOERROR)
	{
	    return 1;
	}

	for(i = 0;;++i)
	{
/*
	    ERROR_PRINT("hello error printf, %u", i);
        WARN_PRINT("hello warn printf, %u", i);        
	    INFO_PRINT("hello info printf, %u", i);
        DEBUG_PRINT("hello debug printf, %u", i);
*/
		DEBUG_LOG("hello debug, %u", i);
		ERROR_LOG("hello error, %u", i);
		usleep(10000);
//		sleep(1);
	}
	tlog_fini(&g_tlog_instance);

	return 0;
}


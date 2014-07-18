#include "tlog_log.h"
#include "tlog_print.h"
#include "tapp.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

static tlog_config_t g_config;

static uint64_t get_current_ms()
{
	struct timeval tv; 
	gettimeofday(&tv, NULL);

	return (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

int main(int argc, char *argv[])
{
	uint32_t i;
	uint64_t max_log_ms = 0;

    tapp_load_config(&g_config, argc, argv, (tapp_xml_reader_t)tlibc_read_tlog_config);
	

	if(tlog_init(&g_tlog_instance, &g_config) != E_TLIBC_NOERROR)
	{
	    return 1;
	}

	for(i = 0;;++i)
	{
		uint64_t start_ms;
		uint64_t end_ms = 0;
		uint64_t diff_ms = 0;
/*
	    ERROR_PRINT("hello error printf, %u", i);
        WARN_PRINT("hello warn printf, %u", i);        
	    INFO_PRINT("hello info printf, %u", i);
        DEBUG_PRINT("hello debug printf, %u", i);
*/

		start_ms = get_current_ms();
		DEBUG_LOG("hello debug, %u", i);
		ERROR_LOG("hello error, %u", i);
		end_ms = get_current_ms();
		diff_ms = end_ms - start_ms;
		if(diff_ms > max_log_ms)
		{
			max_log_ms = diff_ms;
			printf("max_log_ms: %lu\n", max_log_ms);
		}
//		usleep(10000);
//		sleep(1);
	}
	tlog_fini(&g_tlog_instance);

	return 0;
}


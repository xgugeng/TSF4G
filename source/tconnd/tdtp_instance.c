#include "tlibc/platform/tlibc_platform.h"
#include "tconnd/tdtp_instance.h"
#include "tcommon/tdgi_types.h"
#include "tcommon/tdgi_writer.h"
#include "tcommon/tdgi_reader.h"

#include "tconnd/tdtp_socket.h"
#include "tbus/tbus.h"


#include "tlibc/protocol/tlibc_binary_reader.h"
#include "tlibc/protocol/tlibc_binary_writer.h"
#include "tlibc/core/tlibc_mempool.h"
#include "tlibc/core/tlibc_timer.h"
#include "tlibc/core/tlibc_list.h"


#include "tlog/tlog_instance.h"
#include "tconnd/tconnd_config.h"
#include "tlog/tlog.h"



#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <limits.h>
#include <sched.h>

#include "tconnd_tbus.h"

#include <errno.h>
#include <assert.h>
#include <stdio.h>

#include "tconnd/tconnd_timer.h"
#include "tconnd/tconnd_epoll.h"
#include "tconnd/tconnd_listen.h"
#include "tconnd/tconnd_signal.h"
#include "tconnd/tconnd_mempool.h"

int g_tdtp_instance_switch;

TERROR_CODE tdtp_instance_init(const char* config_file)
{
    if(tconnd_config_init(config_file) != E_TS_NOERROR)
    {
        goto ERROR_RET;
    }

    if(tlog_init(&g_tlog_instance, g_config.log_config) != E_TS_NOERROR)
    {
        INFO_PRINT("tlog init [%s] failed.", g_config.log_config);
        goto ERROR_RET;
    }
    INFO_PRINT("tlog init [%s] succeed.", g_config.log_config);
    INFO_LOG("tlog init [%s] succeed.", g_config.log_config);

    if(signal_processing_init() != E_TS_NOERROR)
    {
        goto ERROR_RET;
    }

    if(tconnd_listen_init() != E_TS_NOERROR)
    {
        goto ERROR_RET;
    }

    if(tconnd_epoll_init() != E_TS_NOERROR)
    {
        goto close_listenfd;
    }

    if(tconnd_tbus_init() != E_TS_NOERROR)
    {
        goto close_epollfd;
    }    

	if(tconnd_mempool_init() != E_TS_NOERROR)
	{
	    goto tbus_fini;
	}

    tconnd_timer_init();
	
    g_tdtp_instance_switch = FALSE;

    INFO_PRINT("tconnd init succeed.");
    INFO_LOG("tconnd init succeed.");
	return E_TS_NOERROR;
tbus_fini:
    tconnd_tbus_fini();
close_epollfd:
    tconnd_epoll_fini();
close_listenfd:
    tconnd_listen_fini();
ERROR_RET:
	return E_TS_ERROR;
}

TERROR_CODE tdtp_instance_process()
{
	TERROR_CODE ret = E_TS_WOULD_BLOCK;
	TERROR_CODE r;

	r = process_listen();
	if(r == E_TS_NOERROR)
	{
		ret = E_TS_NOERROR;
	}
	else if(r != E_TS_WOULD_BLOCK)
	{
		ret = r;
		goto done;
	}

	r = process_epool();
	if(r == E_TS_NOERROR)
	{
		ret = E_TS_NOERROR;
	}
	else if(r != E_TS_WOULD_BLOCK)
	{
		ret = r;
		goto done;
	}

    r = process_input_tbus();
	if(r == E_TS_NOERROR)
	{
		ret = E_TS_NOERROR;
	}
	else if(r != E_TS_WOULD_BLOCK)
	{
		ret = r;
		goto done;
	}

	r = signal_processing_proc();
	if(r == E_TS_NOERROR)
	{
	    ret = E_TS_NOERROR;
	}
	else if(r != E_TS_WOULD_BLOCK)
	{
		ret = r;
		goto done;
	}

	r = tconnd_timer_process();
    if(r == E_TS_NOERROR)
	{
	    ret = E_TS_NOERROR;
	}
	else if(r != E_TS_WOULD_BLOCK)
	{
		ret = r;
		goto done;
	}
	
done:
	return ret;
}


void tdtp_instance_loop()
{
    tuint32 idle_count = 0;
    TERROR_CODE ret;

    g_tdtp_instance_switch = TRUE;
	for(;g_tdtp_instance_switch;)
	{
		ret = tdtp_instance_process();
		switch(ret)
		{
		case E_TS_NOERROR:
    		idle_count = 0;
		    break;
		case E_TS_WOULD_BLOCK:
    		{
    			++idle_count;
    			if(idle_count > 30)
    			{
    				usleep(1000);
    				idle_count = 0;
    			}
    			else
    			{
    				sched_yield();
    			}
	    	}
		    break;
		default:
        	goto done;
		}
	}

done:
	return;	
}

void tdtp_instance_fini()
{
    tconnd_mempool_fini();

    tconnd_epoll_fini();

    tconnd_tbus_fini();

    tconnd_listen_fini();
}


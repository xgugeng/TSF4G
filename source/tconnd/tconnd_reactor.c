#include "tconnd/tconnd_reactor.h"

#include "tconnd/tconnd_config.h"
#include "tconnd/tconnd_timer.h"
#include "tconnd/tconnd_epoll.h"
#include "tconnd/tconnd_listen.h"
#include "tconnd/tconnd_signal.h"
#include "tconnd/tconnd_mempool.h"
#include "tconnd/tconnd_tbus.h"

#include "tlog/tlog_instance.h"

#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <string.h>

int g_tconnd_reactor_switch;

TERROR_CODE tconnd_reactor_init(const char* config_file)
{
    tconnd_timer_init();

    if(tconnd_config_init(config_file) != E_TS_NOERROR)
    {
        goto ERROR_RET;
    }

    if(tlog_init(&g_tlog_instance, g_config.log_config) != E_TS_NOERROR)
    {
        ERROR_PRINT("tlog init [%s] failed.", g_config.log_config);
        goto ERROR_RET;
    }
    INFO_PRINT("tlog init(%s) succeed, check the log file for more information.", g_config.log_config);

    if(signal_processing_init() != E_TS_NOERROR)
    {
        goto tlog_fini;
    }
    
	if(tconnd_mempool_init() != E_TS_NOERROR)
	{
	    goto tlog_fini;
	}
	
    if(tconnd_tbus_init() != E_TS_NOERROR)
    {
        goto mempool_fini;
    }

    if(tconnd_listen_init() != E_TS_NOERROR)
    {
        goto tbus_fini;
    }

    if(tconnd_epoll_init() != E_TS_NOERROR)
    {
        goto listen_fini;
    }

    g_tconnd_reactor_switch = FALSE;

    INFO_PRINT("tconnd init succeed.");
	return E_TS_NOERROR;
	
listen_fini:
    tconnd_listen_fini();
tbus_fini:
    tconnd_tbus_fini();
mempool_fini:
    tconnd_mempool_fini();
tlog_fini:
    tlog_fini(&g_tlog_instance);
ERROR_RET:
	return E_TS_ERROR;
}

TERROR_CODE tconnd_reactor_process()
{
	TERROR_CODE ret = E_TS_WOULD_BLOCK;
	TERROR_CODE r;

	r = tconnd_listen_proc();
	if(r == E_TS_NOERROR)
	{
		ret = E_TS_NOERROR;
	}
	else if(r != E_TS_WOULD_BLOCK)
	{
		ret = r;
		goto done;
	}

	r = tconnd_epool_proc();
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


void tconnd_reactor_loop()
{
    tuint32 idle_count = 0;
    TERROR_CODE ret;

    INFO_LOG("tconnd_reactor_loop begin");
    g_tconnd_reactor_switch = TRUE;
	for(;g_tconnd_reactor_switch;)
	{
		ret = tconnd_reactor_process();
		switch(ret)
		{
		case E_TS_NOERROR:
    		idle_count = 0;
		    break;
		case E_TS_WOULD_BLOCK:
    		{
    			++idle_count;
    			if(idle_count > 10)
    			{
    				if((usleep(10 * 1000) != 0) && (errno != EINTR))
    				{
                        ERROR_LOG("usleep errno [%d], %s", errno, strerror(errno));
    				    goto done;
    				}
    				idle_count = 0;    				
    			}
    		
    			/*else
    			{
    				if((sched_yield() != 0) && (errno != EINTR))
    				{
    				    ERROR_LOG("sched_yield errno [%d], %s", errno, strerror(errno));
    				    goto done;
    				}
    			}
    			*/
	    	}
		    break;
		default:
            ERROR_PRINT("tconnd_reactor_process error.");
        	goto done;
		}
	}	
    INFO_LOG("tconnd_reactor_loop end");
done:
	return;	
}

void tconnd_reactor_fini()
{
    INFO_PRINT("tconnd_reactor_fini.");
    
    tconnd_epoll_fini();
    tconnd_listen_fini();
    tconnd_tbus_fini();
    tconnd_mempool_fini();
    tlog_fini(&g_tlog_instance);
}


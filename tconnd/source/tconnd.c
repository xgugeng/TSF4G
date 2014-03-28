#include "tapp.h"

#include "tconnd_config_reader.h"
#include "tconnd_config_types.h"


#include "tconnd_timer.h"
#include "tconnd_epoll.h"
#include "tconnd_listen.h"
#include "tconnd_mempool.h"
#include "tconnd_tbus.h"
#include "tconnd_socket.h"


#include "tlog_log.h"
#include "tlog_print.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>


tconnd_config_t g_config;

static TERROR_CODE init()
{
    struct sigaction  sa;

    if(tlog_init(&g_tlog_instance, g_config.log_config) != E_TS_NOERROR)
    {
        ERROR_PRINT("tlog init [%s] failed.", g_config.log_config);
        goto ERROR_RET;
    }
    INFO_PRINT("tlog init(%s) succeed, check the log file for more information.", g_config.log_config);



	memset(&sa, 0, sizeof(struct sigaction));
	if(sigemptyset(&sa.sa_mask) != 0)
	{
	    ERROR_LOG("sigemptyset errno[%d], %s.", errno, strerror(errno));
		goto ERROR_RET;
	}
	sa.sa_handler = SIG_IGN;
    if(sigaction(SIGPIPE, &sa, NULL) != 0)
    {
        ERROR_LOG("sigaction error[%d], %s.", errno, strerror(errno));
	    goto ERROR_RET;
    }

    if(tconnd_timer_init() != E_TS_NOERROR)
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
    
    if(tconnd_epoll_init() != E_TS_NOERROR)
    {
        goto tbus_fini;
    }

    if(tconnd_listen_init() != E_TS_NOERROR)
    {
        goto epoll_fini;
    }


    INFO_PRINT("tconnd init succeed.");
    return E_TS_NOERROR;
    
epoll_fini:
    tconnd_epoll_fini();
tbus_fini:
    tconnd_tbus_fini();
mempool_fini:
    tconnd_mempool_fini();
tlog_fini:
    tlog_fini(&g_tlog_instance);
ERROR_RET:
    return E_TS_ERROR;

}

static TERROR_CODE process()
{
    TERROR_CODE ret = E_TS_WOULD_BLOCK;
    TERROR_CODE r;

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

    
    tconnd_timer_process();
    
done:
    return ret;

}

static void fini()
{
    INFO_PRINT("tconnd_reactor_fini.");

    tconnd_listen_fini();
    tconnd_epoll_fini();
    tconnd_tbus_fini();
    tconnd_mempool_fini();
    tlog_fini(&g_tlog_instance);
}

int main(int argc, char **argv)
{
    int ret = 0;
    
    tapp_load_config(&g_config, argc, argv, (tapp_xml_reader_t)tlibc_read_tconnd_config_t);

    
	if(init() != E_TS_NOERROR)
	{
		goto ERROR_RET;
	}   

    if(tapp_loop(process, TAPP_IDLE_USEC, TAPP_IDLE_LIMIT, NULL, NULL) == E_TS_NOERROR)
    {
        ret = 0;
    }
    else
    {
        ret = 1;
    }

	fini();

	return 0;
ERROR_RET:
	return 1;
}


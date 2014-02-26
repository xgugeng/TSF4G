#include "tsqld_client.h"
#include <stdio.h>
#include <string.h>
#include "tlog/tlog_print.h"
#include "tlog/tlog_log.h"
#include "tlibc/protocol/tlibc_xml_reader.h"
#include "tsqld_client_tbus.h"
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include "tcommon/terrno.h"
#include "tlibc/core/tlibc_hash.h"

#define PROGRAN_NAME "tsqld_client"
#define TSQLD_VERSION "0.0.1"

static bool g_sig_term;

static void on_signal(int sig)
{
    switch(sig)
    {
        case SIGINT:
        case SIGTERM:
            g_sig_term = true;
            break;
    }
}

static TERROR_CODE init()
{
    TERROR_CODE ret = E_TS_NOERROR;

    struct sigaction  sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = on_signal;
	if(sigemptyset(&sa.sa_mask) != 0)
	{
	    ERROR_LOG("sigemptyset errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
        goto done;
	}

	if((sigaction(SIGTERM, &sa, NULL) != 0)
	 ||(sigaction(SIGINT, &sa, NULL) != 0))
	{
        ERROR_LOG("sigaction error[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
        goto done;
	}
	g_sig_term = false;


	ret = tsqld_client_tbus_init();
	if(ret != E_TS_ERROR)
	{
	    ret = E_TS_ERRNO;
        goto done;
	}

	
	return E_TS_NOERROR;
done:
    return ret;
}

static void work()
{
    uint32_t idle_count = 0;
    TERROR_CODE ret;

    while(!g_sig_term)
    {
        ret = tsqld_client_tbus_proc();
        if(ret == E_TS_NOERROR)
        {
            idle_count = 0;
        }
        else if(ret == E_TS_WOULD_BLOCK)
        {
            ++idle_count;
            if(idle_count > 30)
            {
                if((usleep(1 * 1000) != 0) && (errno != EINTR))
                {
                    ERROR_LOG("usleep errno [%d], %s", errno, strerror(errno));
                    goto done;
                }
                idle_count = 0;                 
            }
        }
        else
        {
            goto done;
        }
    }
done:
    return;
}

static void fini()
{
    tsqld_client_tbus_fini();
}

int main(int argc, char *argv[])
{
    if(init() != E_TS_NOERROR)
    {
        goto ERROR_RET;
    }

	work();
	
	fini();
	
    return 0;
ERROR_RET:
    return 1;
}


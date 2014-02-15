#include "signal_processing.h"
#include "tcommon/terrno.h"

#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include "tconnd/globals.h"

static bool sig_term;

static void on_signal(int sig)
{
    switch(sig)
    {
        case SIGINT:
        case SIGTERM:
            sig_term = true;
            break;
    }
}

TERROR_CODE signal_processing_init()
{
    struct sigaction  sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = on_signal;
	if(sigemptyset(&sa.sa_mask) != 0)
	{
		goto ERROR_RET;
	}
	
	if((sigaction(SIGTERM, &sa, NULL) != 0)
	 ||(sigaction(SIGINT, &sa, NULL) != 0))
	{
	    goto ERROR_RET;
	}

    sig_term = false;
	return E_TS_NOERROR;
ERROR_RET:
	return E_TS_ERROR;
}

TERROR_CODE signal_processing_proc()
{
	if(sig_term)
	{
		g_tdtp_instance_switch = FALSE;
		return E_TS_NOERROR;
	}
	return E_TS_WOULD_BLOCK;
}


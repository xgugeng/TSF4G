#include "tconnd_timer.h"
#include "tconnd/tconnd_reactor.h"
#include "tlibc/core/tlibc_timer.h"
#include "tlog/tlog_instance.h"

#include <sys/time.h>

tlibc_timer_t       g_timer;
static tuint64      timer_start_ms;

static tuint64 get_sys_ms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec*1000 + tv.tv_usec/1000;
}

static tuint64 get_diff_ms()
{
	return get_sys_ms() - timer_start_ms;
}

void tconnd_timer_init()
{
	tlibc_timer_init(&g_timer, 0);
	timer_start_ms = get_sys_ms();
}

TERROR_CODE tconnd_timer_process()
{
    TLIBC_ERROR_CODE tlibc_ret;
    tlibc_ret = tlibc_timer_tick(&g_timer, get_diff_ms());
    if(tlibc_ret == E_TLIBC_NOERROR)
    {
        return E_TS_NOERROR;
    }
    else if(tlibc_ret == E_TLIBC_WOULD_BLOCK)
    {    
        return E_TS_WOULD_BLOCK;
    }
    else
    {
        ERROR_LOG("tlibc_timer_tick return %d.", tlibc_ret);
        return E_TS_ERROR;        
    }    
}


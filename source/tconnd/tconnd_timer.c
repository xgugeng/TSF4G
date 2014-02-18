#include "tconnd_timer.h"
#include "tconnd/tconnd_reactor.h"
#include "tlibc/core/tlibc_timer.h"
#include "tlog/tlog_instance.h"

#include <sys/time.h>
#include <assert.h>

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
    tuint64 current_sys_ms = get_sys_ms();
    if(current_sys_ms >= timer_start_ms)
    {
        return current_sys_ms - timer_start_ms;
    }
    else
    {
        assert(0);
        return tconnd_timer_ms;
    }
}

void tconnd_timer_init()
{
	tlibc_timer_init(&g_timer, 0);
	timer_start_ms = get_sys_ms();
}

TERROR_CODE tconnd_timer_process()
{
    TLIBC_ERROR_CODE tlibc_ret;
    tuint64 current_time_ms = get_diff_ms();    

    tlibc_ret = tlibc_timer_tick(&g_timer, current_time_ms);
    if(tlibc_ret == E_TLIBC_NOERROR)
    {
        DEBUG_LOG("tlibc_timer_tick E_TLIBC_NOERROR [%llu]", current_time_ms);
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


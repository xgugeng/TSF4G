#include "tconnd_timer.h"
#include "tconnd/tconnd_reactor.h"
#include "tlibc/core/tlibc_timer.h"
#include "tlog/tlog_instance.h"

#include <sys/time.h>
#include <assert.h>

tlibc_timer_t       g_timer;
static struct timeval      start_tv;

void tconnd_timer_init()
{
	tlibc_timer_init(&g_timer);
    gettimeofday(&start_tv, NULL);
}

TERROR_CODE tconnd_timer_process()
{
    bool busy = false;
    tuint64      current_time_ms;
    struct timeval      cur_tv;
    gettimeofday(&cur_tv, NULL);
    if(cur_tv.tv_sec >= start_tv.tv_sec)
    {
        current_time_ms = (cur_tv.tv_sec - start_tv.tv_sec) * 1000;
    }
    else
    {
        current_time_ms = 0;
    }

    if(cur_tv.tv_usec >= start_tv.tv_usec)
    {
        current_time_ms += (cur_tv.tv_usec - start_tv.tv_usec) /1000;
    }    

    while(tconnd_timer_ms <= current_time_ms)
    {
        if(tlibc_timer_tick(&g_timer) == E_TLIBC_NOERROR)
        {
            busy = true;
        }
    }
    
    if(busy)
    {
        return E_TS_NOERROR;
    }
    else
    {
        return E_TS_WOULD_BLOCK;
    }
}


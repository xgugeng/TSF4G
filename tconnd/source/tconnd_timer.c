#include "tconnd_timer.h"
#include "tlog_log.h"
#include "tconnd.h"
#include "tconnd_epoll.h"
#include "tconnd_socket.h"
#include "tlibcdef.h"


#include <sys/time.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

uint64_t       g_cur_ticks;

static void tconnd_timer_signal_handler(int signo)
{
    ++g_cur_ticks;
}

TERROR_CODE tconnd_timer_init()
{
    TERROR_CODE ret = E_TS_NOERROR;


    struct sigaction  sa;
    struct itimerval  itv;

    
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = tconnd_timer_signal_handler;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGALRM, &sa, NULL) == -1)
    {
        ERROR_LOG("sigaction return errno [%d], %s.", errno, strerror(errno));
        ret = E_TS_ERROR;
        goto done;
    }
    
    itv.it_interval.tv_sec = (time_t)g_config.interval/ 1000;
    itv.it_interval.tv_usec = (suseconds_t)(g_config.interval % 1000) * 1000;
    itv.it_value.tv_sec = (time_t)g_config.interval / 1000;
    itv.it_value.tv_usec = (suseconds_t)(g_config.interval % 1000 ) * 1000;
    
    if (setitimer(ITIMER_REAL, &itv, NULL) == -1)
    {
        ERROR_LOG("setitimer return errno [%d], %s.", errno, strerror(errno));
        ret = E_TS_ERROR;
        goto done;
    }

    g_cur_ticks = 0;
done:
    return ret;
}

void tconnd_timer_process()
{
    tlibc_list_head_t *iter, *next;
    tconnd_socket_t *s = NULL;
    
    for(iter = g_pending_socket_list.next; iter != &g_pending_socket_list; iter = next)
    {
        s = TLIBC_CONTAINER_OF(iter, tconnd_socket_t, g_pending_socket_list);        
        if(s->pending_ticks > g_cur_ticks)
        {
            break;
        }
        next = iter->next;
        tconnd_socket_delete(s);
    }

   
   for(iter = g_package_socket_list.next; iter != &g_package_socket_list; iter = next)
   {       
       s = TLIBC_CONTAINER_OF(iter, tconnd_socket_t, g_package_socket_list);
       if(s->package_ticks > g_cur_ticks)
       {
            break;
       }
       next = iter->next;
       tconnd_socket_delete(s);
   }
}


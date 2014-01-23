#ifndef _H_TDTP_TIMER_H
#define _H_TDTP_TIMER_H

#include "tlibc/core/tlibc_timer.h"

#include "tconnd/tdtp/timer/tdtp_timer_accept_timeout.h"
/*
 *  存放定时器上下文
*/

typedef struct _tdtp_timer_t
{
    tlibc_timer_entry_t entry;

    tdtp_timer_accept_timeout_t acccept_timeout;    
}tdtp_timer_t;

#endif//_H_TDTP_TIMER_H


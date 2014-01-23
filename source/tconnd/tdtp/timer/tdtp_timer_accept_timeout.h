/*
 *   如果无秒内没有收到接入连接的回复， 那么断开这个连接
*/


#ifndef _H_TDTP_TIMER_ACCEPT_TIMEOUT_H
#define _H_TDTP_TIMER_ACCEPT_TIMEOUT_H

#include "tlibc/platform/tlibc_platform.h"
#include "tlibc/core/tlibc_mempool.h"
#include "tlibc/core/tlibc_timer.h"

#define TDTP_TIMER_ACCEPT_TIME_MS 5000

typedef struct _tdtp_timer_accept_timeout_t
{
    int socketfd;
    tuint64 mid;
    tlibc_mempool_t *mp;
}tdtp_timer_accept_timeout_t;

void tdtp_timer_accept_timeout(const tlibc_timer_entry_t *super);


#endif// _H_TDTP_TIMER_ACCEPT_TIMEOUT_H


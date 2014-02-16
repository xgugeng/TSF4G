#ifndef _H_TCONND_TIMER_H
#define _H_TCONND_TIMER_H

#include "tconnd/tdtp_instance.h"
#include "tlibc/core/tlibc_timer.h"

extern tlibc_timer_t       g_timer;

#define tconnd_timer_ms g_timer.jiffies

void tconnd_timer_init();

TERROR_CODE tconnd_timer_process();

#endif//_H_TCONND_TIMER_H


#ifndef _H_TCONND_TIMER_H
#define _H_TCONND_TIMER_H

#include "tlibc_error_code.h"
#include "tconnd_epoll.h"

#include <stdint.h>

extern uint64_t       			g_cur_ticks;
extern int 			  			g_timer_fd;
extern tconnd_epoll_data_type_t g_timer_etype;


tlibc_error_code_t tconnd_timer_init();

void tconnd_timer_fini();

void tconnd_timer_on_tick();

void tconnd_timer_process();

#endif//_H_TCONND_TIMER_H


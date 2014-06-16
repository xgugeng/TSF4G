#ifndef _H_TCONND_TIMER_H
#define _H_TCONND_TIMER_H

#include "tlibc_error_code.h"
#include <stdint.h>

extern uint64_t       g_cur_ticks;


tlibc_error_code_t tconnd_timer_init();

void tconnd_timer_process();

#endif//_H_TCONND_TIMER_H


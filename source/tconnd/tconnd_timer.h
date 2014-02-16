#ifndef _H_TCONND_TIMER_H
#define _H_TCONND_TIMER_H

#include "tconnd/tdtp_instance.h"

tuint64 get_current_ms();

tuint64 tdtp_instance_get_time_ms(tdtp_instance_t *self);

#endif//_H_TCONND_TIMER_H


#ifndef _H_TCONND_TBUS_H
#define _H_TCONND_TBUS_H

#include "tbus/tbus.h"

extern tbus_t              *g_input_tbus;
extern tbus_t              *g_output_tbus;

TERROR_CODE tconnd_tbus_init();

TERROR_CODE process_input_tbus();

void tconnd_tbus_fini();

#endif//_H_TCONNND_TBUS_H


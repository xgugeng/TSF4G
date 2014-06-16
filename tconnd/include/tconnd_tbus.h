#ifndef _H_TCONND_TBUS_H
#define _H_TCONND_TBUS_H

#include "tbus.h"

extern tbus_t              *g_input_tbus;
extern tbus_t              *g_output_tbus;

tlibc_error_code_t tconnd_tbus_init();

tlibc_error_code_t process_input_tbus();

void tconnd_tbus_fini();

#endif//_H_TCONNND_TBUS_H


#ifndef _H_TCONND_TBUS_H
#define _H_TCONND_TBUS_H

#include "tconnd/tdtp_instance.h"

TERROR_CODE tconnd_tbus_init(tdtp_instance_t *self);

TERROR_CODE process_input_tbus(tdtp_instance_t *self);

void tconnd_tbus_fini(tdtp_instance_t *self);

#endif//_H_TCONNND_TBUS_H


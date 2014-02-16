#ifndef _H_TCONND_LISTEN_H
#define _H_TCONND_LISTEN_H

#include "tconnd/tdtp_instance.h"

TERROR_CODE tconnd_listen_init(tdtp_instance_t *self);

TERROR_CODE process_listen(tdtp_instance_t *self);

void tconnd_listen_fini(tdtp_instance_t *self);

#endif//_H_TCONND_LISTEN_H

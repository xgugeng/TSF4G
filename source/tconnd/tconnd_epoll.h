#ifndef _H_TCONND_EPOLL_H
#define _H_TCONND_EPOLL_H

#include "tconnd/tdtp_instance.h"

TERROR_CODE tconnd_epoll_init(tdtp_instance_t *self);

TERROR_CODE process_epool(tdtp_instance_t *self);

void tconnd_epoll_fini(tdtp_instance_t *self);


#endif//_H_TCONND_EPOLL_H


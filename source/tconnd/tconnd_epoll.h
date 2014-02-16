#ifndef _H_TCONND_EPOLL_H
#define _H_TCONND_EPOLL_H

#include "tcommon/terrno.h"

extern int                 g_epollfd;

TERROR_CODE tconnd_epoll_init();

TERROR_CODE process_epool();

void tconnd_epoll_fini();


#endif//_H_TCONND_EPOLL_H


#ifndef _H_TCONND_EPOLL_H
#define _H_TCONND_EPOLL_H

#include "tcommon/terrno.h"
#include "tlibc/core/tlibc_list.h"

extern int                 g_epollfd;
extern TLIBC_LIST_HEAD g_package_socket_list;

TERROR_CODE tconnd_epoll_init();

TERROR_CODE tconnd_epool_proc();

void tconnd_epoll_fini();


#endif//_H_TCONND_EPOLL_H


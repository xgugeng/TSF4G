#ifndef _H_TCONND_EPOLL_H
#define _H_TCONND_EPOLL_H

#include "tlibc_error_code.h"
#include "core/tlibc_list.h"

extern int                  g_epollfd;
extern tlibc_list_head_t      g_package_socket_list;
extern tlibc_list_head_t      g_pending_socket_list;

TERROR_CODE tconnd_epoll_init();

TERROR_CODE tconnd_epool_proc();

void tconnd_epoll_fini();


#endif//_H_TCONND_EPOLL_H


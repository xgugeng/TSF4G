#ifndef _H_TCONND_EPOLL_H
#define _H_TCONND_EPOLL_H

#include "tlibc_error_code.h"
#include "tlibc_list.h"

extern int                  g_epollfd;
extern tlibc_list_head_t      g_package_socket_list;
extern tlibc_list_head_t      g_pending_socket_list;

typedef enum tconnd_epoll_data_type_e
{
	e_ted_socket = 0,
	e_ted_timer = 1,
}tconnd_epoll_data_type_t;

tlibc_error_code_t tconnd_epoll_init();

tlibc_error_code_t tconnd_epool_proc();

void tconnd_epoll_fini();


#endif//_H_TCONND_EPOLL_H


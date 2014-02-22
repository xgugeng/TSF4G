#ifndef _H_TCONND_MEMPOOL_H
#define _H_TCONND_MEMPOOL_H

#include "tlibc/platform/tlibc_platform.h"
#include "tcommon/terrno.h"
#include "tlibc/core/tlibc_mempool.h"


TERROR_CODE tconnd_mempool_init();


void tconnd_mempool_fini();

#include "tconnd/tconnd_socket.h"



extern tconnd_socket_t *g_socket_list;
extern size_t           g_socket_list_num;
extern TLIBC_LIST_HEAD  g_unused_socket_list;
extern size_t           g_unused_socket_list_num;
extern TLIBC_LIST_HEAD  g_used_socket_list;
extern size_t           g_used_socket_list_num;



extern size_t           g_package_size;
extern size_t           g_package_num;
extern char             *g_package_pool;
extern TLIBC_LIST_HEAD  g_unused_package_list;
extern size_t           g_unused_package_list_num;
extern TLIBC_LIST_HEAD  g_used_package_list;
extern size_t           g_used_package_list_num;





#endif//_H_TCONND_MEMPOOL_H


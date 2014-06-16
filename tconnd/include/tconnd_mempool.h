#ifndef _H_TCONND_MEMPOOL_H
#define _H_TCONND_MEMPOOL_H

#include "terrno.h"
#include "core/tlibc_mempool.h"

extern tlibc_mempool_t g_package_pool;
extern tlibc_mempool_t g_socket_pool;

TERROR_CODE tconnd_mempool_init();

void tconnd_mempool_fini();


#endif//_H_TCONND_MEMPOOL_H


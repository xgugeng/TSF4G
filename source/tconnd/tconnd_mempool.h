#ifndef _H_TCONND_MEMPOOL_H
#define _H_TCONND_MEMPOOL_H

#include "tlibc/platform/tlibc_platform.h"
#include "tcommon/terrno.h"
#include "tlibc/core/tlibc_mempool.h"

extern tlibc_mempool_t g_package_pool;
extern tlibc_mempool_t g_socket_pool;

TERROR_CODE tconnd_mempool_init();

void tconnd_mempool_fini();


#endif//_H_TCONND_MEMPOOL_H


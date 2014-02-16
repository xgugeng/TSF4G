#ifndef _H_TCONND_MEMPOOL_H
#define _H_TCONND_MEMPOOL_H

#include "tlibc/platform/tlibc_platform.h"
#include "tcommon/terrno.h"
#include "tlibc/core/tlibc_mempool.h"


typedef enum _tconnd_mempool_type_e
{
    e_tconnd_socket,
    e_tconnd_package,
}tconnd_mempool_type_e;

tlibc_mempool_t *g_socket_pool;
tlibc_mempool_t *g_package_pool;

TERROR_CODE tconnd_mempool_init();

tuint64 tconnd_mempool_alloc(tconnd_mempool_type_e type);

void* tconnd_mempool_get(tconnd_mempool_type_e type, tuint64 mid);

void tconnd_mempool_free(tconnd_mempool_type_e type, tuint64 mid);

void tconnd_mempool_fini();

#endif//_H_TCONND_MEMPOOL_H


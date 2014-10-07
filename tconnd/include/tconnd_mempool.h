#ifndef _H_TCONND_MEMPOOL_H
#define _H_TCONND_MEMPOOL_H

#include "tlibc_error_code.h"
#include "tlibc_mempool.h"

extern tlibc_mempool_t g_package_pool;
extern tlibc_mempool_t g_socket_pool;

tlibc_error_code_t tconnd_mempool_init();

void tconnd_mempool_fini();


#endif//_H_TCONND_MEMPOOL_H


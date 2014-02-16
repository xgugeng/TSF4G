#include "tconnd/tconnd_mempool.h"
#include "tlibc/core/tlibc_mempool.h"
#include "tconnd/tdtp_socket.h"
#include "globals.h"
#include <assert.h>

static tlibc_mempool_t *socket_pool;
static tuint32 socket_pool_size;
static tlibc_mempool_t *package_pool;
static tuint32 package_pool_size;

TERROR_CODE tconnd_mempool_init()
{
    TERROR_CODE ret = E_TS_NOERROR;

	socket_pool = (tlibc_mempool_t*)malloc(
		TLIBC_MEMPOOL_SIZE(sizeof(tdtp_socket_t), g_config.connections));
	if(socket_pool == NULL)
	{
	    ret = E_TS_ERROR;
        goto done;
	}
	socket_pool_size = tlibc_mempool_init(socket_pool, 
        TLIBC_MEMPOOL_SIZE(sizeof(tdtp_socket_t), g_config.connections)
        , sizeof(tdtp_socket_t));
	assert(socket_pool_size == g_config.connections);



	package_pool = (tlibc_mempool_t*)malloc(
		TLIBC_MEMPOOL_SIZE(sizeof(package_buff_t), MAX_PACKAGE_NUM));
	if(package_pool == NULL)
	{
    	ret = E_TS_ERROR;
		goto done;
	}
	package_pool_size = tlibc_mempool_init(package_pool, 
        TLIBC_MEMPOOL_SIZE(sizeof(package_buff_t), MAX_PACKAGE_NUM)
        , sizeof(package_buff_t));
	assert(package_pool_size == MAX_PACKAGE_NUM);
	
done:
    return ret;
}

tuint64 tconnd_mempool_alloc(tconnd_mempool_type_e type)
{
    switch(type)
    {
        case e_tconnd_socket:
            return tlibc_mempool_alloc(socket_pool);
        case e_tconnd_pakcage:
            return tlibc_mempool_alloc(package_pool);
        default:
            return TLIBC_MEMPOOL_INVALID_INDEX;
    }
}

void* tconnd_mempool_get(tconnd_mempool_type_e type, tuint64 mid)
{
    switch(type)
    {
        case e_tconnd_socket:
            return tlibc_mempool_get(socket_pool, mid);
        case e_tconnd_pakcage:
            return tlibc_mempool_get(package_pool, mid);
        default:
            return NULL;
    }
}

void tconnd_mempool_free(tconnd_mempool_type_e type, tuint64 mid)
{
    switch(type)
    {
        case e_tconnd_socket:
            tlibc_mempool_free(socket_pool, mid);
            break;
        case e_tconnd_pakcage:
            tlibc_mempool_free(package_pool, mid);
            break;
        default:
            return;
    }
}

void tconnd_mempool_fini()
{
    free(socket_pool);
    free(package_pool);
}


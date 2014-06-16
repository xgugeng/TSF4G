#include "tconnd_mempool.h"
#include "core/tlibc_mempool.h"
#include "tconnd_socket.h"
#include "tconnd.h"
#include "tlog_log.h"
#include "tconnd_timer.h"
#include "tlibcdef.h"

#include <stdlib.h>
#include <assert.h>

tlibc_mempool_t g_socket_pool;
tlibc_mempool_t g_package_pool;

TERROR_CODE tconnd_mempool_init()
{
    TERROR_CODE ret = E_TS_NOERROR;

    g_socket_pool.unit_size = sizeof(tconnd_socket_t);
    g_socket_pool.unit_num = g_config.connections;
    if(g_socket_pool.unit_num > (SIZE_MAX / g_socket_pool.unit_size))
    {
        ret = E_TS_NO_MEMORY;
        ERROR_LOG("malloc failed, g_socket_pool.unit_size = %zu, g_socket_pool.unit_num = %zu"
            , g_socket_pool.unit_size, g_socket_pool.unit_num);
        goto done;
    }
    g_socket_pool.pool = (char*)malloc(g_socket_pool.unit_size * g_socket_pool.unit_num);
    if(g_socket_pool.pool == NULL)
    {
        ret = E_TS_NO_MEMORY;        
        ERROR_LOG("malloc failed, g_socket_pool.unit_size = %zu, g_socket_pool.unit_num = %zu"
            , g_socket_pool.unit_size, g_socket_pool.unit_num);
        goto done;
    }    
    tlibc_mempool_init(&g_socket_pool, tconnd_socket_t, mempool_entry, g_socket_pool.pool
        , g_socket_pool.unit_size, g_socket_pool.unit_num);
	INFO_LOG("g_socket_pool.unit_size = %zu, g_socket_pool.unit_num = %zu"
	    , g_socket_pool.unit_size, g_socket_pool.unit_num);
    


    g_package_pool.unit_size = TLIBC_OFFSET_OF(package_buff_t, body) + g_config.package_size;
    g_package_pool.unit_num = g_config.package_connections;
    if(g_package_pool.unit_num > (SIZE_MAX / g_package_pool.unit_size))
    {
        ret = E_TS_NO_MEMORY;
        ERROR_LOG("malloc failed, g_socket_pool.unit_size = %zu, g_socket_pool.unit_num = %zu"
            , g_package_pool.unit_size, g_package_pool.unit_num);
        goto done;
    }
    g_package_pool.pool = (char*)malloc(g_package_pool.unit_size * g_package_pool.unit_num);
    if(g_package_pool.pool == NULL)
    {
        ret = E_TS_NO_MEMORY;        
        ERROR_LOG("malloc failed, g_package_pool.unit_size = %zu, g_package_pool.unit_num = %zu"
            , g_package_pool.unit_size, g_package_pool.unit_num);
        goto done;
    }
    
    tlibc_mempool_init(&g_package_pool, package_buff_t, mempool_entry
        , g_package_pool.pool, g_package_pool.unit_size, g_package_pool.unit_num);

	INFO_LOG("g_package_pool.unit_size = %zu, g_package_pool.unit_num = %zu"
	    , g_package_pool.unit_size, g_package_pool.unit_num);

/*
    socket_used_list_num = SIZE_MAX;
    package_used_list_num = SIZE_MAX;

	TIMER_ENTRY_BUILD(&mempool_log_timeout, 
	    tconnd_timer_ms + TCONND_MEMPOOL_LOG_INTEVAL_MS, tconnd_mempool_log);
	tlibc_timer_push(&g_timer, &mempool_log_timeout);
*/
done:
    return ret;
}

void tconnd_mempool_fini()
{
    free(g_socket_pool.pool);
    free(g_package_pool.pool);
}


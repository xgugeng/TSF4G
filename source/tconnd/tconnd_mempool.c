#include "tconnd/tconnd_mempool.h"
#include "tlibc/core/tlibc_mempool.h"
#include "tconnd/tconnd_socket.h"
#include <assert.h>
#include "tconnd/tconnd_config.h"
#include "tlog/tlog_instance.h"
#include "tconnd/tconnd_timer.h"


tlibc_mempool_t *g_socket_pool;
tlibc_mempool_t *g_package_pool;

static tlibc_timer_entry_t mempool_log_timeout;
static int last_s_total_used;
static int last_p_total_used;

#define TCONND_MEMPOOL_LOG_INTEVAL_MS 10000
static void tconnd_mempool_log(const tlibc_timer_entry_t *super)
{
    TLIBC_UNUSED(super);
    
    if((g_socket_pool->total_used != last_s_total_used) || (g_package_pool->total_used != last_p_total_used))
    {
        last_s_total_used = g_socket_pool->total_used;
        last_p_total_used = g_package_pool->total_used;        
        
        INFO_LOG("g_socket_pool total_used [%d], g_package_pool total_used [%d].", 
            last_s_total_used, last_p_total_used);
    }

    
	TIMER_ENTRY_BUILD(&mempool_log_timeout, 
	    tconnd_timer_ms + TCONND_MEMPOOL_LOG_INTEVAL_MS, tconnd_mempool_log);
	tlibc_timer_push(&g_timer, &mempool_log_timeout);
}

TERROR_CODE tconnd_mempool_init()
{
    TERROR_CODE ret = E_TS_NOERROR;
    tuint32 socket_pool_size;
    tuint32 package_pool_size;

	g_socket_pool = (tlibc_mempool_t*)malloc(
		TLIBC_MEMPOOL_SIZE(sizeof(tconnd_socket_t), g_config.connections));
	if(g_socket_pool == NULL)
	{
	    ERROR_LOG("malloc return NULL.");
	    ret = E_TS_ERROR;
        goto done;
	}
	socket_pool_size = tlibc_mempool_init(g_socket_pool, 
        TLIBC_MEMPOOL_SIZE(sizeof(tconnd_socket_t), g_config.connections)
        , sizeof(tconnd_socket_t));
	assert(socket_pool_size == g_config.connections);



	g_package_pool = (tlibc_mempool_t*)malloc(
		TLIBC_MEMPOOL_SIZE(sizeof(package_buff_t), MAX_PACKAGE_NUM));
	if(g_package_pool == NULL)
	{
        ERROR_LOG("malloc return NULL.");
    	ret = E_TS_ERROR;
		goto done;
	}
	package_pool_size = tlibc_mempool_init(g_package_pool, 
        TLIBC_MEMPOOL_SIZE(sizeof(package_buff_t), MAX_PACKAGE_NUM)
        , sizeof(package_buff_t));
	assert(package_pool_size == MAX_PACKAGE_NUM);


	TIMER_ENTRY_BUILD(&mempool_log_timeout, 
	    tconnd_timer_ms + TCONND_MEMPOOL_LOG_INTEVAL_MS, tconnd_mempool_log);
	tlibc_timer_push(&g_timer, &mempool_log_timeout);

    last_s_total_used = -1;
    last_p_total_used = -1;


done:
    return ret;
}

void* tconnd_mempool_alloc(tconnd_mempool_type_e type)
{
    switch(type)
    {
        case e_tconnd_socket:
            return tlibc_mempool_alloc(g_socket_pool);
        case e_tconnd_package:
            return tlibc_mempool_alloc(g_package_pool);
        default:
            return NULL;
    }
}

void* tconnd_mempool_get(tconnd_mempool_type_e type, tuint64 mid)
{
    switch(type)
    {
        case e_tconnd_socket:
            return tlibc_mempool_mid2ptr(g_socket_pool, mid);
        case e_tconnd_package:
            return tlibc_mempool_mid2ptr(g_package_pool, mid);
        default:
            return NULL;
    }
}

void tconnd_mempool_free(tconnd_mempool_type_e type, void* ptr)
{
    switch(type)
    {
        case e_tconnd_socket:
            tlibc_mempool_free(g_socket_pool, ptr);
            break;
        case e_tconnd_package:
            tlibc_mempool_free(g_package_pool, ptr);
            break;
        default:
            return;
    }
}

void tconnd_mempool_fini()
{
    free(g_socket_pool);
    free(g_package_pool);
}


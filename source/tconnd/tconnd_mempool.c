#include "tconnd/tconnd_mempool.h"
#include "tlibc/core/tlibc_mempool.h"
#include "tconnd/tconnd_socket.h"
#include <assert.h>
#include "tconnd/tconnd_config.h"
#include "tlog/tlog_instance.h"
#include "tconnd/tconnd_timer.h"



static tlibc_timer_entry_t mempool_log_timeout;

#define TCONND_MEMPOOL_LOG_INTEVAL_MS 10000
static void tconnd_mempool_log(const tlibc_timer_entry_t *super)
{
    TLIBC_UNUSED(super);
        
    INFO_LOG("g_used_socket_list_num = %zu, g_socket_list_num = %zu.", 
        g_used_socket_list_num, g_socket_list_num);

    
    INFO_LOG("g_used_package_list_num = %zu, g_package_num = %zu.", 
        g_used_socket_list_num, g_socket_list_num);

	TIMER_ENTRY_BUILD(&mempool_log_timeout, 
	    tconnd_timer_ms + TCONND_MEMPOOL_LOG_INTEVAL_MS, tconnd_mempool_log);
	tlibc_timer_push(&g_timer, &mempool_log_timeout);
}

tconnd_socket_t *g_socket_list;
size_t           g_socket_list_num;
TLIBC_LIST_HEAD  g_unused_socket_list;
size_t           g_unused_socket_list_num;
TLIBC_LIST_HEAD  g_used_socket_list;
size_t           g_used_socket_list_num;


size_t           g_package_size;
size_t           g_package_num;
char             *g_package_pool;
TLIBC_LIST_HEAD  g_unused_package_list;
size_t           g_unused_package_list_num;
TLIBC_LIST_HEAD  g_used_package_list;
size_t           g_used_package_list_num;


TERROR_CODE tconnd_mempool_init()
{
    TERROR_CODE ret = E_TS_NOERROR;
    size_t i;

    g_socket_list_num = g_config.connections;
    g_socket_list = (tconnd_socket_t*)malloc(sizeof(tconnd_socket_t) * g_socket_list_num);
    if(g_socket_list == NULL)
    {
        ret = E_TS_NO_MEMORY;
        goto done;
    }
    tlibc_list_init(&g_unused_socket_list);
    tlibc_list_init(&g_used_socket_list);
    for(i = 0; i < g_socket_list_num; ++i)
    {
        tlibc_list_add_tail(&g_socket_list[i].g_unused_socket_list, &g_unused_socket_list);
    }
    g_unused_socket_list_num = g_socket_list_num;
    g_used_socket_list_num = 0;
	INFO_LOG("sizeof(tconnd_socket_t) = %zu, g_socket_list_num = %zu", sizeof(tconnd_socket_t), g_socket_list_num);



    g_package_size = TLIBC_OFFSET_OF(package_buff_t, body) + g_config.package_size;
    g_package_num = g_config.package_connections;
    
    g_package_pool = (char*)malloc(g_package_size * g_package_num);
    if(g_package_pool == NULL)
    {
        ret = E_TS_NO_MEMORY;
        goto done;
    }
    tlibc_list_init(&g_unused_package_list);
    tlibc_list_init(&g_used_package_list);
    for(i = 0; i < g_package_num; ++i)
    {
        package_buff_t *package = (package_buff_t *)(g_package_pool + g_package_size * i);
        tlibc_list_add_tail(&package->g_unused_package_list, &g_unused_package_list);
    }
    g_unused_package_list_num = g_package_num;
    g_used_package_list_num = 0;
	INFO_LOG("g_package_size = %zu, g_package_num = %zu", g_package_size, g_package_num);


	TIMER_ENTRY_BUILD(&mempool_log_timeout, 
	    tconnd_timer_ms + TCONND_MEMPOOL_LOG_INTEVAL_MS, tconnd_mempool_log);
	tlibc_timer_push(&g_timer, &mempool_log_timeout);

done:
    return ret;
}

void tconnd_mempool_fini()
{
    free(g_socket_list);
    free(g_package_pool);
}


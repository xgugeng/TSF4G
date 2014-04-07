#include "tconnd_tbus.h"

#include "sip.h"
#include "bscp.h"

#include "tconnd_socket.h"
#include "tconnd_mempool.h"
#include "tconnd.h"
#include "tlog_log.h"
#include "tbus.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

tbus_t              *g_input_tbus;
tbus_t              *g_output_tbus;

TERROR_CODE tconnd_tbus_init()
{
    TERROR_CODE ret = E_TS_NOERROR;
    int input_tbusid;
    int output_tbusid;

	input_tbusid = shmget(g_config.input_tbuskey, 0, 0666);
	if(input_tbusid == -1)
	{
	    ERROR_LOG("shmget errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto done;
	}
	g_input_tbus = shmat(input_tbusid, NULL, 0);
	if(g_input_tbus == NULL)
	{
        ERROR_LOG("shmat errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto done;
	}

	output_tbusid = shmget(g_config.output_tbuskey, 0, 0666);
	if(output_tbusid == -1)
	{
        ERROR_LOG("shmget errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto shmdt_input;
	}
	g_output_tbus = shmat(output_tbusid, NULL, 0);
	if(g_output_tbus == NULL)
	{
        ERROR_LOG("shmat errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto shmdt_input;
	}
	

	return E_TS_NOERROR;
shmdt_input:
    shmdt(g_input_tbus);
done:
    return ret;
}

TERROR_CODE process_input_tbus()
{
	TERROR_CODE ret = E_TS_NOERROR;
	char*message, *cur, *message_limit;
	tbus_atomic_size_t message_len = 0;
    TLIBC_LIST_HEAD writable_list;
    TLIBC_LIST_HEAD *iter;
    
    tlibc_list_init(&writable_list);


    
    message_len = tbus_read_begin(g_input_tbus, &message);
    if(message_len == 0)
    {
        ret = E_TS_WOULD_BLOCK;
        goto done;
    }

    message_limit = message + message_len;

    for(cur = message; cur < message_limit;)
    {
        uint32_t i;
        sip_rsp_t *head = NULL;
        ssize_t head_size = 0;
        char* body_addr = NULL;
        size_t body_size = 0;


        head = (sip_rsp_t*)cur;
        head_size = sip_rst_t_decode(cur, message_limit);
        if(head_size < 0)
        {
            ERROR_LOG("can not decode sip_rst_t.");
            goto flush_socket;
        }
        
        if(head->cmd == e_sip_rsp_cmd_send)
        {
            body_size = head->size;
            body_addr = message + head_size;
            if(body_addr + body_size > message_limit)
            {
                ERROR_LOG("sip_rst_t.size out of range.");
                goto flush_socket;
            }
        }
        else
        {
            body_addr = NULL;
            body_size = 0;
        }        
        cur += (size_t)head_size + body_size;

        for(i = 0; i < head->cid_list_num; ++i)
        {
            tconnd_socket_t *socket = NULL;
            if(i >= SIP_BROADCAST_NUM)
            {
                ERROR_LOG("cid [%u] >= SIP_BROADCAST_NUM [%u]", head->cid_list_num, SIP_BROADCAST_NUM);
                break;
            }
            
            if(!tlibc_mempool_id_test(&g_socket_pool, head->cid_list[i].id))
            {
                WARN_LOG("head->cmd = %d [%u, %"PRIu64"] , head->cid_list[i].id[%u] > g_socket_pool->unit_num[%zu]."
                   , head->cmd, head->cid_list[i].id, head->cid_list[i].sn, head->cid_list[i].id, g_socket_pool.unit_num);
                continue;
            }
            socket = (tconnd_socket_t*)tlibc_mempool_id2ptr(&g_socket_pool, head->cid_list[i].id);
            if(!tlibc_mempool_ptr_test(socket, mempool_entry, head->cid_list[i].sn))
            {
                WARN_LOG("socket [%u, %"PRIu64"] head->cmd = %d [%u, %"PRIu64"] mismatch."
                    , head->cid_list[i].id, socket->mempool_entry.sn, head->cmd, head->cid_list[i].id, head->cid_list[i].sn);
                continue;
            }
            
            if(tconnd_socket_push_pkg(socket, head, body_addr, body_size) == E_TS_CLOSE)
            {
                if(socket->writable)
                {
                    socket->writable = FALSE;                
                    tlibc_list_del(&socket->writable_list);
                }
                tconnd_socket_delete(socket);
            }
            else
            {
                if(!socket->writable)
                {
                    socket->writable = TRUE;
                    tlibc_list_add_tail(&socket->writable_list, &writable_list);
                }
            }
        }
    }
    
flush_socket:
    for(iter = writable_list.next; iter != &writable_list; iter = iter->next)
    {
        tconnd_socket_t *socket = TLIBC_CONTAINER_OF(iter, tconnd_socket_t, writable_list);
        TERROR_CODE r = tconnd_socket_flush(socket);        
        socket->writable = FALSE;
        
        if(r == E_TS_CLOSE)
        {
            tconnd_socket_delete(socket);
        }        
    }

    tbus_read_end(g_input_tbus, message_len);    


    DEBUG_LOG("process tbus data [%d].", message_len);
done:
    return ret;
}

void tconnd_tbus_fini()
{
    if(shmdt(g_input_tbus) != 0)
    {
        ERROR_LOG("shmdt errno [%d], %s", errno, strerror(errno));
    }
    
    
    if(shmdt(g_output_tbus) != 0)
    {
        ERROR_LOG("shmdt errno [%d], %s", errno, strerror(errno));
    }    
}


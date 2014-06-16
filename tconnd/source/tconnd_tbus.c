#include "tconnd_tbus.h"
#include "sip.h"
#include "tconnd_socket.h"
#include "tconnd_mempool.h"
#include "tconnd.h"
#include "tlog_log.h"
#include "tbus.h"
#include "tlibcdef.h"

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

#define TCONND_IOV_NUM 65535
TERROR_CODE process_input_tbus()
{
	TERROR_CODE ret = E_TS_NOERROR;

    struct iovec iov[TCONND_IOV_NUM];
    size_t iov_num;    

    tlibc_list_head_t writable_list;
    tlibc_list_head_t *iter;

    tbus_atomic_size_t tbus_head;
    size_t iov_index;
    
    tlibc_list_init(&writable_list);


    iov_num = TCONND_IOV_NUM;
    tbus_head = tbus_read_begin(g_input_tbus, iov, &iov_num);
    if(iov_num == 0)
    {
        if(tbus_head != g_input_tbus->head_offset)
        {
            goto read_end;
        }
        else
        {
            ret = E_TS_WOULD_BLOCK;
            goto done;
        }
    }


    for(iov_index = 0; iov_index < iov_num; ++iov_index)
    {
        uint32_t i;
        const sip_rsp_t *head = NULL;
        size_t head_size = 0;
        char* body_addr = NULL;
        size_t body_size = 0;


        head = (const sip_rsp_t*)iov[iov_index].iov_base;
        head_size = SIZEOF_SIP_RSP_T(head);
        if(iov[iov_index].iov_len < head_size)
        {
            ERROR_LOG("can not decode sip_rst_t.");
            goto flush_socket;
        }
        
        if(head->cmd == e_sip_rsp_cmd_send)
        {
            body_size = head->size;
            body_addr = (char*)iov[iov_index].iov_base + head_size;
            if(head_size + body_size > iov[iov_index].iov_len)
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
                    socket->writable = false;                
                    tlibc_list_del(&socket->writable_list);
                }
                tconnd_socket_delete(socket);
            }
            else
            {
                if(!socket->writable)
                {
                    socket->writable = true;
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
        socket->writable = false;
        
        if(r == E_TS_CLOSE)
        {
            tconnd_socket_delete(socket);
        }        
    }
    
read_end:
    tbus_read_end(g_input_tbus, tbus_head);
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


#include "tconnd_socket.h"
#include "sip.h"
#include "tconnd_timer.h"
#include "tconnd_mempool.h"
#include "tconnd_tbus.h"
#include "tconnd_epoll.h"
#include "tconnd.h"
#include "tconnd_listen.h"
#include "tlog_log.h"
#include "tlog_print.h"
#include "tlibcdef.h"

#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>


void tcond_socket_construct(tconnd_socket_t* self)
{
    self->id = (uint32_t)tlibc_mempool_ptr2id(&g_socket_pool, self);
    self->status = e_tconnd_socket_status_closed;
    self->iov_num = 0;
    self->iov_total_size = 0;
    self->package_buff = NULL;
    

    tlibc_list_init(&self->readable_list);
    tlibc_list_init(&self->writable_list);
    tlibc_list_init(&self->g_package_socket_list);
    tlibc_list_init(&self->g_pending_socket_list);    
    
    self->writable = false;
    self->readable = false;
}

tconnd_socket_t *tconnd_socket_new()
{
    tconnd_socket_t *socket = NULL;

    if(tlibc_mempool_empty(&g_socket_pool))
    {
        goto done;
    }
    
	if(g_socket_pool.sn == tlibc_mempool_invalid_id)
	{
	    ERROR_LOG("g_socket_pool.sn [%"PRIu64"] == g_socket_pool.sn.", g_socket_pool.sn);
	    goto done;
	}
	
	tlibc_mempool_alloc(&g_socket_pool, tconnd_socket_t, mempool_entry, socket);

	tcond_socket_construct(socket);    
done:
    return socket;
}

void tcond_socket_destruct(tconnd_socket_t* self)
{
    if(self->status != e_tconnd_socket_status_closed)
    {
        INFO_LOG("socket [%d, %"PRIu64"] delete state = %d  ", self->id, self->mempool_entry.sn, self->status);
    }

    switch(self->status)
    {
    case e_tconnd_socket_status_closed:
        break;
    case e_tconnd_socket_status_syn_sent:
        if(close(self->socketfd) != 0)
        {
            ERROR_LOG("socket [%d, %"PRIu64"] close errno[%d], %s", self->id, self->mempool_entry.sn, errno, strerror(errno));
        }
        
        tlibc_list_del(&self->g_pending_socket_list);
        break;
    case e_tconnd_socket_status_established:
        if(close(self->socketfd) != 0)
        {
            ERROR_LOG("socket [%d, %"PRIu64"] close errno[%d], %s", self->id, self->mempool_entry.sn, errno, strerror(errno));
        }
        
        if(self->package_buff != NULL)
        {
            tlibc_list_del(&self->g_package_socket_list);
            tlibc_mempool_free(&g_package_pool, package_buff_t, mempool_entry, self->package_buff);
            self->package_buff = NULL;
        }
        
        if(self->readable)
        {
            tlibc_list_del(&self->readable_list);
        }        
        break;
    case e_tconnd_socket_status_listen:
        if(close(self->socketfd) != 0)
        {
            ERROR_LOG("socket [%d, %"PRIu64"] close errno[%d], %s", self->id, self->mempool_entry.sn, errno, strerror(errno));
        }
        
        if(self->readable)
        {
            tlibc_list_del(&self->readable_list);
        }        
        break;
    }
}

void tconnd_socket_delete(tconnd_socket_t *self)
{
    tcond_socket_destruct(self);
    
    tlibc_mempool_free(&g_socket_pool, tconnd_socket_t, mempool_entry, self);
}

tlibc_error_code_t tconnd_socket_flush(tconnd_socket_t *self)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
    ssize_t send_size;
    uint64_t cur_tick;


    if(self->status != e_tconnd_socket_status_established)
    {
        DEBUG_LOG("socket [%d, %"PRIu64"] status != e_tconnd_socket_status_established", self->id, self->mempool_entry.sn);
        ret = E_TLIBC_CLOSE;
        goto done;
    }

    if(self->iov_num == 0)
    {
        goto done;
    }
    
again:
    cur_tick = g_cur_ticks;

    send_size = writev(self->socketfd, self->iov, self->iov_num);

    
    if(send_size < 0)
    {
        if((errno == EINTR) && ((g_cur_ticks != cur_tick)))
        {
            goto again;
        }
        
        if(errno == EAGAIN)
        {
            DEBUG_LOG("socket [%u, %"PRIu64"] writev return [%zi], errno [%d], %s", self->id, self->mempool_entry.sn, send_size, errno, strerror(errno));      
        }
        else
        {
            WARN_LOG("socket [%u, %"PRIu64"] writev return [%zi], errno [%d], %s", self->id, self->mempool_entry.sn, send_size, errno, strerror(errno));      
        }
        ret = E_TLIBC_CLOSE;
        goto done;
    }
    else if((size_t)send_size != self->iov_total_size)
    {
        DEBUG_LOG("socket [%d, %"PRIu64"] writev return [%zi].", self->id, self->mempool_entry.sn, send_size);
        
        ret = E_TLIBC_CLOSE;
        goto done;
    }

    self->iov_num = 0;
    self->iov_total_size = 0;

    
done:
    return ret;
}

static tlibc_error_code_t tconnd_socket_on_cmd_send(tconnd_socket_t *self, void* body, size_t body_size)
{
    tlibc_error_code_t ret = E_TLIBC_NOERROR;
    
    if((body_size > SSIZE_MAX) || (body_size == 0))
    {
        ERROR_LOG("socket [%"PRIu64"] send invalid buff, size = [%zu].", self->mempool_entry.sn, body_size);
        ret = E_TLIBC_CLOSE;
        goto done;
    }
    
    if((self->iov_num >= TCONND_SOCKET_OP_LIST_MAX) || (self->iov_total_size > SSIZE_MAX - body_size))
    {
        ret = tconnd_socket_flush(self);
        if(ret != E_TLIBC_NOERROR)
        {
            goto done;
        }        
        
    }
    assert(self->iov_num < TCONND_SOCKET_OP_LIST_MAX);
    
    self->iov_total_size += body_size;
    self->iov[self->iov_num].iov_base = body;
    self->iov[self->iov_num].iov_len = body_size;
    ++self->iov_num;
done:
    return ret;
}

static tlibc_error_code_t tconnd_socket_on_cmd_accept(tconnd_socket_t *self)
{
    tlibc_error_code_t ret = E_TLIBC_NOERROR;
    struct epoll_event  ev;
    
    if(self->status != e_tconnd_socket_status_syn_sent)
    {
        DEBUG_LOG("socket status[%d] != e_tconnd_socket_status_syn_sent", self->status);
        ret = E_TLIBC_CLOSE;
        goto done;
    }
    tlibc_list_del(&self->g_pending_socket_list);

    memset(&ev, 0, sizeof(ev));
    ev.events = (uint32_t)(EPOLLIN | EPOLLET);
    ev.data.ptr = self;  
    
    if(epoll_ctl(g_epollfd, EPOLL_CTL_ADD, self->socketfd, &ev) == -1)
    {
        DEBUG_LOG("epoll_ctl errno [%d], %s", errno, strerror(errno));
        ret = E_TLIBC_CLOSE;
        goto done;
    }
    else
    {
        DEBUG_LOG("socket [%"PRIu64"] established.", self->mempool_entry.sn);
        
        self->status = e_tconnd_socket_status_established;
    }
    
done:
    return ret;
}

tlibc_error_code_t tconnd_socket_push_pkg(tconnd_socket_t *self, const sip_rsp_t *head, void* body, size_t body_size)
{
    tlibc_error_code_t ret = E_TLIBC_NOERROR;
    
    switch(head->cmd)
    {
    case e_sip_rsp_cmd_send:
        return tconnd_socket_on_cmd_send(self, body, body_size);
    case e_sip_rsp_cmd_accept:
        return tconnd_socket_on_cmd_accept(self);
    case e_sip_rsp_cmd_close:
        {
            DEBUG_LOG("socket [%"PRIu64"] closing.", self->mempool_entry.sn);
            ret = E_TLIBC_CLOSE;
            goto done;
        };
    default:
        ret = E_TLIBC_CLOSE;
        ERROR_LOG("socket [%"PRIu64"] closing when it received the invalid cmd [%d].", self->mempool_entry.sn, head->cmd);
        goto done;

    }

done:
    return ret;
}

tlibc_error_code_t tconnd_socket_recv(tconnd_socket_t *self)
{
    tlibc_error_code_t ret = E_TLIBC_NOERROR;
    char *header_ptr;
    tbus_atomic_size_t header_size;
    char *package_ptr;
    tbus_atomic_size_t package_size;
    char *body_ptr;
    tbus_atomic_size_t body_size;    
    char *limit_ptr;
    char* remain_ptr = NULL;

    
    sip_req_t *pkg;
    tbus_atomic_size_t tbus_size = 0;
    ssize_t r;
    package_buff_t *package_buff = NULL;
    char *iter;


    assert(self->status == e_tconnd_socket_status_established);
    
    header_size = sizeof(sip_req_t);
    if(self->package_buff == NULL)
    {
        package_size = 0;
    }
    else
    {
        package_size = (tbus_atomic_size_t)self->package_buff->size;
        if(package_size != self->package_buff->size)
        {
            ERROR_LOG("package_size [%zu] overflow.", self->package_buff->size);
        }
    }
    
    tbus_size = tbus_send_begin(g_output_tbus, &header_ptr);

    if(tbus_size < header_size + package_size + 1)
    {
        ret = E_TLIBC_TBUS_NOT_ENOUGH_SPACE;
//        WARN_LOG("tbus_send_begin return E_TLIBC_WOULD_BLOCK");
        goto done;
    }


    pkg = (sip_req_t*)header_ptr;
    package_ptr = header_ptr + header_size;
    body_ptr = package_ptr + package_size;
    body_size = tbus_size - header_size - package_size;


    if(tlibc_mempool_empty(&g_package_pool))
    {
        ret = E_TLIBC_NO_MEMORY;
        WARN_LOG("socket [%u, %"PRIu64"] try to receive data with no package buff.", self->id, self->mempool_entry.sn);
        goto done;
    }
    
    r = recv(self->socketfd, body_ptr, (size_t)body_size, 0);

    if((r < 0) && ((errno == EAGAIN) || (errno == EINTR)))
    {
        ret = E_TLIBC_ERRNO;
        goto done;
    }

    if(r <= 0)
    {        
        pkg->cmd = e_sip_req_cmd_recv;
        pkg->cid.id = self->id;
        pkg->cid.sn = self->mempool_entry.sn;
        pkg->size = 0;
        tbus_send_end(g_output_tbus, header_size);
        ret = E_TLIBC_CLOSE;
        DEBUG_LOG("socket [%u, %"PRIu64"] closed by client.", self->id, self->mempool_entry.sn);
        goto done;
    }
    
    if(self->package_buff != NULL)
    {
        DEBUG_LOG("socket [%"PRIu64"] have remain package_buff , size = %zu", self->mempool_entry.sn, self->package_buff->size);

        memcpy(package_ptr, self->package_buff->head, self->package_buff->size);

        tlibc_list_del(&self->g_package_socket_list);
        tlibc_mempool_free(&g_package_pool, package_buff_t, mempool_entry, self->package_buff);
        self->package_buff = NULL;
    }
    limit_ptr = body_ptr + r;
    
    if(limit_ptr < package_ptr + sizeof(bscp_head_t))
    {
        remain_ptr = package_ptr;
    }
    else
    {

        for(iter = package_ptr; iter <= limit_ptr;)
        {        
            bscp_head_t remain_size;
            remain_ptr = iter;
            if((size_t)(limit_ptr - iter) < sizeof(bscp_head_t))
            {
                break;
            }
            remain_size = *(bscp_head_t*)iter;
            remain_size = le16toh(remain_size);


            DEBUG_LOG("receive a pakcage with length [%u]", remain_size);
            if(remain_size > g_config.package_size)
            {
                DEBUG_LOG("package length [%u] > g_config.package_size[%u]"
                    , remain_size, g_config.package_size);
                ret = E_TLIBC_CLOSE;
                goto done;            
            }

            iter += sizeof(bscp_head_t) + remain_size;
        }
    }
    
    if(limit_ptr - remain_ptr > 0)
    {
        tlibc_mempool_alloc(&g_package_pool, package_buff_t, mempool_entry, package_buff);

        package_buff->size = (size_t)(limit_ptr - remain_ptr);
        memcpy(package_buff->head, remain_ptr, package_buff->size);
        self->package_buff = package_buff;
        self->package_ticks = g_cur_ticks + g_config.package_ticks_limit;
        tlibc_list_add_tail(&self->g_package_socket_list, &g_package_socket_list);

        
        DEBUG_LOG("socket [%u, %"PRIu64"] need to cache package buff, size = [%zu].", self->id, self->mempool_entry.sn, self->package_buff->size);
    }


    if(remain_ptr - package_ptr > 0)
    {
        pkg->cmd = e_sip_req_cmd_recv;
        pkg->cid.id = self->id;
        pkg->cid.sn = self->mempool_entry.sn;
        pkg->size = (sip_size_t)(remain_ptr - package_ptr);

        tbus_send_end(g_output_tbus, (tbus_atomic_size_t)(remain_ptr - header_ptr));
        DEBUG_LOG("e_tdgi_cmd_recv sn[%u, %"PRIu64"] size[%u]", pkg->cid.id, pkg->cid.sn, pkg->size);
    }
    
done:
    return ret;
}



#include "tconnd/tconnd_socket.h"
#include <assert.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "tlibc/protocol/tlibc_binary_writer.h"
#include "tlibc/core/tlibc_util.h" 
#include "tcommon/tdgi_writer.h"
#include "tcommon/tdtp.h"
#include "tcommon/tdtp_types.h"

#include "tconnd/tconnd_timer.h"
#include "tconnd/tconnd_mempool.h"
#include "tconnd/tconnd_tbus.h"
#include "tconnd/tconnd_epoll.h"
#include "tconnd/tconnd_config.h"

#include <sys/ioctl.h>
#include <sys/epoll.h>


void tdtp_socket_close_timeout(const tlibc_timer_entry_t *super)
{
    tdtp_socket_t *self = TLIBC_CONTAINER_OF(super, tdtp_socket_t, close_timeout);

    tdtp_socket_delete(self);
}


void tdtp_socket_accept_timeout(const tlibc_timer_entry_t *super)
{
    tdtp_socket_t *self = TLIBC_CONTAINER_OF(super, tdtp_socket_t, accept_timeout);

    tdtp_socket_delete(self);
}


void tdtp_socket_package_timeout(const tlibc_timer_entry_t *super)
{
    tdtp_socket_t *self = TLIBC_CONTAINER_OF(super, tdtp_socket_t, package_timeout);

    tdtp_socket_delete(self);
}

tdtp_socket_t *tdtp_socket_new()
{
    tuint64 mid= tconnd_mempool_alloc(e_tconnd_socket);
    tdtp_socket_t *socket = tconnd_mempool_get(e_tconnd_socket, mid);
	if(socket != NULL)
	{
	    socket->mid = mid;
	    socket->status = e_tdtp_socket_status_closed;
	    socket->op_list.num = 0;
	    socket->package_buff = NULL;
	    tlibc_list_init(&socket->readable_list);
	    tlibc_list_init(&socket->writable_list);

	    
        TIMER_ENTRY_BUILD(&socket->package_timeout, 0, NULL);
        TIMER_ENTRY_BUILD(&socket->close_timeout, 0, NULL);
        TIMER_ENTRY_BUILD(&socket->accept_timeout, 0, NULL);
        socket->writable = FALSE;
        socket->readable = FALSE;
	}
    return socket;
}

void tdtp_socket_delete(tdtp_socket_t *self)
{
    switch(self->status)
    {
    case e_tdtp_socket_status_closed:
        break;
    case e_tdtp_socket_status_syn_sent:
        close(self->socketfd);
        break;
    case e_tdtp_socket_status_established:
        close(self->socketfd);
        break;
    case e_tdtp_socket_status_closing:
        close(self->socketfd);
        break;
    }

    tlibc_timer_pop(&self->close_timeout);
    tlibc_timer_pop(&self->package_timeout);
    tlibc_timer_pop(&self->accept_timeout);
    if(self->readable)
    {
        tlibc_list_del(&self->readable_list);
    }
   
    if(self->package_buff != NULL)
    {
        tconnd_mempool_free(e_tconnd_package, self->package_buff->mid);
    }
    tconnd_mempool_free(e_tconnd_socket, self->mid);
}

TERROR_CODE tdtp_socket_accept(tdtp_socket_t *self, int listenfd)
{
    int nb = 1;
    TERROR_CODE ret = E_TS_NOERROR;
    socklen_t cnt_len;

    memset(&self->socketaddr, 0, sizeof(struct sockaddr_in));
    cnt_len = sizeof(struct sockaddr_in);
    self->socketfd = accept(listenfd, (struct sockaddr *)&self->socketaddr, &cnt_len);

    if(self->socketfd == -1)
    {
        switch(errno)
        {
            case EAGAIN:
                ret = E_TS_WOULD_BLOCK;
                break;
            case EINTR:
                ret = E_TS_WOULD_BLOCK;
                break;
            default:
                fprintf(stderr, "%s.%d: accept failed: %s, errno(%d)\n", __FILE__, __LINE__, strerror(errno), errno);
                ret = E_TS_ERRNO;
                break;
        }
        goto done;
    }
    
	if(ioctl(self->socketfd, FIONBIO, &nb) == -1)
	{
    	ret = E_TS_ERRNO;
		goto done;
	}
    
	TIMER_ENTRY_BUILD(&self->accept_timeout, 
	    tconnd_timer_ms + TDTP_TIMER_ACCEPT_TIME_MS, tdtp_socket_accept_timeout);
	tlibc_timer_push(&g_timer, &self->accept_timeout);
	
	self->status = e_tdtp_socket_status_syn_sent;
done:
    return ret;
}

TERROR_CODE tdtp_socket_process(tdtp_socket_t *self)
{
    TERROR_CODE ret = E_TS_NOERROR;

	tuint32 i, j;
    for(i = 0; i < self->op_list.num;)
    {
        const tdgi_rsp_t *pkg = self->op_list.head[i];
        switch(pkg->cmd)
        {
        case e_tdgi_cmd_accept:
            {
                struct epoll_event 	ev;
                
                if(self->status != e_tdtp_socket_status_syn_sent)
                {
                    ++i;
                    continue;
                }
                tlibc_timer_pop(&self->accept_timeout);

                
                ev.events = EPOLLIN | EPOLLET;
                ev.data.ptr = self;  
                if(epoll_ctl(g_epollfd, EPOLL_CTL_ADD, self->socketfd, &ev) == -1)
                {
                    TIMER_ENTRY_BUILD(&self->close_timeout, 
                        tconnd_timer_ms, tdtp_socket_close_timeout);
                    tlibc_timer_push(&g_timer, &self->close_timeout);
                    self->status = e_tdtp_socket_status_closing;
                }
                else
                {
                    self->status = e_tdtp_socket_status_established;
                }                
                ++i;
                break;
            }
        case e_tdgi_cmd_close:
            {
                if(self->status == e_tdtp_socket_status_closing)
                {
                    ++i;
                    break;
                }
                
                TIMER_ENTRY_BUILD(&self->close_timeout, 
                    tconnd_timer_ms, tdtp_socket_close_timeout);
                tlibc_timer_push(&g_timer, &self->close_timeout);
                self->status = e_tdtp_socket_status_closing;

                ++i;
                break;
            };
        case e_tdgi_cmd_send:
            {
                ssize_t total_size;
                ssize_t send_size;
                
                if(self->status != e_tdtp_socket_status_established)
                {
                    ++i;
                    break;
                }
                
                total_size = 0;
                j= i;
                while((j < self->op_list.num) && (self->op_list.head[j]->cmd == e_tdgi_cmd_send))
                {
                    //在push的时候检查过self->op_list.head[j]->size小于SSIZE_MAX
                    if(total_size > SSIZE_MAX - self->op_list.head[j]->size)
                    {
                        break;
                    }
                    total_size += self->op_list.head[j]->size;
                    ++j;
                }
                send_size = writev(self->socketfd, self->op_list.iov + i, j - i);
                if(send_size != total_size)
                {
                    TIMER_ENTRY_BUILD(&self->close_timeout, 
                        tconnd_timer_ms, tdtp_socket_close_timeout);
                
                
                    tlibc_timer_push(&g_timer, &self->close_timeout);

                    self->status = e_tdtp_socket_status_closing;
                    ret = E_TS_ERROR;
                    goto done;
                }
                i = j;
                break;
            }
        default:
            break;
        }
    }

    self->op_list.num = 0;
done:
    return ret;
}

TERROR_CODE tdtp_socket_push_pkg(tdtp_socket_t *self, const tdgi_rsp_t *head, const char* body, size_t body_size)
{
    TERROR_CODE ret = E_TS_NOERROR;

    if(self->op_list.num >= g_config.iovmax)
    {
        ret = tdtp_socket_process(self);
        if(ret != E_TS_NOERROR)
        {
            goto done;
        }
    }

    assert(self->op_list.num < IOV_MAX);
    
#if TDGI_SIZE_T_MAX >= SSIZE_MAX
    if(head->size > SSIZE_MAX)
    {
        ret = E_TS_ERROR;
        goto done;
    }
#endif

    self->op_list.head[self->op_list.num] = head;
    self->op_list.iov[self->op_list.num].iov_base = (void*)body;
    self->op_list.iov[self->op_list.num].iov_len = body_size;
    ++self->op_list.num;
    
done:
    return ret;
}

TERROR_CODE tdtp_socket_recv(tdtp_socket_t *self)
{
    TERROR_CODE ret = E_TS_NOERROR;
    char *header_ptr;
    size_t header_size;
    char *package_ptr;
    size_t package_size;
    char *body_ptr;
    size_t body_size;    
    char *limit_ptr;

    char* remain_ptr;
    tdtp_size_t remain_size;
    
    tdgi_req_t pkg;
    TLIBC_BINARY_WRITER writer;
    size_t total_size;
    ssize_t r;
    tuint64 package_buff_mid;
    package_buff_t *package_buff = NULL;
    char *iter;


    assert(self->status == e_tdtp_socket_status_established);
    
    header_size = TDGI_REQ_HEAD_SIZE;
    if(self->package_buff == NULL)
    {
        package_size = 0;
    }
    else
    {
        package_size = self->package_buff->buff_size;
    }
    body_size = 1;
    
    total_size = header_size + package_size + body_size;

    ret = tbus_send_begin(g_output_tbus, &header_ptr, &total_size);
    if(ret == E_TS_WOULD_BLOCK)
    {
        goto done;
    }
    else if(ret != E_TS_NOERROR)
    {
        goto done;
    }

    package_ptr = header_ptr + header_size;
    body_ptr = package_ptr + package_size;
    body_size = total_size - header_size - package_size;

    
    package_buff_mid = tconnd_mempool_alloc(e_tconnd_package);
    package_buff = tconnd_mempool_get(e_tconnd_package, package_buff_mid);
    if(package_buff == NULL)
    {
        ret = E_TS_WOULD_BLOCK;
        goto done;
    }

    

    r = recv(self->socketfd, body_ptr, body_size, 0);
    if(r <= 0)
    {
        if((r == 0) || ((errno != EAGAIN) && (errno != EINTR)))
        {        
            tlibc_binary_writer_init(&writer, header_ptr, header_size);
            pkg.cmd = e_tdgi_cmd_recv;
            pkg.mid = self->mid;
            pkg.size = 0;
            if(tlibc_write_tdgi_req_t(&writer.super, &pkg) != E_TLIBC_NOERROR)
            {
                assert(0);
                tconnd_mempool_free(e_tconnd_package, package_buff_mid);
                ret = E_TS_ERROR;
                goto done;
            }
            assert(writer.offset == header_size);
            tbus_send_end(g_output_tbus, header_size);
            ret = E_TS_CLOSE;
        }
        else
        {
            ret = E_TS_ERRNO;
        }
        tconnd_mempool_free(e_tconnd_package, package_buff_mid);
        goto done;
    }
    
    if(self->package_buff != NULL)
    {
        assert(package_size == self->package_buff->buff_size);
        memcpy(package_ptr, self->package_buff->buff, package_size);
        tconnd_mempool_free(e_tconnd_package, self->package_buff->mid);
        self->package_buff = NULL;
        tlibc_timer_pop(&self->package_timeout);
    }
    limit_ptr = body_ptr + r;
    
    if(limit_ptr < package_ptr + sizeof(tdtp_size_t))
    {
        remain_ptr = package_ptr;
        remain_size = limit_ptr - package_ptr;
    }
    else
    {
        for(iter = package_ptr; iter < limit_ptr;)
        {
            remain_size = *(tdtp_size_t*)iter
            TDTP_LITTLE2SIZE(remain_size);
            remain_ptr = iter;
            iter += sizeof(tdtp_size_t) + remain_size;
        }
    }
    
    if(limit_ptr - remain_ptr > 0)
    {
        package_buff->buff_size = limit_ptr - remain_ptr;
        memcpy(package_buff->buff, remain_ptr, limit_ptr - remain_ptr);
        self->package_buff = package_buff;

        
        TIMER_ENTRY_BUILD(&self->package_timeout, 
            tconnd_timer_ms + TDTP_TIMER_PACKAGE_TIME_MS, tdtp_socket_package_timeout);
        tlibc_timer_push(&g_timer, &self->package_timeout);
        assert(self->package_timeout.entry.prev != &self->package_timeout.entry);
    }
    else
    {
        tconnd_mempool_free(e_tconnd_package, package_buff_mid);
    }

    if(remain_ptr - package_ptr > 0)
    {
        pkg.cmd = e_tdgi_cmd_recv;
        pkg.mid = self->mid;
        pkg.size = remain_ptr - package_ptr;

        tlibc_binary_writer_init(&writer, header_ptr, header_size);
        if(tlibc_write_tdgi_req_t(&writer.super, &pkg) != E_TLIBC_NOERROR)
        {
            assert(0);
            ret = E_TS_ERROR;
            goto done;
        }
        tbus_send_end(g_output_tbus, writer.offset + pkg.size);
    }
    
done:
    return ret;
}


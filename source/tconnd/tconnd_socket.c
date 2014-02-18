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
#include "tconnd/tconnd_listen.h"
#include "tlog/tlog_instance.h"

#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>



static void tconnd_socket_accept_timeout(const tlibc_timer_entry_t *super)
{
    tconnd_socket_t *self = TLIBC_CONTAINER_OF(super, tconnd_socket_t, accept_timeout);
    DEBUG_LOG("socket [%llu] accept_timeout", self->mid);
    tconnd_socket_delete(self);
}


static void tconnd_socket_package_timeout(const tlibc_timer_entry_t *super)
{
    tconnd_socket_t *self = TLIBC_CONTAINER_OF(super, tconnd_socket_t, package_timeout);
    DEBUG_LOG("socket [%llu] package_timeout", self->mid);
    tconnd_socket_delete(self);
}

tconnd_socket_t *tconnd_socket_new()
{
    tconnd_socket_t *socket= tconnd_mempool_alloc(e_tconnd_socket);
	if(socket != NULL)
	{
	    socket->mid = tlibc_mempool_ptr2mid(socket);
	    socket->status = e_tconnd_socket_status_closed;
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

void tconnd_socket_delete(tconnd_socket_t *self)
{
    switch(self->status)
    {
    case e_tconnd_socket_status_closed:
        break;
    case e_tconnd_socket_status_syn_sent:
        if(close(self->socketfd) != 0)
        {
            ERROR_LOG("close errno[%d], %s", errno, strerror(errno));
        }
        tlibc_timer_pop(&self->accept_timeout);
        break;
    case e_tconnd_socket_status_established:
        if(close(self->socketfd) != 0)
        {
            ERROR_LOG("close errno[%d], %s", errno, strerror(errno));
        }
        
        if(self->package_buff != NULL)
        {
            tlibc_timer_pop(&self->package_timeout);
            tconnd_mempool_free(e_tconnd_package, self->package_buff);
            self->package_buff = NULL;
        }

        
        if(self->readable)
        {
            tlibc_list_del(&self->readable_list);
        }
        
        break;
    }

    tconnd_mempool_free(e_tconnd_socket, self);
}

TERROR_CODE tconnd_socket_accept(tconnd_socket_t *self)
{
    int nb = 1;
    TERROR_CODE ret = E_TS_NOERROR;
    socklen_t cnt_len;

    memset(&self->socketaddr, 0, sizeof(struct sockaddr_in));
    cnt_len = sizeof(struct sockaddr_in);
    self->socketfd = accept(g_listenfd, (struct sockaddr *)&self->socketaddr, &cnt_len);

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
                ERROR_LOG("accept errno[%d], %s.", errno, strerror(errno));
                ret = E_TS_ERRNO;
                break;
        }
        goto done;
    }
    
	if(ioctl(self->socketfd, FIONBIO, &nb) == -1)
	{
        ERROR_LOG("ioctl errno[%d], %s.", errno, strerror(errno));
    	ret = E_TS_ERRNO;
		goto close_socket;
	}

	TIMER_ENTRY_BUILD(&self->accept_timeout, 
	    tconnd_timer_ms + TDTP_TIMER_ACCEPT_TIME_MS, tconnd_socket_accept_timeout);
	tlibc_timer_push(&g_timer, &self->accept_timeout);
	
	self->status = e_tconnd_socket_status_syn_sent;
	return ret;
close_socket:
    close(self->socketfd);
done:
    return ret;
}

TERROR_CODE tconnd_socket_process(tconnd_socket_t *self)
{
	tuint32 i, j;
	TERROR_CODE ret = E_TS_NOERROR;
	    
    for(i = 0; i < self->op_list.num;)
    {
        const tdgi_rsp_t *pkg = self->op_list.head[i];
        switch(pkg->cmd)
        {
        case e_tdgi_cmd_accept:
            {
                struct epoll_event 	ev;
                
                if(self->status != e_tconnd_socket_status_syn_sent)
                {
                    DEBUG_LOG("socket status[%d] != e_tconnd_socket_status_syn_sent", self->status);
                    ++i;
                    break;
                }
                tlibc_timer_pop(&self->accept_timeout);

                
                ev.events = EPOLLIN | EPOLLET;
                ev.data.ptr = self;  
                if(epoll_ctl(g_epollfd, EPOLL_CTL_ADD, self->socketfd, &ev) == -1)
                {
                    DEBUG_LOG("epoll_ctl errno [%d], %s", errno, strerror(errno));
                    ret = E_TS_CLOSE;
                    goto done;
                }
                else
                {
                    DEBUG_LOG("socket [%llu] established.", self->mid);
                    
                    self->status = e_tconnd_socket_status_established;
                }                
                ++i;
                break;
            }
        case e_tdgi_cmd_close:
            {
                DEBUG_LOG("socket [%llu] closing.", self->mid);
                ret = E_TS_CLOSE;
                goto done;
            };
        case e_tdgi_cmd_send:
            {
                ssize_t total_size;
                ssize_t send_size;
                
                if(self->status != e_tconnd_socket_status_established)
                {
                    DEBUG_LOG("socket [%llu] status != e_tconnd_socket_status_established", self->mid);
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
                    if(send_size < 0)
                    {
                        if((errno != EINTR) && (errno != EAGAIN) && (errno != EPIPE))
                        {
                            WARN_LOG("socket [%llu] writev return [%zi], errno [%d], %s", self->mid, send_size, errno, strerror(errno));
                        }
                        else
                        {
                            DEBUG_LOG("socket [%llu] writev return [%zi], errno [%d], %s", self->mid, send_size, errno, strerror(errno));                            
                        }
                    }
                    else
                    {
                        DEBUG_LOG("socket [%llu] writev return [%zi].", self->mid, send_size);
                    }

                    ret = E_TS_CLOSE;
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

TERROR_CODE tconnd_socket_push_pkg(tconnd_socket_t *self, const tdgi_rsp_t *head, const char* body, size_t body_size)
{
    TERROR_CODE ret;

    if((self->op_list.num >= g_config.iovmax) || (self->op_list.num >= IOV_MAX))
    {
        ret = tconnd_socket_process(self);
        if(ret != E_TS_NOERROR)
        {
            goto done;
        }
    }

    assert(self->op_list.num < IOV_MAX);
    


    self->op_list.head[self->op_list.num] = head;
    self->op_list.iov[self->op_list.num].iov_base = (void*)body;
    self->op_list.iov[self->op_list.num].iov_len = body_size;
    ++self->op_list.num;
done:
    return ret;
}

TERROR_CODE tconnd_socket_recv(tconnd_socket_t *self)
{
    TERROR_CODE ret = E_TS_NOERROR;
    char *header_ptr;
    size_t header_size;
    char *package_ptr;
    size_t package_size;
    char *body_ptr;
    size_t body_size;    
    char *limit_ptr;

    char* remain_ptr = NULL;
    tdtp_size_t remain_size;
    
    tdgi_req_t pkg;
    TLIBC_BINARY_WRITER writer;
    size_t total_size;
    ssize_t r;
    package_buff_t *package_buff = NULL;
    char *iter;


    assert(self->status == e_tconnd_socket_status_established);
    
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
//        WARN_LOG("tbus_send_begin return E_TS_WOULD_BLOCK");
        goto done;
    }
    else if(ret != E_TS_NOERROR)
    {
        ERROR_LOG("tbus_send_begin return [%d]", ret);
        goto done;
    }

    package_ptr = header_ptr + header_size;
    body_ptr = package_ptr + package_size;
    body_size = total_size - header_size - package_size;

    
    package_buff = tconnd_mempool_alloc(e_tconnd_package);
    if(package_buff == NULL)
    {
//        WARN_LOG("tconnd_mempool_alloc(e_tconnd_package) return NULL");
        ret = E_TS_WOULD_BLOCK;
        goto done;
    }

    

    r = recv(self->socketfd, body_ptr, body_size, 0);
    if(g_config.quickack)
    {
        if(setsockopt(self->socketfd, IPPROTO_TCP, TCP_QUICKACK, (int[]){1}, sizeof(int)) != 0)
        {
            WARN_LOG("setsockopt errno [%d], %s.", errno, strerror(errno));
        }
    }
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
                tconnd_mempool_free(e_tconnd_package, package_buff);
                ERROR_LOG("tlibc_write_tdgi_req_t failed.");
                ret = E_TS_ERROR;
                goto done;
            }
            assert(writer.offset == header_size);
            tbus_send_end(g_output_tbus, header_size);
            ret = E_TS_CLOSE;
            DEBUG_LOG("socket [%llu] closed by client.", self->mid);
        }
        else
        {
            ret = E_TS_ERRNO;
        }
        tconnd_mempool_free(e_tconnd_package, package_buff);
        goto done;
    }
    
    if(self->package_buff != NULL)
    {
        DEBUG_LOG("socket [%llu] have remain package_buff , size = %u", self->mid, self->package_buff->buff_size);
        
        assert(package_size == self->package_buff->buff_size);
        memcpy(package_ptr, self->package_buff->buff, package_size);
        tconnd_mempool_free(e_tconnd_package, self->package_buff);
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
            tconnd_timer_ms + TDTP_TIMER_PACKAGE_TIME_MS, tconnd_socket_package_timeout);
        tlibc_timer_push(&g_timer, &self->package_timeout);
        assert(self->package_timeout.entry.prev != &self->package_timeout.entry);
    }
    else
    {
        tconnd_mempool_free(e_tconnd_package, package_buff);
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
            ERROR_LOG("tlibc_write_tdgi_req_t failed.");
            goto done;
        }
        tbus_send_end(g_output_tbus, writer.offset + pkg.size);
        DEBUG_LOG("e_tdgi_cmd_recv mid[%llu] size[%u]", pkg.mid, pkg.size);
    }
    
done:
    return ret;
}


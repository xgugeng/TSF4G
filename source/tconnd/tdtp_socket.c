#include "tconnd/tdtp_socket.h"
#include <assert.h>
#include "globals.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "tlibc/protocol/tlibc_binary_writer.h"
#include "tlibc/core/tlibc_util.h" 
#include "tcommon/tdgi_writer.h"



tdtp_socket_t *tdtp_socket_alloc()
{
    tuint64 mid= tlibc_mempool_alloc(g_tdtp_instance.socket_pool);
    tdtp_socket_t *socket = tlibc_mempool_get(g_tdtp_instance.socket_pool, mid);
	if(socket != NULL)
	{
	    socket->mid = mid;
	    socket->status = e_tdtp_socket_status_closed;
	    socket->op_list.num = 0;
	    socket->package_buff = NULL;
	    tlibc_list_init(&socket->readable_list);
	    tlibc_list_init(&socket->writeable_list);	    
	}
    return socket;
}

void tdtp_socket_free(tdtp_socket_t *self)
{
    switch(self->status)
    {
    case e_tdtp_socket_status_closed:
        break;
    case e_tdtp_socket_status_syn_sent:
    	tlibc_timer_pop(&self->accept_timeout);
        close(self->socketfd);
        break;
    case e_tdtp_socket_status_established:
        close(self->socketfd);
    case e_tdtp_socket_status_closing:
        close(self->socketfd);
        break;
    }
    if(self->package_buff != NULL)
    {
        tlibc_mempool_free(g_tdtp_instance.package_pool, self->package_buff->mid);
    }
    tlibc_mempool_free(g_tdtp_instance.socket_pool, self->mid);
}

void tdtp_socket_accept_timeout(const tlibc_timer_entry_t *super)
{
    tdtp_socket_t *self = TLIBC_CONTAINER_OF(super, tdtp_socket_t, accept_timeout);
    
    close(self->socketfd);
    tlibc_mempool_free(g_tdtp_instance.socket_pool, self->mid);
}

void tdtp_socket_async_close(const tlibc_timer_entry_t *super)
{
    tdtp_socket_t *self = TLIBC_CONTAINER_OF(super, tdtp_socket_t, close_timeout);

    tdtp_socket_free(self);
}


TERROR_CODE tdtp_socket_accept(tdtp_socket_t *self, int listenfd)
{
    TERROR_CODE ret = E_TS_ERROR;
    socklen_t cnt_len;

    memset(&self->socketaddr, 0, sizeof(struct sockaddr_in));
    cnt_len = sizeof(struct sockaddr_in);
    self->socketfd = accept(listenfd, (struct sockaddr *)&self->socketaddr, &cnt_len);

    if(self->socketfd == -1)
    {
        switch(errno)
        {
            case EAGAIN:
                ret = E_TS_AGAIN;
                break;
            case EINTR:
                ret = E_TS_AGAIN;
                break;
            default:
                fprintf(stderr, "%s.%d: accept failed: %s, errno(%d)\n", __FILE__, __LINE__, strerror(errno), errno);
                ret = E_TS_AGAIN;
                break;
        }
        goto done;
    }

    
	TIMER_ENTRY_BUILD(&self->accept_timeout, 
	    g_tdtp_instance.timer.jiffies + TDTP_TIMER_ACCEPT_TIME_MS, tdtp_socket_accept_timeout);

	
	tlibc_timer_push(&g_tdtp_instance.timer, &self->accept_timeout);
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
        const tdgi_t *pkg = self->op_list.head[i];
        switch(pkg->cmd)
        {
        case e_tdgi_cmd_new_connection_ack:
            {
                if(self->status != e_tdtp_socket_status_syn_sent)
                {
                    ++i;
                    continue;
                }
                tlibc_timer_pop(&self->accept_timeout);
                
                self->status = e_tdtp_socket_status_established;
                ++i;
                continue;
            }
        case e_tdgi_cmd_send:
            {
                size_t total_size;
                size_t send_size;
                
                if(self->status != e_tdtp_socket_status_established)
                {
                    ++i;
                    continue;
                    break;
                }
                if(self->op_list.head[i]->size == 0)
                {
                    TIMER_ENTRY_BUILD(&self->close_timeout, 
                        g_tdtp_instance.timer.jiffies + 0, tdtp_socket_async_close);
                    tlibc_timer_push(&g_tdtp_instance.timer, &self->close_timeout);
                    self->status = e_tdtp_socket_status_closing;
                    goto done;
                }
                
                total_size = 0;
                j= i;
                while((j < self->op_list.num) && (self->op_list.head[j]->cmd == e_tdgi_cmd_send))
                {
                    total_size += self->op_list.head[j]->size;
                    ++j;
                }
                send_size = writev(self->socketfd, self->op_list.iov + i, j - i);
                if(send_size < total_size)
                {
                    TIMER_ENTRY_BUILD(&self->close_timeout, 
                        g_tdtp_instance.timer.jiffies + 0, tdtp_socket_async_close);
                
                
                    tlibc_timer_push(&g_tdtp_instance.timer, &self->close_timeout);

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

TERROR_CODE tdtp_socket_push_pkg(tdtp_socket_t *self, const tdgi_t *head, const char* body, size_t body_size)
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
    tuint16 header_size;
    char *package_ptr;
    tuint16 package_size;
    char *body_ptr;
    tuint16 body_size;    
    char *limit_ptr;

    char* remain_ptr;
    tuint16 remain_size;

    
    tdgi_t pkg;
    TLIBC_BINARY_WRITER writer;
    char pkg_buff[sizeof(tdgi_t)];
    tuint16 total_size;
    int r;
    tuint64 package_buff_mid;
    package_buff_t *package_buff = NULL;
    char *iter;


    assert(self->status == e_tdtp_socket_status_established);
    
    //用个常量?
    tlibc_binary_writer_init(&writer, pkg_buff, sizeof(pkg_buff));
    pkg.cmd = e_tdgi_cmd_recv;
    pkg.mid_num = 1;
    tlibc_write_tdgi_t(&writer.super, &pkg);

    
    header_size = writer.offset;
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

    ret = tbus_send_begin(g_tdtp_instance.output_tbus, &header_ptr, &total_size);
    if((ret == E_TS_WOULD_BLOCK) || (ret == E_TS_AGAIN))
    {
        ret = E_TS_AGAIN;
        goto done;
    }
    if(ret == E_TS_NOERROR)
    {
    }
    else
    {
        ret = E_TS_ERROR;
        goto done;
    }

    package_ptr = header_ptr + header_size;
    body_ptr = package_ptr + package_size;
    body_size = total_size - header_size - package_size;

    
    package_buff_mid = tlibc_mempool_alloc(g_tdtp_instance.package_pool);
    package_buff = tlibc_mempool_get(g_tdtp_instance.package_pool, package_buff_mid);
    if(package_buff == NULL)
    {
        ret = E_TS_AGAIN;
        goto done;
    }

    

    r = recv(self->socketfd, body_ptr, body_size, 0);
    if(r <= 0)
    {
        switch(errno)
        {
            case EAGAIN:
                ret = E_TS_AGAIN;
                break;
            case EINTR:
                ret = E_TS_AGAIN;
                break;
            default:
                tlibc_binary_writer_init(&writer, header_ptr, header_size);
                pkg.cmd = e_tdgi_cmd_recv;
                pkg.mid[0] = self->mid;
                pkg.mid_num = 1;
                pkg.size = 0;
                if(tlibc_write_tdgi_t(&writer.super, &pkg) != E_TLIBC_NOERROR)
                {
                    assert(0);
                    tlibc_mempool_free(g_tdtp_instance.package_pool, package_buff_mid);
                    ret = E_TS_ERROR;
                    goto done;
                }
                tbus_send_end(g_tdtp_instance.output_tbus, header_size);
                ret = E_TS_ERROR;
                break;
        }
        tlibc_mempool_free(g_tdtp_instance.package_pool, package_buff_mid);
        goto done;
    }
    
    if(self->package_buff != NULL)
    {
        assert(package_size == self->package_buff->buff_size);
        memcpy(package_ptr, self->package_buff->buff, package_size);
        tlibc_mempool_free(g_tdtp_instance.package_pool, self->package_buff->mid);
        self->package_buff = NULL;
    }
    limit_ptr = body_ptr + r;
    
    //计算末尾残余的字节数
    for(iter = package_ptr; iter < limit_ptr;)
    {
        remain_size = tlibc_little_to_host16(*(tuint16*)iter);
        iter += sizeof(tuint16) + remain_size;
    }
    remain_ptr = limit_ptr - remain_size;
    package_buff->buff_size = limit_ptr - remain_ptr;
    memcpy(package_buff, remain_ptr, limit_ptr - remain_ptr);
    self->package_buff = package_buff;

    pkg.cmd = e_tdgi_cmd_recv;
    pkg.mid[0] = self->mid;
    pkg.mid_num = 1;
    pkg.size = remain_ptr - package_ptr;

    tlibc_binary_writer_init(&writer, header_ptr, header_size);
    if(tlibc_write_tdgi_t(&writer.super, &pkg) != E_TLIBC_NOERROR)
    {
        assert(0);
        tlibc_mempool_free(g_tdtp_instance.package_pool, package_buff_mid);
        ret = E_TS_ERROR;
        goto done;
    }
    tbus_send_end(g_tdtp_instance.output_tbus, header_size  + package_size + body_size);
    
done:
    return ret;
}


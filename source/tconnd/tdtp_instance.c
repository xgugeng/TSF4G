#include "tlibc/platform/tlibc_platform.h"
#include "tconnd/tdtp_instance.h"
#include "tcommon/tdgi_types.h"
#include "tcommon/tdgi_writer.h"
#include "tcommon/tdgi_reader.h"

#include "tconnd/tdtp_socket.h"
#include "tbus/tbus.h"


#include "tlibc/protocol/tlibc_binary_reader.h"
#include "tlibc/protocol/tlibc_binary_writer.h"
#include "tlibc/core/tlibc_mempool.h"
#include "tlibc/core/tlibc_timer.h"
#include "tlibc/core/tlibc_list.h"

#include "tconnd/signal_processing.h"
#include "tconnd/globals.h"
#include "tconnd/tconnd_config.h"



#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <fcntl.h>
#include <limits.h>
#include <sched.h>



#include <errno.h>
#include <assert.h>
#include <stdio.h>

static tuint64 _get_current_ms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec*1000 + tv.tv_usec/1000;
}

tuint64 tdtp_instance_get_time_ms(tdtp_instance_t *self)
{
	return _get_current_ms() - self->timer_start_ms;
}


TERROR_CODE tdtp_instance_init(tdtp_instance_t *self)
{
    int nb = 1;    
    if(tconnd_config_init(g_config_file) != E_TS_NOERROR)
    {
        goto ERROR_RET;
    }

    g_tdtp_instance_switch = FALSE;
    //warning!
    g_head_size = 16;

    if(signal_processing_init() != E_TS_NOERROR)
    {
        goto ERROR_RET;
    }
    

	self->listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(self->listenfd == -1)
	{
		goto ERROR_RET;
	}

	
	if(ioctl(self->listenfd, FIONBIO, &nb) == -1)
	{	    
		goto close_listenfd;
	}

	
	memset(&self->listenaddr, 0, sizeof(struct sockaddr_in));

	self->listenaddr.sin_family=AF_INET;
	self->listenaddr.sin_addr.s_addr = inet_addr(g_config.ip);
	self->listenaddr.sin_port = htons(g_config.port);

	if(bind(self->listenfd,(struct sockaddr *)(&self->listenaddr), sizeof(struct sockaddr_in)) == -1)
	{
		goto close_listenfd;
	}	

	if(listen(self->listenfd, g_config.backlog) == -1)
	{
		goto close_listenfd;
	}


	self->epollfd = epoll_create(g_config.connections);
	if(self->epollfd == -1)
	{
		goto close_epollfd;
	}

	self->input_tbusid = shmget(g_config.input_tbuskey, 0, 0666);
	if(self->input_tbusid == -1)
	{
		goto close_epollfd;
	}
	self->input_tbus = shmat(self->input_tbusid, NULL, 0);
	if(self->input_tbus == NULL)
	{
		goto close_epollfd;
	}

	self->output_tbusid = shmget(g_config.output_tbuskey, 0, 0666);
	if(self->output_tbusid == -1)
	{
		goto input_dt;
	}
	self->output_tbus = shmat(self->output_tbusid, NULL, 0);
	if(self->output_tbus == NULL)
	{
		goto input_dt;
	}

	tlibc_timer_init(&self->timer, 0);
	self->timer_start_ms = _get_current_ms();

	self->socket_pool = (tlibc_mempool_t*)malloc(
		TLIBC_MEMPOOL_SIZE(sizeof(tdtp_socket_t), g_config.connections));
	if(self->socket_pool == NULL)
	{
		goto output_dt;
	}
	self->socket_pool_size = tlibc_mempool_init(self->socket_pool, 
        TLIBC_MEMPOOL_SIZE(sizeof(tdtp_socket_t), g_config.connections)
        , sizeof(tdtp_socket_t));
	assert(self->socket_pool_size == g_config.connections);



	self->package_pool = (tlibc_mempool_t*)malloc(
		TLIBC_MEMPOOL_SIZE(sizeof(package_buff_t), MAX_PACKAGE_NUM));
	if(self->package_pool == NULL)
	{
		goto free_socket_pool;
	}
	self->package_pool_size = tlibc_mempool_init(self->package_pool, 
        TLIBC_MEMPOOL_SIZE(sizeof(package_buff_t), MAX_PACKAGE_NUM)
        , sizeof(package_buff_t));
	assert(self->package_pool_size == MAX_PACKAGE_NUM);


	tlibc_list_init(&self->readable_list);
	
	return E_TS_NOERROR;
free_socket_pool:
    free(self->socket_pool);
output_dt:
    shmdt(self->output_tbus);
input_dt:
    shmdt(self->input_tbus);
close_epollfd:
    close(self->epollfd);
close_listenfd:
    close(self->listenfd);
ERROR_RET:
	return E_TS_ERROR;
}

static TERROR_CODE process_listen(tdtp_instance_t *self)
{
	int ret = E_TS_NOERROR;
	tdtp_socket_t *conn_socket;
	
	char *tbus_writer_ptr;
	size_t tbus_writer_size;	
	tdgi_req_t pkg;
	TLIBC_BINARY_WRITER writer;

//1, 检查tbus是否能发送新的连接包
	tbus_writer_size = g_head_size;
	ret = tbus_send_begin(self->output_tbus, &tbus_writer_ptr, &tbus_writer_size);
	if(ret == E_TS_WOULD_BLOCK)
	{
		ret = E_TS_WOULD_BLOCK;
		goto done;
	}
	else if(ret != E_TS_NOERROR)
	{
	    goto done;
	}
	
//2, 检查是否能分配socket
	conn_socket = tdtp_socket_alloc();
	if(conn_socket == NULL)
	{
		ret = E_TS_WOULD_BLOCK;
		goto done;
	}

    ret = tdtp_socket_accept(conn_socket, self->listenfd);
	if(ret != E_TS_NOERROR)
	{
    	goto free_socket;
	}	

//5, 发送连接的通知	
	pkg.cmd = e_tdgi_cmd_connect;
	pkg.mid = conn_socket->mid;
	pkg.size = 0;
	
	tlibc_binary_writer_init(&writer, tbus_writer_ptr, tbus_writer_size);
	if(tlibc_write_tdgi_req_t(&writer.super, &pkg) != E_TLIBC_NOERROR)
	{
	    assert(0);
		ret = E_TS_ERROR;
		goto free_socket;
	}
	conn_socket->status = e_tdtp_socket_status_syn_sent;
	tbus_send_end(self->output_tbus, writer.offset);

done:
	return ret;
free_socket:
    tdtp_socket_free(conn_socket);
	return ret;
}

#define TDTP_MAX_EVENTS 1024
static TERROR_CODE process_epool(tdtp_instance_t *self)
{
	int i;
	TERROR_CODE ret = E_TS_NOERROR;
	TLIBC_LIST_HEAD *iter, *next;

	if(tlibc_list_empty(&self->readable_list))
	{
		struct epoll_event 	events[TDTP_MAX_EVENTS];
		int                 events_num;
		
		events_num = epoll_wait(self->epollfd, events, TDTP_MAX_EVENTS, 0);
	    if(events_num == -1)
		{
		    if(errno == EINTR)
		    {
		        ret = E_TS_WOULD_BLOCK;
		    }
		    else
		    {
		        ret = E_TS_ERRNO;
		    }
			goto done;
	    }

	    for(i = 0; i < events_num; ++i)
	    {
            tdtp_socket_t *socket = events[i].data.ptr;
            if(socket->readable)
            {
                assert(0);
                continue;
            }
            socket->readable = TRUE;
            tlibc_list_init(&socket->readable_list);
            tlibc_list_add_tail(&socket->readable_list, &self->readable_list);
	    }
	}

	if(tlibc_list_empty(&self->readable_list))
	{
        ret = E_TS_WOULD_BLOCK;
	    goto done;
	}
	
    for(iter = self->readable_list.next; iter != &self->readable_list; iter = next)
    {
        TERROR_CODE r;
        tdtp_socket_t *socket = TLIBC_CONTAINER_OF(iter, tdtp_socket_t, readable_list);
        next = iter->next;
        
        r = tdtp_socket_recv(socket);
        if(r == E_TS_ERRNO)
        {
            switch(errno)
            {
                case EAGAIN:
                    socket->readable = FALSE;
                    tlibc_list_del(iter);
                    break;
                case EINTR:
                    break;
                default:
                    assert(0);
                    ret = E_TS_ERROR;
                    goto done;
            }
        }
        else if(r == E_TS_WOULD_BLOCK)
        {
            ret = E_TS_WOULD_BLOCK;
            break;
        }        
        else if(r != E_TS_NOERROR)
        {
            socket->readable = FALSE;
            tlibc_list_del(iter);
            tdtp_socket_free(socket);
        }
    }

done:	
	return ret;
}

//tbus中最多一次处理的包的个数
#define MAX_PACKAGE_LIST_NUM 255
static TERROR_CODE process_input_tbus(tdtp_instance_t *self)
{
    static tdgi_rsp_t pkg_list[MAX_PACKAGE_LIST_NUM];
    tuint32 pkg_list_num = 0;
	TERROR_CODE ret = E_TS_NOERROR;
	const char*message;
	size_t message_len;
	TLIBC_BINARY_READER reader;
	size_t len;
	TLIBC_ERROR_CODE r;
	tuint32 i;
    TLIBC_LIST_HEAD writable_list;
    TLIBC_LIST_HEAD *iter, *next;


    ret = tbus_read_begin(self->input_tbus, &message, &message_len);
    if(ret == E_TS_WOULD_BLOCK)
    {
        goto done;
    }
    else if(ret != E_TS_NOERROR)
    {
        goto done;
    }

    len = message_len;
    tlibc_list_init(&writable_list);
    while(len > 0)
    {
        tdgi_rsp_t *pkg = &pkg_list[pkg_list_num];
        size_t pkg_size;
        const char* body_addr;
        size_t body_size;
        
        tlibc_binary_reader_init(&reader, message, len);
        r = tlibc_read_tdgi_rsp_t(&reader.super, pkg);
        if(r != E_TLIBC_NOERROR)
        {
            ret = E_TS_ERROR;
            goto done;
        }
        pkg_size = reader.offset;
        if(pkg->size > TDTP_SIZE_T_MAX)
        {
            ret = E_TS_ERROR;
            goto done;
        }

        for(i = 0; i < pkg->mid_num; ++i)
        {
            tdtp_socket_t *socket = (tdtp_socket_t*)tlibc_mempool_get(self->socket_pool, pkg->mid[i]);
            
            if(pkg->cmd == e_tdgi_cmd_send)
            {
                body_addr = message + reader.offset + pkg->size;
                body_size = pkg->size;
            }
            else
            {
                body_addr = NULL;
                body_size = 0;
            }
            
            if(socket != NULL)
            {
                ++pkg_list_num;
                ret = tdtp_socket_push_pkg(socket, pkg, body_addr, body_size);
                if(ret != E_TS_NOERROR)
                {
                    goto done;
                }

                if(!socket->writable)
                {
                    socket->writable = TRUE;
                    tlibc_list_add_tail(&socket->writable_list, &writable_list);
                }
                
                if(pkg_list_num >= MAX_PACKAGE_LIST_NUM)
                {
                    for(iter = writable_list.next; iter != &writable_list; iter = next)
                    {
                        tdtp_socket_t *s = TLIBC_CONTAINER_OF(iter, tdtp_socket_t, writable_list);
                        next = iter->next;
                        
                        ret = tdtp_socket_process(s);
                        s->writable = FALSE;
                        if(ret != E_TS_NOERROR)
                        {
                            goto done;
                        }
                        tlibc_list_del(iter);
                        tlibc_list_init(&s->writable_list);
                    }
                    pkg_list_num = 0;
                }
            }
        }
        
        message += pkg_size + body_size;
        len -= pkg_size + body_size;
    }
    
    for(iter = writable_list.next; iter != &writable_list; iter = iter->next)
    {        
        tdtp_socket_t *s = TLIBC_CONTAINER_OF(iter, tdtp_socket_t, writable_list);
        ret = tdtp_socket_process(s);
        s->writable = FALSE;
        if(ret != E_TS_NOERROR)
        {
            goto done;
        }
    }
    pkg_list_num = 0;

    tbus_read_end(self->input_tbus, message_len);
    
done:
    return ret;
}


TERROR_CODE tdtp_instance_process(tdtp_instance_t *self)
{
	TERROR_CODE ret = E_TS_WOULD_BLOCK;
	TERROR_CODE r;
	TLIBC_ERROR_CODE tlibc_ret;

	r = process_listen(self);
	if(r == E_TS_NOERROR)
	{
		ret = E_TS_NOERROR;
	}
	else if(r != E_TS_WOULD_BLOCK)
	{
		ret = r;
		goto done;
	}

	r = process_epool(self);
	if(r == E_TS_NOERROR)
	{
		ret = E_TS_NOERROR;
	}
	else if(r != E_TS_WOULD_BLOCK)
	{
		ret = r;
		goto done;
	}

    r = process_input_tbus(self);
	if(r == E_TS_NOERROR)
	{
		ret = E_TS_NOERROR;
	}
	else if(r != E_TS_WOULD_BLOCK)
	{
		ret = r;
		goto done;
	}

	r = signal_processing_proc();
	if(r == E_TS_NOERROR)
	{
	    ret = E_TS_NOERROR;
	}
	else if(r != E_TS_WOULD_BLOCK)
	{
		ret = r;
		goto done;
	}

    tlibc_ret = tlibc_timer_tick(&self->timer, tdtp_instance_get_time_ms(self));
    if(tlibc_ret == E_TLIBC_NOERROR)
    {
        ret = E_TS_NOERROR;
    }
    else if(tlibc_ret != E_TLIBC_WOULD_BLOCK)
    {
        ret = E_TS_ERROR;
        goto done;
    }
	
done:
	return ret;
}


TERROR_CODE tdtp_instance_loop(tdtp_instance_t *self)
{
    tuint32 idle_count = 0;
    TERROR_CODE ret;

    g_tdtp_instance_switch = TRUE;
	for(;g_tdtp_instance_switch;)
	{
		ret = tdtp_instance_process(self);
		switch(ret)
		{
		case E_TS_NOERROR:
    		idle_count = 0;
		    break;
		case E_TS_WOULD_BLOCK:
    		{
    			++idle_count;
    			if(idle_count > 30)
    			{
    				usleep(1000);
    				idle_count = 0;
    			}
    			else
    			{
    				sched_yield();
    			}
	    	}
		    break;
		default:
        	goto done;
		}
	}

done:
	return ret;	
}

void tdtp_instance_fini(tdtp_instance_t *self)
{
    int i;

    shmdt(self->input_tbus);
    shmdt(self->output_tbus);
    
    for(i = self->socket_pool->used_head; i < self->socket_pool->unit_num; )
    {
        tlibc_mempool_block_t *b = TLIBC_MEMPOOL_GET_BLOCK(self->socket_pool, i);
        tdtp_socket_t *s = (tdtp_socket_t *)&b->data;
        tdtp_socket_free(s);

        i = b->next;
    }
        

    free(self->socket_pool);
    free(self->package_pool);

    close(self->listenfd);
    close(self->epollfd);
}


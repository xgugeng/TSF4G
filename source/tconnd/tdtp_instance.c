#include "tlibc/platform/tlibc_platform.h"
#include "tconnd/tdtp_instance.h"
#include "tcommon/tdgi_types.h"
#include "tcommon/tdgi_writer.h"
#include "tcommon/tdgi_reader.h"

#include "tconnd/tdtp_socket.h"
#include "tbus/tbus.h"
#include "tconnd/timer/tdtp_timer.h"

#include "tlibc/protocol/tlibc_binary_reader.h"
#include "tlibc/protocol/tlibc_binary_writer.h"
#include "tlibc/core/tlibc_mempool.h"
#include "tlibc/core/tlibc_timer.h"

#include "tconnd/globals.h"



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

	self->listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(self->listenfd == -1)
	{
		goto ERROR_RET;
	}

	
	if(ioctl(self->listenfd, FIONBIO, &nb) == -1)
	{
		goto ERROR_RET;
	}

	
	memset(&self->listenaddr, 0, sizeof(struct sockaddr_in));

	self->listenaddr.sin_family=AF_INET;
	self->listenaddr.sin_addr.s_addr = inet_addr(g_config.ip);
	self->listenaddr.sin_port = htons(g_config.port);

	if(bind(self->listenfd,(struct sockaddr *)(&self->listenaddr), sizeof(struct sockaddr_in)) == -1)
	{
		goto ERROR_RET;
	}	

	if(listen(self->listenfd, g_config.backlog) == -1)
	{
		goto ERROR_RET;
	}


	self->epollfd = epoll_create(g_config.connections);
	if(self->epollfd == -1)
	{
		goto ERROR_RET;
	}

	self->input_tbusid = shmget(g_config.input_tbuskey, 0, 0666);
	if(self->input_tbusid == -1)
	{
		goto ERROR_RET;
	}
	self->input_tbus = shmat(self->input_tbusid, NULL, 0);
	if(self->input_tbus == NULL)
	{
		goto ERROR_RET;
	}

	self->output_tbusid = shmget(g_config.output_tbuskey, 0, 0666);
	if(self->output_tbusid == -1)
	{
		goto ERROR_RET;
	}
	self->output_tbus = shmat(self->output_tbusid, NULL, 0);
	if(self->output_tbus == NULL)
	{
		goto ERROR_RET;
	}

	tlibc_timer_init(&self->timer, 0);
	self->timer_start_ms = _get_current_ms();

	self->socket_pool = (tlibc_mempool_t*)malloc(
		TLIBC_MEMPOOL_SIZE(sizeof(tdtp_socket_t), g_config.connections));
	if(self->socket_pool == NULL)
	{
		goto ERROR_RET;
	}
	self->socket_pool_size = tlibc_mempool_init(self->socket_pool, 
        TLIBC_MEMPOOL_SIZE(sizeof(tdtp_socket_t), g_config.connections)
        , sizeof(tdtp_socket_t));
	assert(self->socket_pool_size == g_config.connections);


	self->timer_pool = (tlibc_mempool_t*)malloc(
		TLIBC_MEMPOOL_SIZE(sizeof(tdtp_timer_t), g_config.connections));
	if(self->timer_pool == NULL)
	{
		goto ERROR_RET;
	}
	self->timer_pool_size = tlibc_mempool_init(self->timer_pool, 
		TLIBC_MEMPOOL_SIZE(sizeof(tdtp_timer_t), g_config.connections)
		, sizeof(tdtp_timer_t));
	assert(self->timer_pool_size == g_config.connections);

	
	return E_TS_NOERROR;
ERROR_RET:
	return E_TS_ERROR;
}

static TERROR_CODE process_listen(tdtp_instance_t *self)
{
	int nb = 1;
	int ret = E_TS_AGAIN;
	
	struct epoll_event 	ev;
    socklen_t cnt_len;
	
	tuint64 conn_mid = 0;
	tdtp_socket_t *conn_socket;
	
	char *tbus_writer_ptr;
	tuint16 tbus_writer_size;	
	tdgi_t pkg;
	tuint32 pkg_len;
	TLIBC_BINARY_WRITER writer;
	char pkg_buff[sizeof(tdgi_t)];

	tdtp_timer_t *timeout_timer = 0;
	tuint64 timeout_mid;
	struct sockaddr_in socketaddr;


//1, 检查tbus是否能发送新的连接包
	tlibc_binary_writer_init(&writer, pkg_buff, sizeof(pkg_buff));
	pkg.cmd = e_tdgi_cmd_new_connection_req;
	pkg.mid = 0;
	tlibc_write_tdgi_t(&writer.super, &pkg);
	pkg_len = writer.offset;
	tbus_writer_size = pkg_len;
	

	ret = tbus_send_begin(self->output_tbus, &tbus_writer_ptr, &tbus_writer_size);
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
	
//2, 检查是否能分配socket
	conn_mid = tlibc_mempool_alloc(self->socket_pool);
	conn_socket = tlibc_mempool_get(self->socket_pool, conn_mid);
	if(conn_socket == NULL)
	{
		ret = E_TS_AGAIN;
		goto done;
	}


//2, 检查是否能分配超时定时器
	timeout_mid = tlibc_mempool_alloc(self->timer_pool);
	timeout_timer = tlibc_mempool_get(self->timer_pool, timeout_mid);

	if(timeout_timer == NULL)
	{
		ret = E_TS_AGAIN;
		goto free_socket;
	}

    memset(&socketaddr, 0, sizeof(struct sockaddr_in));
    cnt_len = sizeof(struct sockaddr_in);
//3, 接入连接
   	conn_socket->socketfd = accept(self->listenfd,
    							(struct sockaddr *)&socketaddr, &cnt_len);

	if(conn_socket->socketfd == -1)
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
				ret = E_TS_NOERROR;
				break;
		}
		
		goto free_timer;
	}

	if(ioctl(conn_socket->socketfd, FIONBIO, &nb) == -1)
	{
		ret = E_TS_NOERROR;
		goto close_socket;
	}
	
	ev.events = EPOLLIN | EPOLLET;
	ev.data.ptr = conn_socket;	
    if(epoll_ctl(self->epollfd, EPOLL_CTL_ADD, conn_socket->socketfd, &ev) == -1)
	{
		ret = E_TS_NOERROR;
		goto close_socket;
	}

//4, 加入超市定时器
    timeout_timer->acccept_timeout.mid = conn_mid;
    timeout_timer->acccept_timeout.socketfd = conn_socket->socketfd;
    timeout_timer->acccept_timeout.mp = self->socket_pool;
    
    conn_socket->accept_timeout_timer = timeout_mid;


	TIMER_ENTRY_BUILD(&timeout_timer->entry, 
	    self->timer.jiffies + TDTP_TIMER_ACCEPT_TIME_MS, tdtp_timer_accept_timeout);

	
	tlibc_timer_push(&self->timer, &timeout_timer->entry);
//5, 发送连接的通知	
	pkg.cmd = e_tdgi_cmd_new_connection_req;
	pkg.mid = conn_mid;
	
	tlibc_binary_writer_init(&writer, tbus_writer_ptr, tbus_writer_size);
	if(tlibc_write_tdgi_t(&writer.super, &pkg) != E_TLIBC_NOERROR)
	{
		ret = E_TS_NOERROR;
		goto close_socket;
	}
	conn_socket->status = e_tdtp_socket_status_accept;
	tbus_send_end(self->output_tbus, writer.offset);

done:
	return ret;
close_socket:
	close(conn_socket->socketfd);
free_timer:	
    tlibc_mempool_free(self->timer_pool, timeout_mid);
free_socket:
	tlibc_mempool_free(self->socket_pool, conn_mid);
	return ret;
}

static TERROR_CODE process_epool(tdtp_instance_t *self)
{
	int i;
	TERROR_CODE ret = E_TS_AGAIN;
	
	if(self->events_num == 0)
	{
		self->events_num = epoll_wait(self->epollfd, self->events, TDTP_MAX_EVENTS, 0);
	    if(self->events_num == -1)
		{
			goto ERROR_RET;
	    }
	}

	if(self->events_num == 0)
	{
	    goto AGAIN;
	}

	for(i = 0; i < self->events_num; ++i)
	{
//		char *buff;
//		tuint16 buff_size = 1;
		
//		tbus_send_begin();
	}
	
	return ret;
AGAIN:
    return E_TS_AGAIN;
ERROR_RET:
	return E_TS_ERROR;	
}
/*
static TERROR_CODE process_input_pkg_list(tdtp_instance_t *self
    , tdgi_t *pkg_list
    //, const char* pkg_content[]
    , tuint32 pkg_size)
{
    tuint32 i;
    for(i = 0; i < pkg_size; ++i)
    {
        tdtp_socket_t *s = tlibc_mempool_get(self->socket_pool, pkg_list[i].mid);
        if(s == NULL)
        {
            continue;
        }
        //tdtp_socket_push_pkg(s, pkg_list[i], pkg_content[i]);
    }
	return 0;
}
*/
#define ACTIVE_SOCKET_NUM 1024
#define MAX_PACKAGE_LIST_NUM 1024
static TERROR_CODE process_input_tbus(tdtp_instance_t *self)
{
    tdgi_t pkg_list[MAX_PACKAGE_LIST_NUM];
    const char* pkg_content[MAX_PACKAGE_LIST_NUM];
    tuint32 pkg_list_num;

    
	TERROR_CODE ret = E_TS_AGAIN;
	const char*message;
	tuint16 message_len;
	TLIBC_BINARY_READER reader;
	tuint16 len;
	TLIBC_ERROR_CODE r;
	pkg_list_num = 0;
	
    ret = tbus_read_begin(self->input_tbus, &message, &message_len);
    if(ret == E_TS_AGAIN)
    {
        goto AGAIN;
    }
    else if(ret == E_TS_WOULD_BLOCK)
    {
        goto AGAIN;
    }
    else
    {
        goto ERROR_RET;
    }

    assert(ret == E_TS_NOERROR);
    len = message_len;
    while(len > 0)
    {
        tlibc_binary_reader_init(&reader, message, len);
        r = tlibc_read_tdgi_t(&reader.super, &pkg_list[pkg_list_num]);
        if(r != E_TLIBC_NOERROR)
        {
            goto ERROR_RET;
        }
        if(pkg_list_num >= MAX_PACKAGE_LIST_NUM)
        {
            //process_input_pkg_list(self, pkg_list, pkg_content, pkg_list_num);
            pkg_list_num = 0;
        }
        
        if(pkg_list[pkg_list_num].cmd == e_tdgi_cmd_send)
        {
            pkg_content[pkg_list_num] = message + reader.offset;
            message += reader.offset + pkg_list[pkg_list_num].body.send_size;
            len -= reader.offset + pkg_list[pkg_list_num].body.send_size;
        }
        else
        {
            len -= reader.offset;
            message += reader.offset;
        }
        ++pkg_list_num;
    }
    
//    process_input_pkg_list(self, pkg_list, pkg_list_num);
    pkg_list_num = 0;
    tbus_read_end(self->input_tbus, message_len);
    
    ret = E_TS_NOERROR;

//done:
    return ret;
AGAIN:
    return E_TS_AGAIN;
ERROR_RET:
    return E_TS_ERROR;
}

TERROR_CODE tdtp_instance_process(tdtp_instance_t *self)
{
	TERROR_CODE ret = E_TS_AGAIN;
	TERROR_CODE r;

	r = process_listen(self);	

	if(r == E_TS_NOERROR)
	{
		ret = E_TS_NOERROR;
	}
	else if(r == E_TS_AGAIN)
	{
	}
	else
	{
		ret = r;
		goto done;
	}
	
	r = process_epool(self);
	if(r == E_TS_NOERROR)
	{
		ret = E_TS_NOERROR;
	}
	else if(r == E_TS_AGAIN)
	{
	}
	else
	{
		ret = r;
		goto done;
	}


	if(tlibc_timer_tick(&self->timer, tdtp_instance_get_time_ms(self)) == E_TLIBC_NOERROR)
	{
		ret = E_TS_NOERROR;
	}


    r = process_input_tbus(self);
	if(r == E_TS_NOERROR)
	{
		ret = E_TS_NOERROR;
	}
	else if(r == E_TS_AGAIN)
	{
	}
	else
	{
		ret = r;
		goto done;
	}

	
done:
	return ret;
}



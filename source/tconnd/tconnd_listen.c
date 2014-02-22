#include "tconnd_listen.h"
#include "tcommon/terrno.h"
#include "tconnd/tconnd_reactor.h"
#include "tconnd/tconnd_socket.h"
#include "tbus/tbus.h"
#include "tlibc/protocol/tlibc_binary_writer.h"
#include "tcommon/sip.h"
#include "tlibc/core/tlibc_list.h"


#include "tconnd/tconnd_mempool.h"
#include "tconnd/tconnd_tbus.h"
#include "tconnd/tconnd_config.h"
#include "tconnd/tconnd_timer.h"

#include "tlog/tlog_instance.h"

#include <sys/ioctl.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <assert.h>
#include <errno.h>

int g_listenfd;

static void tconnd_socket_accept_timeout(const tlibc_timer_entry_t *super)
{
    tconnd_socket_t *self = TLIBC_CONTAINER_OF(super, tconnd_socket_t, accept_timeout);
    DEBUG_LOG("socket [%u, %llu] accept_timeout", self->id, self->mempool_entry.sn);
    tconnd_socket_delete(self);
}


TERROR_CODE tconnd_listen_init()
{
    TERROR_CODE ret = E_TS_NOERROR;
	struct sockaddr_in  listenaddr;
    int value;

	g_listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(g_listenfd == -1)
	{
	    ERROR_LOG("socket errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto done;
	}

	value = 1;
	if(ioctl(g_listenfd, FIONBIO, &value) == -1)
	{
        ERROR_LOG("ioctl errno[%d], %s.", errno, strerror(errno));
        ret = E_TS_ERRNO;
		goto close_listenfd;
	}
	
	memset(&listenaddr, 0, sizeof(struct sockaddr_in));

	listenaddr.sin_family=AF_INET;
	listenaddr.sin_addr.s_addr = inet_addr(g_config.ip);
	listenaddr.sin_port = htons(g_config.port);

	if(bind(g_listenfd,(struct sockaddr *)(&listenaddr), sizeof(struct sockaddr_in)) == -1)
	{
    	ERROR_LOG("bind errno[%d], %s.", errno, strerror(errno));
        ret = E_TS_ERRNO;
		goto close_listenfd;
	}	

	if(listen(g_listenfd, g_config.backlog) == -1)
	{
        ERROR_LOG("listen errno[%d], %s.", errno, strerror(errno));
        ret = E_TS_ERRNO;
		goto close_listenfd;
	}
	
	if(ioctl(g_listenfd, FIONBIO, &value) == -1)
	{
        ERROR_LOG("ioctl errno[%d], %s.", errno, strerror(errno));
        ret = E_TS_ERRNO;
		goto close_listenfd;
	}

    if (setsockopt(g_listenfd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &g_config.defer_accept, sizeof(g_config.defer_accept)))
    {
        ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
        ret = E_TS_ERRNO;
		goto close_listenfd;
    }



    if(setsockopt(g_listenfd, SOL_SOCKET, SO_SNDBUF, &g_config.sndbuf, sizeof(g_config.sndbuf)) == -1)
	{	
        ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
        ret = E_TS_ERRNO;
        goto close_listenfd;
	}

    if(setsockopt(g_listenfd, SOL_SOCKET, SO_RCVBUF, &g_config.rcvbuf, sizeof(g_config.rcvbuf)) == -1)
	{	
        ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
        ret = E_TS_ERRNO;
        goto close_listenfd;
	}

	

	
    if(setsockopt(g_listenfd, IPPROTO_TCP, TCP_NODELAY, &g_config.nodelay, sizeof(g_config.nodelay)) == -1)
    {    
        ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
        ret = E_TS_ERRNO;
        goto close_listenfd;
    }

    if(setsockopt(g_listenfd, IPPROTO_TCP, TCP_CORK, &g_config.cork, sizeof(g_config.cork)) == -1)
    {    
        ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
        ret = E_TS_ERRNO;
        goto close_listenfd;
    }

    
    if(setsockopt(g_listenfd, SOL_SOCKET, SO_KEEPALIVE, &g_config.keepalive, sizeof(g_config.keepalive)) == -1)
    {   
        ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
        ret = E_TS_ERRNO;
        goto close_listenfd;
    }

	if(g_config.keepalive)
	{
	    if(setsockopt(g_listenfd, IPPROTO_TCP, TCP_KEEPIDLE, &g_config.keepidle, sizeof(g_config.keepidle)) == -1)
    	{	
            ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
            ret = E_TS_ERRNO;
            goto close_listenfd;
    	} 

    	if(setsockopt(g_listenfd, IPPROTO_TCP, TCP_KEEPINTVL, &g_config.keepintvl, sizeof(g_config.keepintvl)) == -1)
    	{	
            ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
            ret = E_TS_ERRNO;
            goto close_listenfd;
    	}

    	
    	if(setsockopt(g_listenfd, IPPROTO_TCP, TCP_KEEPCNT, &g_config.keepcnt, sizeof(g_config.keepcnt)) == -1)
    	{	
            ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
            ret = E_TS_ERRNO;
            goto close_listenfd;
    	}
	}

	goto done;
close_listenfd:
    if(close(g_listenfd) != 0)
    {
        ERROR_LOG("close errno[%d], %s", errno, strerror(errno));
    }

done:
    return ret;
}

TERROR_CODE tconnd_listen_proc()
{
	int ret = E_TS_NOERROR;
	tconnd_socket_t *conn_socket;

    int socketfd;	
    socklen_t cnt_len;
    struct sockaddr_in sockaddr;	
	size_t tbus_writer_size;
	sip_req_t *pkg;
	int nb;

//1, 检查tbus是否能发送新的连接包
	tbus_writer_size = SIP_REQ_SIZE;
	ret = tbus_send_begin(g_output_tbus, (char**)&pkg, &tbus_writer_size);
	if(ret == E_TS_WOULD_BLOCK)
	{
//	    WARN_LOG("tbus_send_begin return E_TS_WOULD_BLOCK");
		ret = E_TS_WOULD_BLOCK;
		goto done;
	}
	else if(ret != E_TS_NOERROR)
	{
        ERROR_LOG("tbus_send_begin return [%d]", ret);
	    goto done;
	}
	
//2, 检查是否能分配socket
    if(tm_over(&g_socket_pool))
    {
        ret = E_TS_ERROR;
        ERROR_LOG("g_socket_pool.sn [%llu] == tm_invalid_id", g_socket_pool.sn);
        goto done;
    }

    if(tm_empty(&g_socket_pool))
    {
        ret = E_TS_WOULD_BLOCK;
        goto done;
    }


    memset(&sockaddr, 0, sizeof(struct sockaddr_in));
    cnt_len = sizeof(struct sockaddr_in);
    socketfd = accept(g_listenfd, (struct sockaddr *)&sockaddr, &cnt_len);

    if(socketfd == -1)
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

    nb = 1;
	if(ioctl(socketfd, FIONBIO, &nb) == -1)
	{
        ERROR_LOG("ioctl errno[%d], %s.", errno, strerror(errno));
    	ret = E_TS_ERRNO;
		goto close_socket;
	}

	conn_socket = tconnd_socket_new();
	conn_socket->socketfd = socketfd;

	TIMER_ENTRY_BUILD(&conn_socket->accept_timeout, 
	    tconnd_timer_ms + g_config.accept_ms_limit, tconnd_socket_accept_timeout);
	tlibc_timer_push(&g_timer, &conn_socket->accept_timeout);
	
	conn_socket->status = e_tconnd_socket_status_syn_sent;



//5, 发送连接的通知	
	pkg->cmd = e_sip_req_cmd_connect;
    pkg->cid.sn = conn_socket->mempool_entry.sn;
	pkg->cid.id = conn_socket->id;
	pkg->size = 0;
	sip_req_t_code(pkg);
	
	conn_socket->status = e_tconnd_socket_status_syn_sent;
	tbus_send_end(g_output_tbus, SIP_REQ_SIZE);
    DEBUG_LOG("[%u, %llu] connect.", pkg->cid.id, pkg->cid.sn);

done:
	return ret;
close_socket:
    close(socketfd);
	return ret;
}


void tconnd_listen_fini()
{
    TLIBC_LIST_HEAD *iter;    

    for(iter = g_socket_pool.mempool_entry.used_list.next; iter != &g_socket_pool.mempool_entry.used_list; iter = iter->next)
    {
        tconnd_socket_t *s = TLIBC_CONTAINER_OF(iter, tconnd_socket_t, mempool_entry.used_list);
        tconnd_socket_delete(s);
    }


    if(close(g_listenfd) != 0)
    {
        ERROR_LOG("close errno[%d], %s", errno, strerror(errno));
    }
}


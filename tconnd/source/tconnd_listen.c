#include "tconnd_listen.h"
#include "tlibc_error_code.h"
#include "tconnd_socket.h"
#include "tbus.h"
#include "protocol/tlibc_binary_writer.h"
#include "sip.h"
#include "core/tlibc_list.h"
#include "tconnd_mempool.h"
#include "tconnd_tbus.h"
#include "tconnd.h"
#include "tconnd_timer.h"
#include "tconnd_epoll.h"
#include "tlog_log.h"
#include "tlibcdef.h"

#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>

tconnd_socket_t g_listen;

tlibc_error_code_t tconnd_listen_init()
{
    tlibc_error_code_t ret = E_TLIBC_NOERROR;
	struct sockaddr_in  listenaddr;
    int value;
    int socketfd;
    struct epoll_event  ev;



	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd == -1)
	{
	    ERROR_LOG("socket errno[%d], %s.", errno, strerror(errno));
	    ret = E_TLIBC_ERRNO;
		goto done;
	}

	value = 1;
	if(ioctl(socketfd, FIONBIO, &value) == -1)
	{
        ERROR_LOG("ioctl errno[%d], %s.", errno, strerror(errno));
        ret = E_TLIBC_ERRNO;
		goto close_listenfd;
	}
	
	memset(&listenaddr, 0, sizeof(struct sockaddr_in));

	listenaddr.sin_family=AF_INET;
	listenaddr.sin_addr.s_addr = inet_addr(g_config.ip);
	listenaddr.sin_port = htons(g_config.port);

	if(bind(socketfd,(struct sockaddr *)(&listenaddr), sizeof(struct sockaddr_in)) == -1)
	{
    	ERROR_LOG("bind errno[%d], %s.", errno, strerror(errno));
        ret = E_TLIBC_ERRNO;
		goto close_listenfd;
	}	

	if(listen(socketfd, g_config.backlog) == -1)
	{
        ERROR_LOG("listen errno[%d], %s.", errno, strerror(errno));
        ret = E_TLIBC_ERRNO;
		goto close_listenfd;
	}
	
	
	if(ioctl(socketfd, FIONBIO, &value) == -1)
	{
        ERROR_LOG("ioctl errno[%d], %s.", errno, strerror(errno));
        ret = E_TLIBC_ERRNO;
		goto close_listenfd;
	}

    if (setsockopt(socketfd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &g_config.defer_accept, sizeof(g_config.defer_accept)))
    {
        ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
        ret = E_TLIBC_ERRNO;
		goto close_listenfd;
    }



    if(setsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, &g_config.sndbuf, sizeof(g_config.sndbuf)) == -1)
	{	
        ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
        ret = E_TLIBC_ERRNO;
        goto close_listenfd;
	}

    if(setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, &g_config.rcvbuf, sizeof(g_config.rcvbuf)) == -1)
	{	
        ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
        ret = E_TLIBC_ERRNO;
        goto close_listenfd;
	}

	

	
    if(setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY, &g_config.nodelay, sizeof(g_config.nodelay)) == -1)
    {    
        ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
        ret = E_TLIBC_ERRNO;
        goto close_listenfd;
    }

    if(setsockopt(socketfd, IPPROTO_TCP, TCP_CORK, &g_config.cork, sizeof(g_config.cork)) == -1)
    {    
        ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
        ret = E_TLIBC_ERRNO;
        goto close_listenfd;
    }

    
    if(setsockopt(socketfd, SOL_SOCKET, SO_KEEPALIVE, &g_config.keepalive, sizeof(g_config.keepalive)) == -1)
    {   
        ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
        ret = E_TLIBC_ERRNO;
        goto close_listenfd;
    }

	if(g_config.keepalive)
	{
	    if(setsockopt(socketfd, IPPROTO_TCP, TCP_KEEPIDLE, &g_config.keepidle, sizeof(g_config.keepidle)) == -1)
    	{	
            ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
            ret = E_TLIBC_ERRNO;
            goto close_listenfd;
    	} 

    	if(setsockopt(socketfd, IPPROTO_TCP, TCP_KEEPINTVL, &g_config.keepintvl, sizeof(g_config.keepintvl)) == -1)
    	{	
            ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
            ret = E_TLIBC_ERRNO;
            goto close_listenfd;
    	}

    	
    	if(setsockopt(socketfd, IPPROTO_TCP, TCP_KEEPCNT, &g_config.keepcnt, sizeof(g_config.keepcnt)) == -1)
    	{	
            ERROR_LOG("setsockopt errno[%d], %s.", errno, strerror(errno));
            ret = E_TLIBC_ERRNO;
            goto close_listenfd;
    	}
	}

    memset(&ev, 0, sizeof(ev));
    ev.events = (uint32_t)(EPOLLIN | EPOLLET);
	g_listen.etype = e_ted_socket;
    ev.data.ptr = &g_listen.etype;  
    if(epoll_ctl(g_epollfd, EPOLL_CTL_ADD, socketfd, &ev) == -1)
    {
        DEBUG_LOG("epoll_ctl errno [%d], %s", errno, strerror(errno));
        ret = E_TLIBC_ERRNO;
        goto close_listenfd;
    }

    tcond_socket_construct(&g_listen);
    g_listen.socketfd = socketfd;
    g_listen.status = e_tconnd_socket_status_listen;

	goto done;
close_listenfd:
    if(close(socketfd) != 0)
    {
        ERROR_LOG("close errno[%d], %s", errno, strerror(errno));
    }

done:
    return ret;
}

tlibc_error_code_t tconnd_listen()
{
	int ret = E_TLIBC_NOERROR;
	tconnd_socket_t *conn_socket;

    int socketfd;	
    socklen_t cnt_len;
    struct sockaddr_in sockaddr;	
	tbus_atomic_size_t tbus_writer_size;
	sip_req_t *pkg;
	int nb;

//1, 检查tbus是否能发送新的连接包
	tbus_writer_size = tbus_send_begin(g_output_tbus, (char**)&pkg);
	if(tbus_writer_size < sizeof(sip_req_t))
	{
//	    WARN_LOG("tbus_send_begin return E_TLIBC_TBUS_NOT_ENOUGH_SPACE");
        ret = E_TLIBC_TBUS_NOT_ENOUGH_SPACE;
		goto done;
	}
	
//2, 检查是否能分配socket
    if(tlibc_mempool_over(&g_socket_pool))
    {
        ERROR_LOG("g_socket_pool.sn [%"PRIu64"] == tm_invalid_id", g_socket_pool.sn);
        ret = E_TLIBC_ERROR;
        goto done;
    }

    if(tlibc_mempool_empty(&g_socket_pool))
    {
        ret = E_TLIBC_TOO_MANY_SOCKET;
        goto done;
    }


    memset(&sockaddr, 0, sizeof(struct sockaddr_in));
    cnt_len = sizeof(struct sockaddr_in);

again:
    socketfd = accept(g_listen.socketfd, (struct sockaddr *)&sockaddr, &cnt_len);

    if(socketfd == -1)
    {
        switch(errno)
        {
            case EAGAIN:
				break;
            case EINTR:
				goto again;
            default:
                ERROR_LOG("accept errno[%d], %s.", errno, strerror(errno));
                break;
        }
        ret = E_TLIBC_ERRNO;
        goto done;
    }

    nb = 1;
	if(ioctl(socketfd, FIONBIO, &nb) == -1)
	{
    	ret = E_TLIBC_ERRNO;
		goto close_socket;
	}

	conn_socket = tconnd_socket_new();
	conn_socket->socketfd = socketfd;
	conn_socket->pending_ticks = g_cur_ticks + g_config.accept_ticks_limit;
	tlibc_list_add_tail(&conn_socket->g_pending_socket_list, &g_pending_socket_list);
	
	conn_socket->status = e_tconnd_socket_status_syn_sent;



//5, 发送连接的通知	
	pkg->cmd = e_sip_req_cmd_connect;
    pkg->cid.sn = conn_socket->mempool_entry.sn;
	pkg->cid.id = conn_socket->id;
	pkg->size = 0;
	
	conn_socket->status = e_tconnd_socket_status_syn_sent;
	tbus_send_end(g_output_tbus, sizeof(sip_req_t));
    DEBUG_LOG("[%u, %"PRIu64"] connect.", pkg->cid.id, pkg->cid.sn);

done:
	return ret;
close_socket:
    if(close(socketfd) != 0)
    {
        ERROR_LOG("close [%d] return errno [%d], %s.", socketfd, errno, strerror(errno));
    }
	return ret;
}


void tconnd_listen_fini()
{
    tlibc_list_head_t *iter;
    tcond_socket_destruct(&g_listen);

    for(iter = g_socket_pool.mempool_entry.used_list.next; iter != &g_socket_pool.mempool_entry.used_list; iter = iter->next)
    {
        tconnd_socket_t *s = TLIBC_CONTAINER_OF(iter, tconnd_socket_t, mempool_entry.used_list);
        tconnd_socket_delete(s);
    }
}


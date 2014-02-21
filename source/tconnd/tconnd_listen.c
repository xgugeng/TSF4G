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
#include "tlog/tlog_instance.h"

#include <sys/ioctl.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <assert.h>
#include <errno.h>

int g_listenfd;


TERROR_CODE tconnd_listen_init()
{
    int nb = 1;
    TERROR_CODE ret = E_TS_NOERROR;
	struct sockaddr_in  listenaddr;

	g_listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(g_listenfd == -1)
	{
	    ERROR_LOG("socket errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto done;
	}

	
	if(ioctl(g_listenfd, FIONBIO, &nb) == -1)
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
	
	if(ioctl(g_listenfd, FIONBIO, &nb) == -1)
	{
        ERROR_LOG("ioctl errno[%d], %s.", errno, strerror(errno));
        ret = E_TS_ERRNO;
		goto close_listenfd;
	}

    if (setsockopt(g_listenfd, IPPROTO_TCP, TCP_DEFER_ACCEPT, (int[]){1}, sizeof(int)))
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
	
	size_t tbus_writer_size;
	sip_req_t *pkg;

//1, 检查tbus是否能发送新的连接包
	tbus_writer_size = sizeof(sip_req_t);
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
	conn_socket = tconnd_socket_new();
	if(conn_socket == NULL)
	{
//到达连接数上线
//        DEBUG_LOG("tconnd_socket_new return NULL");
		ret = E_TS_WOULD_BLOCK;
		goto done;
	}

    ret = tconnd_socket_accept(conn_socket);
    if(ret == E_TS_WOULD_BLOCK)
    {
        goto free_socket;
    }
	else if(ret != E_TS_NOERROR)
	{
    	goto free_socket;
	}	

//5, 发送连接的通知	
	pkg->cmd = e_sip_req_cmd_connect;
    pkg->cid.sn = 0;
	pkg->cid.id = tlibc_mempool_ptr2id(g_socket_pool, conn_socket);
	pkg->size = 0;
	
	conn_socket->status = e_tconnd_socket_status_syn_sent;
	tbus_send_end(g_output_tbus, sizeof(pkg));

done:
	return ret;
free_socket:
    tconnd_socket_delete(conn_socket);
	return ret;
}


void tconnd_listen_fini()
{
    TLIBC_LIST_HEAD *iter, *next;
    

    for(iter = g_socket_pool->used_list.next; iter != &g_socket_pool->used_list; iter = next)
    {        
        tlibc_mempool_block_t *b = TLIBC_CONTAINER_OF(iter, tlibc_mempool_block_t, used_list);
        next = iter->next;
        tconnd_socket_t *s = (tconnd_socket_t *)&b->data;
        tconnd_socket_delete(s);
    }

    if(close(g_listenfd) != 0)
    {
        ERROR_LOG("close errno[%d], %s", errno, strerror(errno));
    }
}


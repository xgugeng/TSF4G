#include "tconnd_listen.h"
#include "tcommon/terrno.h"
#include "tconnd/tdtp_instance.h"
#include "tconnd/tdtp_socket.h"
#include "tbus/tbus.h"
#include "tlibc/protocol/tlibc_binary_writer.h"
#include <assert.h>
#include "tcommon/tdgi_types.h"
#include "tcommon/tdgi_writer.h"

#include <sys/ioctl.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "tconnd/tconnd_mempool.h"
#include "tconnd/tconnd_tbus.h"
#include "tconnd/tconnd_config.h"


int g_listenfd;


TERROR_CODE tconnd_listen_init()
{
    int nb = 1;
    TERROR_CODE ret = E_TS_NOERROR;
	struct sockaddr_in  listenaddr;

	g_listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(g_listenfd == -1)
	{
	    ret = E_TS_ERRNO;
		goto done;
	}

	
	if(ioctl(g_listenfd, FIONBIO, &nb) == -1)
	{
        ret = E_TS_ERRNO;
		goto close_listenfd;
	}

	
	memset(&listenaddr, 0, sizeof(struct sockaddr_in));

	listenaddr.sin_family=AF_INET;
	listenaddr.sin_addr.s_addr = inet_addr(g_config.ip);
	listenaddr.sin_port = htons(g_config.port);

	if(bind(g_listenfd,(struct sockaddr *)(&listenaddr), sizeof(struct sockaddr_in)) == -1)
	{
        ret = E_TS_ERRNO;
		goto close_listenfd;
	}	

	if(listen(g_listenfd, g_config.backlog) == -1)
	{
        ret = E_TS_ERRNO;
		goto close_listenfd;
	}
	
close_listenfd:
    close(g_listenfd);
done:
    return ret;
}

TERROR_CODE process_listen()
{
	int ret = E_TS_NOERROR;
	tdtp_socket_t *conn_socket;
	
	char *tbus_writer_ptr;
	size_t tbus_writer_size;	
	tdgi_req_t pkg;
	TLIBC_BINARY_WRITER writer;

//1, 检查tbus是否能发送新的连接包
	tbus_writer_size = TDGI_REQ_HEAD_SIZE;
	ret = tbus_send_begin(g_output_tbus, &tbus_writer_ptr, &tbus_writer_size);
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
	conn_socket = tdtp_socket_new();
	if(conn_socket == NULL)
	{
		ret = E_TS_WOULD_BLOCK;
		goto done;
	}

    ret = tdtp_socket_accept(conn_socket, g_listenfd);
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
    assert(writer.offset == TDGI_REQ_HEAD_SIZE);
	conn_socket->status = e_tdtp_socket_status_syn_sent;
	tbus_send_end(g_output_tbus, writer.offset);

done:
	return ret;
free_socket:
    tdtp_socket_delete(conn_socket);
	return ret;
}


void tconnd_listen_fini()
{
    int i;
      
    for(i = g_socket_pool->used_head; i < g_socket_pool->unit_num; )
    {
        tlibc_mempool_block_t *b = TLIBC_MEMPOOL_GET_BLOCK(g_socket_pool, i);
        tdtp_socket_t *s = (tdtp_socket_t *)&b->data;
        tdtp_socket_delete(s);

        i = b->next;
    }

    close(g_listenfd);
}


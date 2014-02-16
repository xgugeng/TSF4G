#include "tconnd_listen.h"
#include "tcommon/terrno.h"
#include "tconnd/tdtp_instance.h"
#include "tconnd/tdtp_socket.h"
#include "tbus/tbus.h"
#include "tlibc/protocol/tlibc_binary_writer.h"
#include "globals.h"
#include <assert.h>
#include "tcommon/tdgi_types.h"
#include "tcommon/tdgi_writer.h"

#include <sys/ioctl.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>




TERROR_CODE tconnd_listen_init(tdtp_instance_t *self)
{
    int nb = 1;
    TERROR_CODE ret = E_TS_NOERROR;

	self->listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(self->listenfd == -1)
	{
	    ret = E_TS_ERRNO;
		goto done;
	}

	
	if(ioctl(self->listenfd, FIONBIO, &nb) == -1)
	{
        ret = E_TS_ERRNO;
		goto close_listenfd;
	}

	
	memset(&self->listenaddr, 0, sizeof(struct sockaddr_in));

	self->listenaddr.sin_family=AF_INET;
	self->listenaddr.sin_addr.s_addr = inet_addr(g_config.ip);
	self->listenaddr.sin_port = htons(g_config.port);

	if(bind(self->listenfd,(struct sockaddr *)(&self->listenaddr), sizeof(struct sockaddr_in)) == -1)
	{
        ret = E_TS_ERRNO;
		goto close_listenfd;
	}	

	if(listen(self->listenfd, g_config.backlog) == -1)
	{
        ret = E_TS_ERRNO;
		goto close_listenfd;
	}
	
close_listenfd:
    close(self->listenfd);
done:
    return ret;
}

TERROR_CODE process_listen(tdtp_instance_t *self)
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


void tconnd_listen_fini(tdtp_instance_t *self)
{
    int i;
      
    for(i = self->socket_pool->used_head; i < self->socket_pool->unit_num; )
    {
        tlibc_mempool_block_t *b = TLIBC_MEMPOOL_GET_BLOCK(self->socket_pool, i);
        tdtp_socket_t *s = (tdtp_socket_t *)&b->data;
        tdtp_socket_free(s);

        i = b->next;
    }

    close(self->listenfd);
}


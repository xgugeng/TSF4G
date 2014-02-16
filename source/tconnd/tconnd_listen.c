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


#include "tlibc/platform/tlibc_platform.h"
#include "tconnd/instance/tdtp_instance.h"
#include "tcommon/tdgi_types.h"
#include "tcommon/tdgi_writer.h"


#include "tlibc/protocol/tlibc_compact_reader.h"
#include "tlibc/protocol/tlibc_compact_writer.h"


#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#include <errno.h>

#include <stdio.h>


TERROR_CODE tdtp_instance_init(tdtp_instance_t *self, const tconnd_tdtp_t *config)
{
    int  nb = 1;

	self->config = config;
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(struct sockaddr_in));

	sockaddr.sin_family=AF_INET;
	sockaddr.sin_addr.s_addr = inet_addr(config->ip);
	sockaddr.sin_port = htons(config->port);
	
	self->listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(self->listenfd == -1)
	{
		goto ERROR_RET;
	}
	
	if(bind(self->listenfd,(struct sockaddr *)(&sockaddr), sizeof(struct sockaddr_in)) == -1)
	{
		goto ERROR_RET;
	}
	
	if(ioctl(self->listenfd, FIONBIO, &nb) == -1)
	{
		goto ERROR_RET;
	}

	if(listen(self->listenfd, config->backlog) == -1)
	{
		goto ERROR_RET;
	}


	self->epollfd = epoll_create(config->epoll_size);
	if(self->epollfd == -1)
	{
		goto ERROR_RET;
	}

	
	self->input_tbusid = shmget(config->input_tbuskey, 0, 0666);
	if(self->input_tbusid == -1)
	{
		goto ERROR_RET;
	}
	self->input_tbus = shmat(self->input_tbusid, NULL, 0);
	if(self->input_tbus == NULL)
	{
		goto ERROR_RET;
	}

	self->output_tbusid = shmget(config->output_tbuskey, 0, 0666);
	if(self->output_tbusid == -1)
	{
		goto ERROR_RET;
	}
	self->output_tbus = shmat(self->output_tbusid, NULL, 0);
	if(self->output_tbus == NULL)
	{
		goto ERROR_RET;
	}
	
	return E_TS_NOERROR;
ERROR_RET:
	return E_TS_ERROR;
}

static TERROR_CODE process_listen(tdtp_instance_t *self)
{
	int ret = E_TS_AGAIN;
	struct sockaddr_in cnt_addr;
    socklen_t          cnt_len = sizeof(struct sockaddr_in);
	int conn_sock = 0;
	char *obuff;
	tuint16 obuff_len;
	
	tdgi_t pkg;
	tuint32 pkg_len;
	TLIBC_COMPACT_WRITER compact_writer;
	char pkg_buff[sizeof(tdgi_t)];

	if(self->events_num >= TDTP_MAX_EVENTS)
	{
		ret = E_TS_AGAIN;
		goto done;
	}

	tlibc_compact_writer_init(&compact_writer, pkg_buff, sizeof(pkg_buff));	
	pkg.cmd = e_tdgi_cmd_new_connection_req;
	pkg.body.new_connection.cid = 0;
	tlibc_write_tdgi_t(&compact_writer.super, &pkg);
	pkg_len = compact_writer.offset;
	obuff_len = pkg_len;

	ret = tbus_send_begin(self->output_tbus, &obuff, &obuff_len);
	if((ret == E_TS_WOULD_BLOCK) || (ret == E_TS_AGAIN))
	{
		ret = E_TS_AGAIN;
		goto done;
	}
	else
	{
		ret = E_TS_ERROR;
		goto done;
	}

	
	conn_sock = accept(self->listenfd, (struct sockaddr *)&cnt_addr, &cnt_len);
	if(conn_sock == -1)
	{
		switch(errno)
		{
			case EAGAIN:
#if EWOULDBLOCK != EAGAIN
			case EWOULDBLOCK:
#endif
				ret = E_TS_AGAIN;
				goto done;
			default:
				fprintf(stderr, "%s.%d: accept failed: %s, errno(%d)\n", __FILE__, __LINE__, strerror(errno), errno);
				ret = E_TS_NOERROR;
				goto done;
		}
	}
	else
	{
		int nb = 1;
		if(ioctl(conn_sock, FIONBIO, &nb) == -1)
		{
			close(conn_sock);
			ret = E_TS_NOERROR;
			goto done;
		}
		self->events[self->events_num].events = EPOLLIN | EPOLLET;
		self->events[self->events_num].data.fd = conn_sock;
		
        if(epoll_ctl(self->epollfd, EPOLL_CTL_ADD, conn_sock,
                                   &self->events[self->events_num]) == -1)
		{
			ret = E_TS_NOERROR;
			goto done;
        }
		else
		{
			++self->events_num;
		}
	}
	
done:
	return ret;
}

static TERROR_CODE process_epool(tdtp_instance_t *self)
{
	int i;
	
	if(self->events_num == 0)
	{
		self->events_num = epoll_wait(self->epollfd, self->events, TDTP_MAX_EVENTS, -1);
	    if(self->events_num == -1)
		{
			goto ERROR_RET;
	    }
	}

	for(i = 0; i < self->events_num; ++i)
	{
//		char *buff;
//		tuint16 buff_size = 1;
		
//		tbus_send_begin();
	}
	
	return E_TS_NOERROR;
ERROR_RET:
	return E_TS_ERROR;
}

TERROR_CODE tdtp_instance_process(tdtp_instance_t *self)
{
	TERROR_CODE ret = E_TS_NOERROR;
	TERROR_CODE r;

	r = process_listen(self);	
	if(r == E_TS_NOERROR)
	{
	}
	else if(r == E_TS_AGAIN)
	{
		ret = E_TS_AGAIN;		
	}
	else
	{
		ret = r;
		goto done;
	}
	
	r = process_epool(self);
	if(r == E_TS_NOERROR)
	{
	}
	else if(r == E_TS_AGAIN)
	{
		ret = E_TS_AGAIN;
	}
	else
	{
		ret = r;
		goto done;
	}

	
done:
	return ret;
}



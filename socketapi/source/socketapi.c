#include "socketapi.h"
#include "tlibc_error_code.h"

#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

tlibc_error_code_t socketapi_open(socketapi_t *self)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	int nb;
	int r;
	int socketfd = -1;

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketfd == -1)
    {
		goto errno_ret;
    }

    if(setsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, &self->sndbuf, sizeof(self->sndbuf)) == -1)
	{	
		goto close_socket;
	}

    if(setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, &self->rcvbuf, sizeof(self->rcvbuf)) == -1)
	{
		goto close_socket;
	}
	
    nb = 1;
	if(ioctl(socketfd, FIONBIO, &nb) == -1)
	{
		goto close_socket;
	}

	r = connect(socketfd, (struct sockaddr *)&self->address, sizeof(self->address));
	if((errno != EINPROGRESS) && (errno != EAGAIN))
	{
		goto close_socket;
	}

	self->socket_fd = socketfd;

	return ret;
close_socket:
	close(socketfd);
errno_ret:
	return E_TLIBC_ERRNO;
}

void socketapi_close(socketapi_t *self)
{
	if(self->socket_fd != -1)
	{
		self->socket_fd = -1;
		close(self->socket_fd);
	}
}

void socketapi_init(socketapi_t *self, const char *ip, uint16_t port, uint32_t sndbuf, uint32_t rcvbuf)
{
	self->socket_fd = -1;
	self->sndbuf = sndbuf;
	self->rcvbuf = rcvbuf;
	self->recvbuf_size = 0;

	memset(&self->address, 0, sizeof(self->address));
	self->address.sin_family = AF_INET;
	self->address.sin_addr.s_addr = inet_addr(ip);
	self->address.sin_port  = htons(port);
}

tlibc_error_code_t socketapi_process(socketapi_t *self)
{
	char *iter, *last, *limit;
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	ssize_t recv_size;
	if(self->socket_fd == -1)
	{
		ret = E_TLIBC_WOULD_BLOCK;
		goto done;
	}

	recv_size = recv(self->socket_fd, self->recvbuf + self->recvbuf_size, (size_t)(SOCKETAPI_RECVBUF_SIZE - self->recvbuf_size), 0);
	if(recv_size < 0)
	{
		if(errno == EAGAIN)
		{
			ret = E_TLIBC_WOULD_BLOCK;
			goto done;
		}
		else if(errno == EINTR)
		{
			goto done;
		}
		else
		{
			socketapi_close(self);
			goto done;
		}
	}

	self->recvbuf_size += (size_t)recv_size;



	last = NULL;
	limit = self->recvbuf + self->recvbuf_size;
	for(iter = self->recvbuf; iter <= limit;)
	{
		char *next;
		uint16_t packet_size;
		if(limit - iter < sizeof(uint16_t))
		{
			break;
		}

		packet_size = *(uint16_t*)iter;
#ifdef TSF4G_BIGENDIAN
		packet_size = be16toh(packet_size);
#endif//TSF4G_BIGENDIAN
		next = iter + sizeof(uint16_t) + packet_size;
		if(next <= limit)
		{
			last = next;
			if(self->on_recv)
			{
				self->on_recv(self, iter, (size_t)(last - iter));
			}
		}
		iter = next;
	}

	if((last) && (last < limit))
	{
		self->recvbuf_size = (size_t)(limit - last);
		memcpy(self->recvbuf, last, self->recvbuf_size);
	}

done:
	return ret;
}

tlibc_error_code_t socketapi_send(socketapi_t *self, char *packet, uint16_t packet_len)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	struct iovec iov[2];
	uint16_t package_size = packet_len;
	ssize_t send_size;

	if(self->socket_fd == -1)
	{
		if(!socketapi_open(self))
		{
			ret = E_TLIBC_ERRNO;
			goto done;
		}
	}
#ifdef TSF4G_BIGENDIAN
	package_size = htobe16(package_size);
#endif
	iov[0].iov_base = &package_size;
	iov[0].iov_len = sizeof(package_size);
	iov[1].iov_base = (char*)packet;
	iov[1].iov_len = packet_len;
	
	//可以加入缓存降低系统调用次数。
    send_size = writev(self->socket_fd, iov, 2);
	if(send_size != iov[0].iov_len + iov[1].iov_len)
	{
		ret = E_TLIBC_ERRNO;
		if((errno == EAGAIN) || (errno == EINTR))
		{
			goto done;
		}
		else
		{
			socketapi_close(self);
			goto done;
		}
	}

done:
	return ret;
}


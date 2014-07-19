#ifndef _H_SOCKETAPI_H
#define _H_SOCKETAPI_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "tlibc_error_code.h"

#include <stdint.h>
#include <stddef.h>
#include <netinet/in.h>
#include <stdbool.h>

#define SOCKETAPI_RECVBUF_SIZE 0xfffff

typedef struct socketapi_s socketapi_t;
typedef void (*socketapi_on_recv_func)(socketapi_t *self, const char *buf, size_t buf_len);
struct socketapi_s
{
	int socket_fd;
	uint32_t sndbuf;
	uint32_t rcvbuf;
	struct sockaddr_in address;

	char recvbuf[SOCKETAPI_RECVBUF_SIZE];
	size_t recvbuf_size;

	socketapi_on_recv_func on_recv;
};

void socketapi_init(socketapi_t *self, const char *ip, uint16_t port, uint32_t sndbuf, uint32_t rcvbuf);

tlibc_error_code_t socketapi_open(socketapi_t *self);

void socketapi_close(socketapi_t *self);

tlibc_error_code_t socketapi_send(socketapi_t *self, char *packet, uint16_t packet_len);

tlibc_error_code_t socketapi_process(socketapi_t *self);

#ifdef  __cplusplus
}
#endif 
#endif//_H_SOCKETAPI_H


#ifndef _H_SOCKETAPI_H
#define _H_SOCKETAPI_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "tlibc_error_code.h"

#include <stdint.h>
#include <stddef.h>

typedef struct socketapi_s socketapi_t;
typedef void (*socketapi_on_recv_func)(socketapi_t *self, const char *buf, size_t buf_len);
struct socketapi_s
{
	int socket_fd;

	socketapi_on_recv_func on_recv;
};

tlibc_error_code_t socketapi_init(socketapi_t *self, const char *ip, uint16_t port);

void socketapi_fini(socketapi_t *self);

void socketapi_send(socketapi_t *self, const char *packet, size_t packet_len);

tlibc_error_code_t socketapi_process(socketapi_t *self);

#ifdef  __cplusplus
}
#endif 
#endif//_H_SOCKETAPI_H


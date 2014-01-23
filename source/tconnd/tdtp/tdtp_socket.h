#ifndef _H_TDTP_SOCKET_H
#define _H_TDTP_SOCKET_H

#include "tlibc/platform/tlibc_platform.h"
#include <netinet/in.h>
#include "tlibc/core/tlibc_timer.h"

typedef enum _tdtp_socket_status_t
{
	e_tdtp_socket_status_accept = 0,
}tdtp_socket_status_t;

typedef struct _tdtp_socket_t
{
	tdtp_socket_status_t status;

	int socketfd;
	struct sockaddr_in socketaddr;

	tuint64 accept_timeout_timer;
}tdtp_socket_t;

#endif//_H_TDTP_SOCKET_H

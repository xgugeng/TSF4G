#ifndef _H_TDTP_SOCKET_H
#define _H_TDTP_SOCKET_H

#include "tlibc/platform/tlibc_platform.h"
#include <netinet/in.h>
#include "tlibc/core/tlibc_timer.h"
#include "tcommon/tdgi_types.h"


#include <sys/uio.h>

typedef enum _tdtp_socket_status_t
{
	e_tdtp_socket_status_accept = 0,
	e_tdtp_socket_status_established = 0,
}tdtp_socket_status_t;

typedef struct _tdtp_socket_op
{
    const tdgi_t *head;
    struct iovec iov;
}tdtp_socket_op;

//合并处理的消息个数
#define TDTP_SOCKET_OP_LIST_NUM 32
typedef struct _tdtp_socket_t
{
	tdtp_socket_status_t status;

	int socketfd;

	tuint64 accept_timeout_timer;


    tdtp_socket_op op_list[TDTP_SOCKET_OP_LIST_NUM];
    tuint32 op_list_num;
}tdtp_socket_t;



void tdtp_socket_process_pkg(tdtp_socket_t *self);

void tdtp_socket_push_pkg(tdtp_socket_t *self, const tdgi_t *head, const char* content);

#endif//_H_TDTP_SOCKET_H

#ifndef _H_TDTP_SOCKET_H
#define _H_TDTP_SOCKET_H

#include "tlibc/platform/tlibc_platform.h"
#include <netinet/in.h>
#include "tlibc/core/tlibc_timer.h"
#include "tcommon/tdgi_types.h"
#include "tcommon/terrno.h"

#include <sys/uio.h>
#include <limits.h>

#define TDTP_TIMER_ACCEPT_TIME_MS 5000

#define MAX_PACKAGE_NUM 1024

typedef struct _package_buff_t
{
    tuint64 mid;
    char buff[TLIBC_UINT16_MAX];
    tuint16 buff_size;
}package_buff_t;


typedef enum _tdtp_socket_status_t
{
	e_tdtp_socket_status_closed = 0,
	e_tdtp_socket_status_syn_sent = 1,
	e_tdtp_socket_status_established = 2,
	e_tdtp_socket_status_closing = 3,
}tdtp_socket_status_t;

typedef struct _tdtp_socket_op_list
{
    tuint32 num;
    const tdgi_t *head[IOV_MAX];
    struct iovec iov[IOV_MAX];
}tdtp_socket_op_list;

typedef struct _tdtp_socket_t
{
    tuint64 mid;
	tdtp_socket_status_t status;
	int socketfd;
	struct sockaddr_in socketaddr;

	tlibc_timer_entry_t accept_timeout;
	tlibc_timer_entry_t close_timeout;
    tlibc_timer_entry_t package_timeout;

    package_buff_t *package_buff;
    TLIBC_LIST_HEAD readable_list;
    TLIBC_LIST_HEAD writeable_list;


    tdtp_socket_op_list op_list;
}tdtp_socket_t;

tdtp_socket_t *tdtp_socket_alloc();

void tdtp_socket_free(tdtp_socket_t *self);

TERROR_CODE tdtp_socket_accept(tdtp_socket_t *self, int listenfd);

TERROR_CODE tdtp_socket_process(tdtp_socket_t *self);

TERROR_CODE tdtp_socket_push_pkg(tdtp_socket_t *self, const tdgi_t *head, const char* body, size_t body_size);

TERROR_CODE tdtp_socket_recv(tdtp_socket_t *self);



#endif//_H_TDTP_SOCKET_H

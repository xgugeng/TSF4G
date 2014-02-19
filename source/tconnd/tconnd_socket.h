#ifndef _H_TCONND_SOCKET_H
#define _H_TCONND_SOCKET_H

#include "tlibc/platform/tlibc_platform.h"
#include <netinet/in.h>
#include "tlibc/core/tlibc_timer.h"
#include "tcommon/tdgi_types.h"
#include "tcommon/terrno.h"
#include "tcommon/tdtp.h"

#include <sys/uio.h>
#include <limits.h>

#define TDTP_TIMER_ACCEPT_TIME_MS 5000
#define TDTP_TIMER_PACKAGE_TIME_MS 5000


#define MAX_PACKAGE_NUM 1024

typedef struct _package_buff_t
{
    char buff[TDTP_SIZE_T_MAX];
    tdtp_size_t buff_size;
}package_buff_t;


typedef enum _tconnd_socket_status_t
{
	e_tconnd_socket_status_closed = 1,
	e_tconnd_socket_status_syn_sent = 2,
	e_tconnd_socket_status_established = 3,
}tconnd_socket_status_t;

#ifndef IOV_MAX
    #error "IOV_MAX not define."
#endif

#define TCONND_SOCKET_OP_LIST_MAX 16
#if TCONND_SOCKET_OP_LIST_MAX > IOV_MAX
        #error "TCONND_SOCKET_OP_LIST_MAX > IOV_MAX"
#endif


typedef struct _tconnd_socket_op_list
{
    tuint32 num;
    const tdgi_rsp_t *head[TCONND_SOCKET_OP_LIST_MAX];
    struct iovec iov[TCONND_SOCKET_OP_LIST_MAX];
}tconnd_socket_op_list;

typedef struct _tconnd_socket_t
{
    tuint64 mid;
	tconnd_socket_status_t status;
	int socketfd;

	tlibc_timer_entry_t accept_timeout;
    tlibc_timer_entry_t package_timeout;
	tlibc_timer_entry_t close_timeout;

    package_buff_t *package_buff;
    TLIBC_LIST_HEAD readable_list;
    TLIBC_LIST_HEAD writable_list;
    int writable;
    int readable;


    tconnd_socket_op_list op_list;
}tconnd_socket_t;

tconnd_socket_t *tconnd_socket_new();

void tconnd_socket_delete(tconnd_socket_t *self);

TERROR_CODE tconnd_socket_accept(tconnd_socket_t *self);

TERROR_CODE tconnd_socket_process(tconnd_socket_t *self);

TERROR_CODE tconnd_socket_push_pkg(tconnd_socket_t *self, const tdgi_rsp_t *head, const char* body, size_t body_size);

TERROR_CODE tconnd_socket_recv(tconnd_socket_t *self);



#endif//_H_TCONND_SOCKET_H

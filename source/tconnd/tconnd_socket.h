#ifndef _H_TCONND_SOCKET_H
#define _H_TCONND_SOCKET_H

#include "tlibc/platform/tlibc_platform.h"
#include <netinet/in.h>
#include "tlibc/core/tlibc_timer.h"
#include "tcommon/terrno.h"
#include "tcommon/sip.h"
#include <sys/uio.h>
#include <limits.h>
#include "tcommon/bscp.h"

#include "tlibc/core/tlibc_mempool.h"


#pragma pack(push,1)
typedef struct _package_buff_t
{
    tlibc_mempool_entry_t mempool_entry;
    
    size_t size;
    char head[BSCP_HEAD_T_SIZE];    
    char body[1];
}package_buff_t;
#pragma pack(pop)


typedef enum _tconnd_socket_status_t
{
	e_tconnd_socket_status_closed = 1,
	e_tconnd_socket_status_syn_sent = 2,
	e_tconnd_socket_status_established = 3,
}tconnd_socket_status_t;

#ifdef IOV_MAX
    #define TCONND_SOCKET_OP_LIST_MAX 16
    #if TCONND_SOCKET_OP_LIST_MAX > IOV_MAX
        #error "TCONND_SOCKET_OP_LIST_MAX > IOV_MAX"
    #endif
#endif

typedef struct _tconnd_socket_t
{
    tlibc_mempool_entry_t mempool_entry;

    uint32_t id;

	tconnd_socket_status_t status;
	int socketfd;

	tlibc_timer_entry_t accept_timeout;
	tlibc_timer_entry_t close_timeout;

    TLIBC_LIST_HEAD package_socket_list;
    tlibc_timer_entry_t package_timeout;
    package_buff_t *package_buff;
    
    TLIBC_LIST_HEAD readable_list;
    TLIBC_LIST_HEAD writable_list;
    int writable;
    int readable;


    size_t iov_total_size;
    struct iovec iov[TCONND_SOCKET_OP_LIST_MAX];
    size_t iov_num;
}tconnd_socket_t;

tconnd_socket_t *tconnd_socket_new();

void tconnd_socket_delete(tconnd_socket_t *self);

TERROR_CODE tconnd_socket_flush(tconnd_socket_t *self);

TERROR_CODE tconnd_socket_push_pkg(tconnd_socket_t *self, const sip_rsp_t *head, const char* body, size_t body_size);

TERROR_CODE tconnd_socket_recv(tconnd_socket_t *self);



#endif//_H_TCONND_SOCKET_H

#ifndef _H_TCONND_SOCKET_H
#define _H_TCONND_SOCKET_H

#include <netinet/in.h>
#include "terrno.h"
#include "sip.h"
#include <sys/uio.h>
#include <limits.h>
#include "core/tlibc_list.h"
#include "core/tlibc_mempool.h"
#include "bscp_types.h"

#include <stdbool.h>
#include <stdint.h>



#pragma pack(push,1)
typedef struct _package_buff_t
{
    tlibc_mempool_entry_t mempool_entry;
    
    size_t size;
    char head[sizeof(bscp_head_t)];    
    char body[1];
}package_buff_t;
#pragma pack(pop)


typedef enum _tconnd_socket_status_t
{
	e_tconnd_socket_status_closed = 1,
	e_tconnd_socket_status_syn_sent = 2,
	e_tconnd_socket_status_established = 3,
	e_tconnd_socket_status_listen = 4,
}tconnd_socket_status_t;

#define TCONND_SOCKET_OP_LIST_MAX 128
typedef struct _tconnd_socket_t
{
    tlibc_mempool_entry_t mempool_entry;

    uint32_t id;

	tconnd_socket_status_t status;
	int socketfd;


    uint64_t        pending_ticks;
	tlibc_list_head_t g_pending_socket_list;

    uint64_t        package_ticks;
    tlibc_list_head_t g_package_socket_list;
    package_buff_t *package_buff;
    
    tlibc_list_head_t readable_list;
    tlibc_list_head_t writable_list;
    bool writable;
    bool readable;


    size_t iov_total_size;
    struct iovec iov[TCONND_SOCKET_OP_LIST_MAX];
    int iov_num;
}tconnd_socket_t;

void tcond_socket_construct(tconnd_socket_t* self);

void tcond_socket_destruct(tconnd_socket_t* self);


tconnd_socket_t *tconnd_socket_new();

void tconnd_socket_delete(tconnd_socket_t *self);

TERROR_CODE tconnd_socket_flush(tconnd_socket_t *self);

TERROR_CODE tconnd_socket_push_pkg(tconnd_socket_t *self, const sip_rsp_t *head, void* body, size_t body_size);

TERROR_CODE tconnd_socket_recv(tconnd_socket_t *self);



#endif//_H_TCONND_SOCKET_H

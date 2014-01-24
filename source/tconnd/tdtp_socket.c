#include "tconnd/tdtp_socket.h"
#include <assert.h>
#include "tconnd/timer/tdtp_timer.h"
#include "globals.h"

void tdtp_socket_process(tdtp_socket_t *self)
{
	tuint32 i;
    for(i = 0; i < self->op_list_num; ++i)
    {
        const tdgi_t *pkg = &self->op_list[i]->head;
        switch(pkg->cmd)
        {
        case e_tdgi_cmd_new_connection_ack:
            {            
                tdtp_timer_t *timeout_timer = NULL;
                timeout_timer = tlibc_mempool_get(g_tdtp_instance.timer_pool
                    , self->accept_timeout_timer);            
                assert(timeout_timer != NULL);
                tlibc_timer_pop(&timeout_timer->entry);
                tlibc_mempool_free(g_tdtp_instance.timer_pool
                    , self->accept_timeout_timer);
                
                self->status = e_tdtp_socket_status_established;                     
                break;
            }
        case e_tdgi_cmd_send:
            {
                break;
            }
        default:
            goto AGAIN;
        }
    }
AGAIN:
    self->op_list_num = 0;

	return;
}

void tdtp_socket_push_pkg(tdtp_socket_t *self, const tdtp_socket_op *head)
{
    if(self->op_list_num >= TDTP_SOCKET_OP_LIST_NUM)
    {
        tdtp_socket_process(self);
    }
    assert(self->op_list_num < TDTP_SOCKET_OP_LIST_NUM);
    self->op_list[self->op_list_num] = head;
    ++self->op_list_num;
}


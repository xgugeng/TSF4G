#include "tconnd_tbus.h"
#include "tconnd/tdtp_instance.h"

#include "tcommon/tdgi_types.h"
#include "tcommon/tdgi_reader.h"
#include "tlibc/protocol/tlibc_binary_reader.h"
#include "tcommon/tdtp.h"
#include "tconnd/tdtp_socket.h"
#include "globals.h"

#include <sys/ipc.h>
#include <sys/shm.h>


TERROR_CODE tconnd_tbus_init(tdtp_instance_t *self)
{
    TERROR_CODE ret = E_TS_NOERROR;

	self->input_tbusid = shmget(g_config.input_tbuskey, 0, 0666);
	if(self->input_tbusid == -1)
	{
	    ret = E_TS_ERRNO;
		goto done;
	}
	self->input_tbus = shmat(self->input_tbusid, NULL, 0);
	if(self->input_tbus == NULL)
	{
	    ret = E_TS_ERRNO;
		goto done;
	}

	self->output_tbusid = shmget(g_config.output_tbuskey, 0, 0666);
	if(self->output_tbusid == -1)
	{
	    ret = E_TS_ERRNO;
		goto shmdt_input;
	}
	self->output_tbus = shmat(self->output_tbusid, NULL, 0);
	if(self->output_tbus == NULL)
	{
	    ret = E_TS_ERRNO;
		goto shmdt_input;
	}
	goto done;

	
shmdt_input:
    shmdt(self->input_tbus);
done:
    return ret;
}

//tbus中最多一次处理的包的个数
#define MAX_PACKAGE_LIST_NUM 255
TERROR_CODE process_input_tbus(tdtp_instance_t *self)
{
    static tdgi_rsp_t pkg_list[MAX_PACKAGE_LIST_NUM];
    tuint32 pkg_list_num = 0;
	TERROR_CODE ret = E_TS_NOERROR;
	const char*message;
	size_t message_len;
	TLIBC_BINARY_READER reader;
	size_t len;
	TLIBC_ERROR_CODE r;
	tuint32 i;
    TLIBC_LIST_HEAD writable_list;
    TLIBC_LIST_HEAD *iter, *next;


    ret = tbus_read_begin(self->input_tbus, &message, &message_len);
    if(ret == E_TS_WOULD_BLOCK)
    {
        goto done;
    }
    else if(ret != E_TS_NOERROR)
    {
        goto done;
    }

    len = message_len;
    tlibc_list_init(&writable_list);
    while(len > 0)
    {
        tdgi_rsp_t *pkg = &pkg_list[pkg_list_num];
        size_t pkg_size;
        const char* body_addr;
        size_t body_size;
        
        tlibc_binary_reader_init(&reader, message, len);
        r = tlibc_read_tdgi_rsp_t(&reader.super, pkg);
        if(r != E_TLIBC_NOERROR)
        {
            ret = E_TS_ERROR;
            goto done;
        }
        pkg_size = reader.offset;
        if(pkg->size > TDTP_SIZE_T_MAX)
        {
            ret = E_TS_ERROR;
            goto done;
        }

        for(i = 0; i < pkg->mid_num; ++i)
        {
            tdtp_socket_t *socket = (tdtp_socket_t*)tlibc_mempool_get(self->socket_pool, pkg->mid[i]);
            
            if(pkg->cmd == e_tdgi_cmd_send)
            {
                body_addr = message + reader.offset + pkg->size;
                body_size = pkg->size;
            }
            else
            {
                body_addr = NULL;
                body_size = 0;
            }
            
            if(socket != NULL)
            {
                ++pkg_list_num;
                ret = tdtp_socket_push_pkg(socket, pkg, body_addr, body_size);
                if(ret != E_TS_NOERROR)
                {
                    goto done;
                }

                if(!socket->writable)
                {
                    socket->writable = TRUE;
                    tlibc_list_add_tail(&socket->writable_list, &writable_list);
                }
                
                if(pkg_list_num >= MAX_PACKAGE_LIST_NUM)
                {
                    for(iter = writable_list.next; iter != &writable_list; iter = next)
                    {
                        tdtp_socket_t *s = TLIBC_CONTAINER_OF(iter, tdtp_socket_t, writable_list);
                        next = iter->next;
                        
                        ret = tdtp_socket_process(s);
                        s->writable = FALSE;
                        if(ret != E_TS_NOERROR)
                        {
                            goto done;
                        }
                        tlibc_list_del(iter);
                        tlibc_list_init(&s->writable_list);
                    }
                    pkg_list_num = 0;
                }
            }
        }
        
        message += pkg_size + body_size;
        len -= pkg_size + body_size;
    }
    
    for(iter = writable_list.next; iter != &writable_list; iter = iter->next)
    {        
        tdtp_socket_t *s = TLIBC_CONTAINER_OF(iter, tdtp_socket_t, writable_list);
        ret = tdtp_socket_process(s);
        s->writable = FALSE;
        if(ret != E_TS_NOERROR)
        {
            goto done;
        }
    }
    pkg_list_num = 0;

    tbus_read_end(self->input_tbus, message_len);
    
done:
    return ret;
}

void tconnd_tbus_fini(tdtp_instance_t *self)
{
    shmdt(self->input_tbus);
    shmdt(self->output_tbus);
}


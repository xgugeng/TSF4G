#include "tconnd_tbus.h"

#include "tcommon/tdgi_types.h"
#include "tcommon/tdgi_reader.h"
#include "tlibc/protocol/tlibc_binary_reader.h"
#include "tcommon/tdtp.h"

#include "tconnd/tconnd_socket.h"
#include "tconnd/tconnd_mempool.h"
#include "tconnd/tconnd_config.h"
#include "tlog/tlog_instance.h"
#include "tbus/tbus.h"

#include <sys/ipc.h>
#include <sys/shm.h>

#include <string.h>
#include <errno.h>

tbus_t              *g_input_tbus;
tbus_t              *g_output_tbus;

TERROR_CODE tconnd_tbus_init()
{
    TERROR_CODE ret = E_TS_NOERROR;
    int input_tbusid;
    int output_tbusid;

	input_tbusid = shmget(g_config.input_tbuskey, 0, 0666);
	if(input_tbusid == -1)
	{
	    ERROR_LOG("shmget errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto done;
	}
	g_input_tbus = shmat(input_tbusid, NULL, 0);
	if(g_input_tbus == NULL)
	{
        ERROR_LOG("shmat errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto done;
	}

	output_tbusid = shmget(g_config.output_tbuskey, 0, 0666);
	if(output_tbusid == -1)
	{
        ERROR_LOG("shmget errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto shmdt_input;
	}
	g_output_tbus = shmat(output_tbusid, NULL, 0);
	if(g_output_tbus == NULL)
	{
        ERROR_LOG("shmat errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto shmdt_input;
	}
	

	goto done;	
shmdt_input:
    shmdt(g_input_tbus);
done:
    return ret;
}

#define MAX_PACKAGE_LIST_NUM 255
TERROR_CODE process_input_tbus()
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


    ret = tbus_read_begin(g_input_tbus, &message, &message_len);
    if(ret == E_TS_WOULD_BLOCK)
    {
        goto done;
    }
    else if(ret != E_TS_NOERROR)
    {
        ERROR_LOG("tbus_read_begin return %d", ret);
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
            ERROR_LOG("tlibc_read_tdgi_rsp_t failed.");
            goto done;
        }
        pkg_size = reader.offset;
        if(pkg->size > TDTP_SIZE_T_MAX)
        {
            ERROR_LOG("tdgi_rsp.size >= TDTP_SIZE_T_MAX[%u].", TDTP_SIZE_T_MAX);
            ret = E_TS_ERROR;
            goto done;
        }

        for(i = 0; i < pkg->mid_num; ++i)
        {
            tconnd_socket_t *socket = (tconnd_socket_t*)tconnd_mempool_get(e_tconnd_socket, pkg->mid[i]);

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
                ret = tconnd_socket_push_pkg(socket, pkg, body_addr, body_size);
                if(ret != E_TS_NOERROR)
                {
                    goto done;
                }

                if(!socket->writable)
                {
                    socket->writable = TRUE;
                    tlibc_list_add_tail(&socket->writable_list, &writable_list);
                    DEBUG_LOG("socket [%llu] marked as writeable.", socket->mid);
                }
                
                if(pkg_list_num >= MAX_PACKAGE_LIST_NUM)
                {
                    for(iter = writable_list.next; iter != &writable_list; iter = next)
                    {
                        tconnd_socket_t *s = TLIBC_CONTAINER_OF(iter, tconnd_socket_t, writable_list);
                        next = iter->next;
                        
                        ret = tconnd_socket_process(s);
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
        tconnd_socket_t *s = TLIBC_CONTAINER_OF(iter, tconnd_socket_t, writable_list);
        ret = tconnd_socket_process(s);
        s->writable = FALSE;
        DEBUG_LOG("socket [%llu] marked as unwriteable.", s->mid);
        if(ret != E_TS_NOERROR)
        {
            goto done;
        }
    }
    pkg_list_num = 0;

    tbus_read_end(g_input_tbus, message_len);
    
done:
    return ret;
}

void tconnd_tbus_fini()
{
    if(shmdt(g_input_tbus) != 0)
    {
        ERROR_LOG("shmdt errno [%d], %s", errno, strerror(errno));
    }
    
    
    if(shmdt(g_output_tbus) != 0)
    {
        ERROR_LOG("shmdt errno [%d], %s", errno, strerror(errno));
    }    
}


#include "tconnd/tconnd_epoll.h"
#include "tlibc/core/tlibc_list.h"
#include "tcommon/terrno.h"
#include "tconnd/tconnd_reactor.h"
#include "tconnd/tconnd_socket.h"
#include "tconnd/tconnd_config.h"
#include "tlog/tlog_instance.h"

#include <unistd.h>
#include <assert.h>
#include <sys/epoll.h>

#include <errno.h>
#include <string.h>

int                 g_epollfd;
static TLIBC_LIST_HEAD     readable_list;

TERROR_CODE tconnd_epoll_init()
{
    TERROR_CODE ret = E_TS_NOERROR;

	tlibc_list_init(&readable_list);

	g_epollfd = epoll_create(g_config.connections);
	if(g_epollfd == -1)
	{
        ERROR_LOG("epoll_create errno[%d], %s.", errno, strerror(errno));
	    ret = E_TS_ERRNO;
		goto done;
	}


done:
    return ret;
}

#define TCONND_EPOLL_MAX_EVENTS 1024
TERROR_CODE tconnd_epool_proc()
{
	int i;
	TERROR_CODE ret = E_TS_NOERROR;
	TLIBC_LIST_HEAD *iter, *next;

	if(tlibc_list_empty(&readable_list))
	{
		struct epoll_event 	events[TCONND_EPOLL_MAX_EVENTS];
		int                 events_num;
		
		events_num = epoll_wait(g_epollfd, events, TCONND_EPOLL_MAX_EVENTS, 0);
	    if(events_num == -1)
		{
		    if(errno == EINTR)
		    {
		        ret = E_TS_WOULD_BLOCK;
		        DEBUG_LOG("tconnd_epool_proc reutrn E_TS_WOULD_BLOCK");
		    }
		    else
		    {
                ERROR_LOG("epoll_wait errno[%d], %s.", errno, strerror(errno));
		        ret = E_TS_ERRNO;
		    }
			goto done;
	    }

	    for(i = 0; i < events_num; ++i)
	    {
            tconnd_socket_t *socket = events[i].data.ptr;
            if(socket->readable)
            {
                ERROR_LOG("socket [%llu] already readable.", socket->sn);
                assert(0);
                ret = E_TS_ERROR;
                goto done;
            }
            socket->readable = TRUE;
            tlibc_list_init(&socket->readable_list);
            tlibc_list_add_tail(&socket->readable_list, &readable_list);
	    }
	}

	if(tlibc_list_empty(&readable_list))
	{
        ret = E_TS_WOULD_BLOCK;
	    goto done;
	}
	
    for(iter = readable_list.next; iter != &readable_list; iter = next)
    {
        TERROR_CODE r;
        tconnd_socket_t *socket = TLIBC_CONTAINER_OF(iter, tconnd_socket_t, readable_list);
        next = iter->next;
        
        r = tconnd_socket_recv(socket);
        if(r == E_TS_ERRNO)
        {
            switch(errno)
            {
                case EAGAIN:
                    socket->readable = FALSE;
                    tlibc_list_del(iter);
                    break;
                case EINTR:
                    break;
                default:
                    assert(0);
                    ret = E_TS_ERROR;
                    goto done;
            }
        }
        else if(r == E_TS_WOULD_BLOCK)
        {
            //tbusÂúÁË
            ret = E_TS_WOULD_BLOCK;
            break;
        }        
        else if(r != E_TS_NOERROR)
        {
            socket->readable = FALSE;
            tlibc_list_del(iter);
            tconnd_socket_delete(socket);
        }
    }

done:	
	return ret;
}

void tconnd_epoll_fini()
{
    if(close(g_epollfd) != 0)
    {
        ERROR_LOG("close errno[%d], %s", errno, strerror(errno));
    }
}


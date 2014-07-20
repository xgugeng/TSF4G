#include "tconnd_timer.h"
#include "tlog_log.h"
#include "tconnd.h"
#include "tconnd_epoll.h"
#include "tconnd_socket.h"
#include "tlibcdef.h"


#include <sys/time.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>

uint64_t       g_cur_ticks;
int			   g_timer_fd;
tconnd_epoll_data_type_t g_timer_etype;

void tconnd_timer_on_tick()
{
	for(;;)
	{
		uint64_t c;
		ssize_t rc;

		rc = read(g_timer_fd, &c, sizeof(uint64_t));
		if(rc < 0)
		{
			if(errno != EAGAIN)
			{
				ERROR_LOG("read return errno [%d], %s.", errno, strerror(errno));
			}
			goto done;
		}
		g_cur_ticks+=c;
	}

done:
	return;
}

tlibc_error_code_t tconnd_timer_init()
{
    tlibc_error_code_t ret = E_TLIBC_NOERROR;
	struct itimerspec its;
    struct epoll_event  ev;
	int r;
	int arg = 0;

	g_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
	if(g_timer_fd == -1)
	{
		ERROR_LOG("timerfd_create return errno [%d], %s.", errno, strerror(errno));
		ret = E_TLIBC_ERROR;
		goto done;
	}

	arg = fcntl(g_timer_fd, F_GETFL);
	if (arg < 0)
	{
		ERROR_LOG("fcntl return errno [%d], %s.", errno, strerror(errno));
		ret = E_TLIBC_ERROR;
		goto done;
	}

	if (fcntl(g_timer_fd, F_SETFL, arg | O_NONBLOCK) < 0)
	{
		ERROR_LOG("fcntl return errno [%d], %s.", errno, strerror(errno));
		ret = E_TLIBC_ERROR;
		goto done;
	}

	its.it_value.tv_sec  = g_config.tick_size;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = g_config.tick_size;
	its.it_interval.tv_nsec = 0;
	r = timerfd_settime(g_timer_fd, 0, &its, NULL);

	memset(&ev, 0, sizeof(ev));
	ev.events = (uint32_t)(EPOLLIN | EPOLLET);
	g_timer_etype = e_ted_timer;
	ev.data.ptr = &g_timer_etype;  
	if(epoll_ctl(g_epollfd, EPOLL_CTL_ADD, g_timer_fd, &ev) == -1)
	{
		DEBUG_LOG("epoll_ctl errno [%d], %s", errno, strerror(errno));
		ret = E_TLIBC_ERROR;
		goto done;
	}

	g_cur_ticks = 0;
done:
	return ret;
}

void tconnd_timer_fini()
{
	close(g_timer_fd);
}

void tconnd_timer_process()
{
	tlibc_list_head_t *iter, *next;
	tconnd_socket_t *s = NULL;

	for(iter = g_pending_socket_list.next; iter != &g_pending_socket_list; iter = next)
	{
		s = TLIBC_CONTAINER_OF(iter, tconnd_socket_t, g_pending_socket_list);        
		if(s->pending_ticks > g_cur_ticks)
		{
			break;
		}
		next = iter->next;
		tconnd_socket_delete(s);
	}


	for(iter = g_package_socket_list.next; iter != &g_package_socket_list; iter = next)
	{       
		s = TLIBC_CONTAINER_OF(iter, tconnd_socket_t, g_package_socket_list);
		if(s->package_ticks > g_cur_ticks)
		{
			break;
		}
		next = iter->next;
		tconnd_socket_delete(s);
	}
}


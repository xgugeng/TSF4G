#include "tbus/tbus.h"

#include <sys/ipc.h>
#include <sys/shm.h>


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sched.h>
#include "tlibc/core/tlibc_timer.h"
#include "tlibc/core/tlibc_error_code.h"
#include <sys/time.h>





#include "tcommon/tdtp.h"

#define PORT 7001
#define BUFF_SIZE 1024
int g_epollfd;
tlibc_timer_t g_timer;


typedef struct _robot_s
{
    tlibc_timer_entry_t entry;

    int socketfd;
    struct sockaddr_in address;
}robot_s;

int robot_init(robot_s *self);

int robot_send_buff(robot_s *self)
{
    char buff[BUFF_SIZE];
    tdtp_size_t *pkg_ptr = (tdtp_size_t *)buff;
    char *data_ptr = buff + TDTP_SIZEOF_SIZE_T;
    int len;
    int total_size, send_size;
    int idle_times;

    snprintf(data_ptr, BUFF_SIZE - TDTP_SIZEOF_SIZE_T, "hello %ld!\n", time(0));
    len = 1024;
    *pkg_ptr = len;
    TDTP_SIZE2LITTLE(*pkg_ptr);
    total_size = len + 2;
    send_size = 0;
    while(send_size < total_size)
    {
        int r = send(self->socketfd, buff + send_size, total_size - send_size, 0);
        if(r < 0)
        {
            switch(errno)
            {
            case EINTR:
            case EAGAIN:
                ++idle_times;
                if(idle_times > 50)
                {
                    usleep(1000);
                }
                continue;
            default:
                printf("send error\n");
                exit(1);
            }
        }
        else
        {
            idle_times = 0;
            send_size += r;
        }
    }
    return TRUE;
}

#define THINKING_INTERVAL 500

void robot_timeout(const tlibc_timer_entry_t *super)
{
    robot_s *self = TLIBC_CONTAINER_OF(super, robot_s, entry);

    robot_send_buff(self);

    TIMER_ENTRY_BUILD(&self->entry, 
	    g_timer.jiffies + THINKING_INTERVAL, robot_timeout);
    tlibc_timer_push(&g_timer, &self->entry);
}


int robot_init(robot_s *self)
{
    int nb = 1;
    int r;
	struct epoll_event 	ev;
	

    self->socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(self->socketfd == -1)
    {
        goto ERROR_RET;
    }
    
    memset(&self->address, 0, sizeof(self->address));
	self->address.sin_family 	    = AF_INET;
	self->address.sin_port 	    = htons(PORT);
	self->address.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    r = connect(self->socketfd, (struct sockaddr *)&self->address, sizeof(self->address));
    if(r != 0)
    {
        return FALSE;
    }

  	if(ioctl(self->socketfd, FIONBIO, &nb) == -1)
	{	    
		goto ERROR_RET;
	}	
	ev.events = EPOLLIN;
	ev.data.ptr = self;	
	

    if(epoll_ctl(g_epollfd, EPOLL_CTL_ADD, self->socketfd, &ev) == -1)
	{
	    goto ERROR_RET;
	}

	
	TIMER_ENTRY_BUILD(&self->entry, 
	    g_timer.jiffies + 0, robot_timeout);

	tlibc_timer_push(&g_timer, &self->entry);

    robot_send_buff(self);

    return TRUE;
ERROR_RET:
    return FALSE;    
}


void robot_process_recv(robot_s *self)
{
    char buff[BUFF_SIZE];
    int r;
    for(;;)
    {
        r = recv(self->socketfd, buff, BUFF_SIZE, 0);
        if(r < 0)
        {
            if (errno == EAGAIN)
            {
                break;
            }
            printf("recv error!\n");
            exit(1);
        }
    }
}

static tuint64 _get_current_ms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec*1000 + tv.tv_usec/1000;
}


#define ROBOT_NUM 1
#define ROBOT_MAX_EVENTS 1
int main()
{
    tuint64 start_ms;
    robot_s robot[ROBOT_NUM];
    int i;
    struct epoll_event  events[ROBOT_MAX_EVENTS];
    int                 events_num;
    size_t idle_times = 0;
    int busy = FALSE;

    start_ms = _get_current_ms();
    tlibc_timer_init(&g_timer, 0);

    g_epollfd = epoll_create(ROBOT_NUM);
    if(g_epollfd == -1)
    {
        goto ERROR_RET;
    }
    for(i = 0;i < ROBOT_NUM; ++i)
    {
        robot_init(&robot[i]);
    }


    
    for(;;)
    {
        busy = FALSE;
        
        events_num = epoll_wait(g_epollfd, events, ROBOT_MAX_EVENTS, 0);
        if(events_num == -1)
        {
            if(errno == EINTR)
            {
                ++idle_times;
            }
            else
            {
                goto ERROR_RET;
            }
        }
        else if(events_num > 0)
        {
            busy = TRUE;
            for(i = 0;i < events_num; ++i)
            {
                robot_process_recv(events[i].data.ptr);
            }
        }

        if(tlibc_timer_tick(&g_timer, _get_current_ms() - start_ms) == E_TLIBC_NOERROR)
        {
            busy = TRUE;
        }
        
        if(busy)
        {
            idle_times = 0;
        }
        else
        {
            ++idle_times;
        }
        
        if(idle_times > 30)
        {
            usleep(1000);
            idle_times = 0;
        }
        else
        {
            sched_yield();
        }

    }
	
	return 0;
ERROR_RET:
    return 1;
}


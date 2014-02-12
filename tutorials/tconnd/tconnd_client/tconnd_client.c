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





#include "tcommon/tdtp.h"

#define PORT 7001
#define BUFF_SIZE 1024
int g_epollfd;


typedef enum _robot_state_e
{
    e_close,
    e_syn_send,
    e_establish,
}robot_state_e;

typedef struct _robot_s
{
    robot_state_e state;
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

    len = snprintf(data_ptr, BUFF_SIZE - TDTP_SIZEOF_SIZE_T, "hello %ld!\n", time(0));
    ++len;
    *pkg_ptr = len;
    TDTP_SIZE2LITTLE(*pkg_ptr);
    total_size = TDTP_SIZEOF_SIZE_T + len;
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
                continue;
            default:                    
                close(self->socketfd);
                self->state = e_close;
                return robot_init(self);
            }
        }
        else
        {
            send_size += r;
        }
    }
    return TRUE;
}


int robot_init(robot_s *self)
{
    int nb = 1;
    int r;
	struct epoll_event 	ev;
	

    self->state = e_close;
    self->socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(self->socketfd == -1)
    {
        goto ERROR_RET;
    }
    
	if(ioctl(self->socketfd, FIONBIO, &nb) == -1)
	{	    
		goto ERROR_RET;
	}
	
	ev.events = EPOLLIN | EPOLLERR;
	ev.data.ptr = self;	

    memset(&self->address, 0, sizeof(self->address));
	self->address.sin_family 	    = AF_INET;
	self->address.sin_port 	    = htons(PORT);
	self->address.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    r = connect(self->socketfd, (struct sockaddr *)&self->address, sizeof(self->address));
    if(r == 0)
    {
        self->state = e_establish;
        robot_send_buff(self);
    }
    else
    {
        switch(errno)
        {
        case EINPROGRESS:
            self->state = e_syn_send;
            break;
        default:
            goto ERROR_RET;
        }        
    }

    
    if(epoll_ctl(g_epollfd, EPOLL_CTL_ADD, self->socketfd, &ev) == -1)
	{
	    goto ERROR_RET;
	}


    return TRUE;
ERROR_RET:
    return FALSE;    
}


int robot_process(robot_s *self, int pollin)
{
    char buff[BUFF_SIZE];
    
    switch(self->state)
    {
    case e_close:
        return robot_init(self);
    case e_syn_send:
        {            
            int error = 0;  
            socklen_t len = sizeof(int);  
            if(getsockopt(self->socketfd, SOL_SOCKET, SO_ERROR, &error, &len) != 0)
            {
                close(self->socketfd);
                self->state = e_close;
                return robot_init(self);
            }
            else
            {
                if(error == 0)
                {
                    self->state = e_establish;
                    return robot_send_buff(self);
                }
            }
            break;
        }
    case e_establish:
        {
            if(pollin)
            {
                int r;
                r = recv(self->socketfd, buff, BUFF_SIZE, 0);
            }
            return robot_send_buff(self);
        }
        break;
    }
    return TRUE;
}


#define ROBOT_NUM 1
#define ROBOT_MAX_EVENTS 1
int main()
{
    robot_s robot[ROBOT_NUM];
    int i;
    struct epoll_event  events[ROBOT_MAX_EVENTS];
    int                 events_num;
    size_t idle_times = 0;


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
            for(i = 0;i < events_num; ++i)
            {
                robot_process(events[i].data.ptr, TRUE);
            }	
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


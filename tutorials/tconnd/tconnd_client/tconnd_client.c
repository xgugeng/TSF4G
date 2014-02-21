
#include "tbus/tbus.h"
#include "tcommon/bscp.h"
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
#include <signal.h>
#include <assert.h>

#define TLOG_INSTANCE_LEVEL e_tlog_info
#include "tlog/tlog_instance.h"




#include "tcommon/sip.h"

#define PORT 7001
#define BUFF_SIZE 1024
int g_epollfd;
tlibc_timer_t g_timer;


#define ROBOT_NUM 1000
#define ROBOT_MAX_EVENTS 1024

tuint64 g_start_ms;
tuint64 g_connected_ms;
tuint32 g_total = 0;
tuint32 g_limit = 1000 * 1000000;
tuint32 g_server_close_connections;
tuint32 g_client_close_connections;
tuint32 g_total_connection;
tuint32 g_max_connection;
tuint32 g_cur_connection;





typedef enum _robot_state
{
    e_closed,
    e_connected,
    e_establish,
    e_connecting,
}robot_state;

typedef struct _robot_s
{
    int id;
    
    robot_state state;
    tlibc_timer_entry_t entry;
    int socketfd;
}robot_s;

tuint64 get_current_ms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec*1000 + tv.tv_usec/1000;
}

void robot_halt()
{
    tuint64 current_time_ms = get_current_ms();
    tuint64 connect_time_ms = g_connected_ms - g_start_ms;
    tuint64 send_and_recv_time_ms = current_time_ms - g_connected_ms;

    ERROR_PRINT("summary:");    
    INFO_PRINT("g_total_connect %u", g_total_connection);
    INFO_PRINT("g_max_connection %u", g_max_connection);
    INFO_PRINT("g_server_close_connections %u", g_server_close_connections);
    INFO_PRINT("g_client_close_connections %u", g_client_close_connections);
    INFO_PRINT("g_total_mb %.2lf", (double)g_total / (1024 * 1024));
    INFO_PRINT("connect_time_s %.2lf", (double)connect_time_ms / 1000);
    INFO_PRINT("send_and_recv_time_s %.2lf", (double)send_and_recv_time_ms / 1000);
    exit(0);
}

void robot_on_establish(robot_s *self)
{
    char buff[BUFF_SIZE];
    bscp_head_t *pkg_ptr = (bscp_head_t *)buff;
    char *data_ptr = buff + sizeof(bscp_head_t);
    int len;
    ssize_t total_size;
    ssize_t send_size;
    snprintf(data_ptr, BUFF_SIZE - sizeof(bscp_head_t), "hello %ld", time(0));
    len = 1024;
    *pkg_ptr = len;
    bscp_head_t_code(*pkg_ptr);
    total_size = len + 2;

    send_size = send(self->socketfd, buff, total_size, 0);
    if(send_size != total_size)
    {
        close(self->socketfd);
        self->state = e_closed;
        --g_cur_connection;
        if((send_size < 0) && (errno != EINTR) && (errno != EAGAIN) && (errno != ECONNRESET) && (errno != EPIPE))
        {
//            ERROR_PRINT("robot [%d] send errno [%d], %s\n", self->id, errno, strerror(errno));
        }
        ++g_client_close_connections;
//        WARN_PRINT("robot [%d] closed by client, total_size [%zu] send_size [%zu] g_total [%u]."
//        , self->id, total_size, send_size, g_total);
    }
    else
    {
        DEBUG_PRINT("robot [%d] send buff [%zi].", self->id, total_size);
        g_total += send_size;
        if(g_total >= g_limit)
        {
            robot_halt();
        }
    }
}

#define THINKING_INTERVAL 500

int sndbuf = 10000000;
int rcvbuf = 10000000;

void robot_on_closed(robot_s *self)
{
    int nb = 1;

    self->socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(self->socketfd == -1)
    {
        ERROR_PRINT("robot [%d] socket errno [%d], %s.", self->id, errno, strerror(errno));
        exit(1);
    }
    
  	if(ioctl(self->socketfd, FIONBIO, &nb) == -1)
	{
        ERROR_PRINT("robot [%d] socket errno [%d], %s.", self->id, errno, strerror(errno));
		exit(1);
	}

	
    if(setsockopt(self->socketfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) == -1)
	{	
        ERROR_PRINT("robot [%d] setsockopt errno[%d], %s.", self->id, errno, strerror(errno));
        exit(1);    
	}

    if(setsockopt(self->socketfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) == -1)
	{	
        ERROR_PRINT("robot [%d] setsockopt errno[%d], %s.", self->id, errno, strerror(errno));
        exit(1);
	}

    self->state = e_connected;
    DEBUG_PRINT("robot [%d] init", self->id);
}


void robot_on_connected(robot_s *self)
{
    struct sockaddr_in address;
	struct epoll_event 	ev;
    int r;

    
    memset(&address, 0, sizeof(address));
	address.sin_family 	    = AF_INET;
	address.sin_port 	    = htons(PORT);
	address.sin_addr.s_addr = inet_addr("127.0.0.1"); 	

    r = connect(self->socketfd, (struct sockaddr *)&address, sizeof(address));
    if(r != 0)
    {
        if(errno == EINPROGRESS)
        {
            ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        	ev.data.ptr = self;	
        	

            if(epoll_ctl(g_epollfd, EPOLL_CTL_ADD, self->socketfd, &ev) == -1)
        	{
                ERROR_PRINT("robot [%d] epoll_ctl errno [%d], %s", self->id, errno, strerror(errno));
        	    exit(1);
        	}
        	self->state = e_connecting;
        }
        else
        {
            self->state = e_closed;
            --g_cur_connection;
            close(self->socketfd);            
            ERROR_PRINT("robot [%d] connect errno [%d], %s", self->id, errno, strerror(errno));
            exit(1);
        }
    }
    else
    {
        assert(0);
        ERROR_LOG("oh~ my god~~~");
        exit(1);
        /*
        DEBUG_PRINT("robot [%d] connect to server.", self->id);
        ++g_total_connection;
        ++g_cur_connection;
        if(g_cur_connection > g_max_connection)
        {
            g_max_connection = g_cur_connection;
        }
    	self->state = e_establish;
    	*/
    }
}

int g_connected = FALSE;
void robot_timeout(const tlibc_timer_entry_t *super)
{
    robot_s *self = TLIBC_CONTAINER_OF(super, robot_s, entry);

    switch(self->state)
    {
    case e_closed:
        robot_on_closed(self);
        TIMER_ENTRY_BUILD(&self->entry, g_timer.jiffies, robot_timeout);
        tlibc_timer_push(&g_timer, &self->entry);
        break;
    case e_connected:
        robot_on_connected(self);
        if(self->state != e_connecting)
        {
            TIMER_ENTRY_BUILD(&self->entry, g_timer.jiffies, robot_timeout);
            tlibc_timer_push(&g_timer, &self->entry);
        }
        break;
    case e_establish:    
        robot_on_establish(self);
        if(self->state != e_establish)
        {
            TIMER_ENTRY_BUILD(&self->entry, g_timer.jiffies, robot_timeout);
        }
        else
        {
            TIMER_ENTRY_BUILD(&self->entry, g_timer.jiffies, robot_timeout);
        }
        tlibc_timer_push(&g_timer, &self->entry);
        break;
    default:
        assert(0);
        break;
    }

}

robot_s g_robot[ROBOT_NUM];


void robot_on_connect(robot_s *self)
{
    int i;
    
    DEBUG_PRINT("robot [%d] connect to server.", self->id);
    self->state = e_establish;
    ++g_total_connection;
    ++g_cur_connection;
    if(g_cur_connection > g_max_connection)
    {
        g_max_connection = g_cur_connection;
    }

    if(!g_connected)
    {
        if(g_cur_connection % 10 == 0)
        {
            INFO_PRINT("g_cur_connection = %u", g_cur_connection);
        }
        
        if(g_cur_connection == ROBOT_NUM)
        {
            g_connected = TRUE;
            g_connected_ms = get_current_ms();


            for(i = 0;i < ROBOT_NUM; ++i)
            {
                if(g_robot[i].state != e_establish)
                {
                    assert(0);
                    ERROR_LOG("robot [%d] not establish.", g_robot[i].id);
                    exit(1);
                }
                
                TIMER_ENTRY_BUILD(&g_robot[i].entry, g_timer.jiffies, robot_timeout);
                tlibc_timer_push(&g_timer, &g_robot[i].entry);
            }

            INFO_PRINT("All the robots connection established.");
        }
    }
    else
    {    
        TIMER_ENTRY_BUILD(&self->entry, g_timer.jiffies, robot_timeout);
        tlibc_timer_push(&g_timer, &self->entry);
    }
}


void robot_init(robot_s *self, int id)
{
	self->state = e_closed;
	self->id = id;
	
    robot_on_closed(self);
    if(self->state != e_connected)
    {
        ERROR_PRINT("robot [%d] state error [%d]", self->id, self->state);
        exit(1);
    }
    
    robot_on_connected(self);    
    if(self->state != e_connecting)
    {
        ERROR_PRINT("robot [%d] state error %d", self->id, self->state);
        exit(1);
    }
}


void robot_on_recv(robot_s *self)
{
    char buff[BUFF_SIZE];
    int r;
    for(;;)
    {
        r = recv(self->socketfd, buff, BUFF_SIZE, 0);
        if(r <= 0)
        {
            if (errno == EAGAIN)
            {
                break;
            }
            close(self->socketfd);
            self->state= e_closed;
            DEBUG_PRINT("robot [%d] close by server.", self->id);
            ++g_server_close_connections;
            --g_cur_connection;
            break;
        }
    }
}

int g_sig_term = FALSE;
static void on_signal(int sig)
{
    switch(sig)
    {
        case SIGINT:
        case SIGTERM:
            g_sig_term = true;
            break;
    }
}


struct epoll_event  events[ROBOT_MAX_EVENTS];
int                 events_num;

int main()
{
    int i;
    size_t idle_times = 0;
    int busy = FALSE;
    struct sigaction  sa;

    
    g_cur_connection = 0;
    g_max_connection = 0;
    g_total_connection = 0;
    g_server_close_connections = 0;
    g_start_ms = get_current_ms();
    tlibc_timer_init(&g_timer, 0);

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = SIG_IGN;
    if(sigaction(SIGPIPE, &sa, NULL) != 0)
    {
        ERROR_PRINT("sigaction error[%d], %s.", errno, strerror(errno));
        exit(1);
    }

    sa.sa_handler = on_signal;
    if((sigaction(SIGTERM, &sa, NULL) != 0)
        ||(sigaction(SIGINT, &sa, NULL) != 0))
    {
        ERROR_PRINT("sigaction error[%d], %s.", errno, strerror(errno));
        exit(1);
    }



    g_epollfd = epoll_create(ROBOT_NUM);
    if(g_epollfd == -1)
    {
        goto ERROR_RET;
    }

    for(i = 0;i < ROBOT_NUM; ++i)
    {        
        robot_init(&g_robot[i], i);
    }


    
    for(;!g_sig_term;)
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
                robot_s *socket = events[i].data.ptr;
                
                if(socket->state == e_connecting)
                {
                    robot_on_connect(socket);
                }
                else if(socket->state == e_establish)
                {
                    robot_on_recv(socket);
                }
            }
        }

        if(tlibc_timer_tick(&g_timer, get_current_ms() - g_start_ms) == E_TLIBC_NOERROR)
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


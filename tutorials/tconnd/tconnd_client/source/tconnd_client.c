#include "tbus.h"
#include "bscp.h"
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
#include "core/tlibc_timer.h"
#include "core/tlibc_error_code.h"
#include <sys/time.h>
#include <signal.h>
#include <assert.h>


#include "tlog_log.h"
#ifdef MAKE_RELEASE
#define DEBUG_PRINT_OFF
#define INFO_PRINT_OFF
#endif

#include "tlog_print.h"





#include "sip.h"

#define PORT 7001

int g_epollfd;
tlibc_timer_t g_timer;
#define ROBOT_MAX_EVENTS 1024


//#define ROBOT_NUM 1000
#define ROBOT_NUM 350


//#define BLOCK_NUM 1024 *1024
#define BLOCK_NUM 1024 * 1024

#define BLOCK_SIZE 1024 + BSCP_HEAD_T_SIZE

uint64_t block_send_ms[BLOCK_NUM];//每块发送的时间
size_t block_send_num;//总共发送了多少块
uint64_t block_recv_ms[BLOCK_NUM];//收到回包的时间
size_t block_recv_num;//总共发送了多少块



uint64_t g_start_ms = 0;
size_t g_total_recv = 0;
size_t g_total_send = 0;



uint32_t g_server_close_connections;
uint32_t g_client_close_connections;
uint32_t g_total_connection;
uint32_t g_max_connection;
uint32_t g_cur_connection;





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

static uint64_t get_current_ms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

static void robot_halt()
{
    uint64_t current_time_ms = get_current_ms();
    uint64_t send_and_recv_time_ms = current_time_ms - g_start_ms;

    ERROR_PRINT("summary:");    
    WARN_PRINT("g_total_connect %u", g_total_connection);
    WARN_PRINT("g_max_connection %u", g_max_connection);
    WARN_PRINT("g_server_close_connections %u", g_server_close_connections);
    WARN_PRINT("g_client_close_connections %u", g_client_close_connections);
    WARN_PRINT("g_total_send %.2lfmb", (double)g_total_send / (1024 * 1024));
	WARN_PRINT("g_total_recv %.2lfmb", (double)g_total_recv / (1024 * 1024));	
    WARN_PRINT("send_and_recv_time_s %.2lf", (double)send_and_recv_time_ms / 1000);
    exit(0);
}

static void robot_on_establish(robot_s *self)
{
    char buff[BLOCK_SIZE];
    bscp_head_t *pkg_ptr = (bscp_head_t *)buff;
    char *data_ptr = buff + BSCP_HEAD_T_SIZE;
    bscp_head_t len;
    ssize_t send_size;
    uint64_t cur_time_ms = get_current_ms();
/*
    if(block_send_num >= BLOCK_NUM)
    {
        return;
    }
    */
    *(uint64_t*)data_ptr = cur_time_ms;
    len = BLOCK_SIZE - BSCP_HEAD_T_SIZE;
    *pkg_ptr = len;
    
    bscp_head_t_code(*pkg_ptr);

    send_size = send(self->socketfd, buff, BLOCK_SIZE, 0);
    if(send_size < 0)
    {
        if(errno == EAGAIN)
        {
            return;
        }
        
        --g_cur_connection;
        ++g_server_close_connections;

        close(self->socketfd);
        self->state = e_closed;        
        
        if((errno != EINTR) && (errno != EAGAIN) && (errno != ECONNRESET) && (errno != EPIPE))
        {
            ERROR_PRINT("robot [%d] send errno [%d], %s", self->id, errno, strerror(errno));
        }
        else
        {
            WARN_PRINT("robot [%d] send errno [%d], %s", self->id, errno, strerror(errno));
        }
    }
    else
    {
        size_t cur_block_num;
        size_t block_size = BLOCK_SIZE;
		
		g_total_send += (size_t)send_size;
		cur_block_num = g_total_send / block_size;
		if(cur_block_num > BLOCK_NUM)
		{
    		cur_block_num = BLOCK_NUM;
		}

        for(;block_send_num < cur_block_num; ++block_send_num)
        {
            block_send_ms[block_send_num] = cur_time_ms;
            DEBUG_PRINT("robot [%d] send buff [%zi].", self->id, send_size);
        }
    }

}

#define THINKING_INTERVAL 500

int sndbuf = 10000000;
int rcvbuf = 10000000;

static void robot_on_closed(robot_s *self)
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


static void robot_on_connected(robot_s *self)
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
            ev.events = (uint32_t)(EPOLLIN | EPOLLOUT | EPOLLET);
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


static void robot_timeout(const tlibc_timer_entry_t *super)
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

/*
static void robot_on_connect(robot_s *self)
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
        if(g_cur_connection % 100 == 0)
        {
            WARN_PRINT("g_cur_connection = %u", g_cur_connection);
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
            WARN_PRINT("[%d] robots connection established.", ROBOT_NUM);
        }
    }
    else
    {    
        TIMER_ENTRY_BUILD(&self->entry, g_timer.jiffies, robot_timeout);
        tlibc_timer_push(&g_timer, &self->entry);
    }
}
*/

static void robot_init(robot_s *self, int id)
{
	self->state = e_closed;
	self->id = id;

	for(;;)
	{
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

        

        robot_on_establish(self);
        if(self->state == e_connecting)
        {
            self->state = e_establish;
            
            TIMER_ENTRY_BUILD(&self->entry, g_timer.jiffies, robot_timeout);
            tlibc_timer_push(&g_timer, &self->entry);
            break;
        }
        else
        {
            exit(1);
        }
    }
}

#define RECV_BUFF_SIZE 1024 * 1024
size_t lmb = 0;

static void robot_on_recv(robot_s *self)
{
    char buff[RECV_BUFF_SIZE];
    size_t total_size = BLOCK_SIZE - BSCP_HEAD_T_SIZE;
    ssize_t r;
	size_t cur_block_num;
	uint64_t cur_time_ms;

    r = recv(self->socketfd, buff, RECV_BUFF_SIZE, 0);
    if((r < 0) && (errno == EAGAIN))
    {           
        return;
    }
    if(r <= 0)
    {        
        ERROR_PRINT("robot [%d] close by server.", self->id);
        close(self->socketfd);
        self->state= e_closed;
        ++g_server_close_connections;
        --g_cur_connection;

        return;        
    }

    cur_time_ms = get_current_ms();
	
	g_total_recv += (size_t)r;
	cur_block_num = g_total_recv / total_size;
    if(cur_block_num >= BLOCK_NUM)
    {
        cur_block_num = BLOCK_NUM;
    }
    
    for(;block_recv_num < cur_block_num; ++block_recv_num)
    {
        size_t mb;
        block_recv_ms[block_recv_num] = cur_time_ms;
        
        mb = g_total_recv / (1024 * 1024);
        if(mb - lmb > 100)
        {
            WARN_PRINT("%zubm recv.", mb);
            lmb = mb;
        }
    }
    
    if(block_recv_num >= BLOCK_NUM)
    {
        robot_halt();                
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
    tlibc_timer_init(&g_timer);

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

    block_send_num = 0;
    block_recv_num = 0;



    g_epollfd = epoll_create(ROBOT_NUM);
    if(g_epollfd == -1)
    {
        goto ERROR_RET;
    }

    for(i = 0;i < ROBOT_NUM; ++i)
    {        
        robot_init(&g_robot[i], i);
    }
    g_cur_connection = ROBOT_NUM;
    
    
    for(;!g_sig_term;)
    {
        busy = FALSE;
        uint64_t g_cur_ms;
        
        events_num = epoll_wait(g_epollfd, events, ROBOT_MAX_EVENTS, 0);
        if(events_num == -1)
        {
            if(errno == EINTR)
            {
                busy = TRUE;
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
                /*
                if(socket->state == e_connecting)
                {
                    robot_on_connect(socket);
                }
                else if(socket->state == e_establish)
                {*/
                    robot_on_recv(socket);
                //}
            }
        }

        g_cur_ms = get_current_ms() - g_start_ms;
        while(tlibc_timer_jiffies(&g_timer) <= g_cur_ms)
        {
            if(tlibc_timer_tick(&g_timer) == E_TLIBC_NOERROR)
            {
                busy = TRUE;
            }
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
    }
	
	return 0;
ERROR_RET:
    return 1;
}


#include "tapp.h"
#include "tconnd_proto.h"
#include "core/tlibc_error_code.h"

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include "bscp.h"


#include "tlog_log.h"
#ifdef MAKE_RELEASE
#define DEBUG_PRINT_OFF
#endif
#include "tlog_print.h"

#define ROBOT_PROTO_LEN 124

#define BLOCK_SIZE 65536
#define ROBOT_NUM 10

static int sndbuf = 10000000;
static int rcvbuf = 10000000;

#define PACKET_HEAD_LENGTH sizeof(bscp_head_t)
#define PACKET_BODY_LENGTH sizeof(robot_proto_t)
#define PACKET_LENGTH PACKET_HEAD_LENGTH + PACKET_BODY_LENGTH

#define MAX_PACKET_LENGTH 65536
#define ROBOT_RECVBUF PACKET_HEAD_LENGTH + MAX_PACKET_LENGTH

typedef union packet_buff_u
{
	char packet[PACKET_LENGTH];
	bscp_head_t packet_head;
}packet_buff_t;

typedef struct robot_s
{
    int id;
    
    int socketfd;
	packet_buff_t packet_buff;

	char recvbuf[ROBOT_RECVBUF];
	size_t recvbuf_len;
	uint64_t max_delay;
	pthread_t thread_id;
	bool working;
}robot_t;

static robot_t g_robot[ROBOT_NUM];

static uint64_t get_current_ms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

static void robot_init(robot_t *self, int id)
{
	self->id = id;
	self->socketfd = -1;
	self->recvbuf_len = 0;
	self->max_delay = 0;
	self->working = false;
}

static void robot_fini(robot_t *self)
{
	self->working = false;
}

static void robot_open_connection(robot_t *self)
{
	int r;
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port  = htons(7001);
	address.sin_addr.s_addr = inet_addr("127.0.0.1");

    self->socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(self->socketfd == -1)
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

	for(;;)
	{
	    r = connect(self->socketfd, (struct sockaddr *)&address, sizeof(address));
		if(r == 0)
		{
			break;
		}
		if((errno != EINTR) && (errno != EAGAIN) &&(errno != EWOULDBLOCK))
		{
			ERROR_PRINT("robot [%d] connect errno [%d], %s", self->id, errno, strerror(errno));
			exit(1);
		}
		usleep(10000);
	}
}

static void robot_close_connection(robot_t *self)
{
	close(self->socketfd);
}

static bool robot_send(robot_t *self, const robot_proto_t *msg)
{
	size_t send_size, total_size;

	self->packet_buff.packet_head = sizeof(robot_proto_t);
	tlibc_host16_to_little(*(bscp_head_t*)self->packet);
	memcpy(self->packet_buff.packet + PACKET_HEAD_LENGTH, msg, sizeof(robot_proto_t));
	total_size = PACKET_HEAD_LENGTH + sizeof(robot_proto_t);
	send_size = 0;
	while(send_size < total_size)
	{
    	int r = send(self->socketfd, self->packet_buff.packet + send_size, total_size - send_size, 0);
		if(r >= 0)
		{
			send_size += (size_t)r;
		}
		else
		{
			if((errno != EINTR) && (errno != EWOULDBLOCK) && (errno != EAGAIN))
			{
				return false;
			}
		}
	}
	return true;
}

static bool robot_expect(robot_t *self, robot_proto_t *msg)
{
	for(;;)
	{
		int r = recv(self->socketfd, self->recvbuf + self->recvbuf_len, ROBOT_RECVBUF - self->recvbuf_len, 0);
		if(r > 0)
		{
			self->recvbuf_len += (size_t)r;
			if(self->recvbuf_len >= PACKET_BODY_LENGTH)
			{
				memcpy(msg, self->recvbuf, PACKET_BODY_LENGTH);
				memcpy(self->recvbuf, self->recvbuf + PACKET_BODY_LENGTH, self->recvbuf_len - PACKET_BODY_LENGTH);
				self->recvbuf_len -= PACKET_BODY_LENGTH;
				return true;
			}
		}
		else
		{
			if((errno != EINTR) && (errno != EWOULDBLOCK) && (errno != EAGAIN))
			{
				return false;
			}
		}
	}
	return true;
}

static void robot_test_login(robot_t *self)
{
	uint64_t send_time;
	uint64_t current_time;
	uint64_t delay_time;

	robot_open_connection(self);

	while(self->working)
	{
		robot_proto_t req;
		robot_proto_t rsp;
		req.message_id = e_robot_login_req;
		snprintf(req.message_body.login_req.name, ROBOT_STR_LEN, "robot_%d", self->id);
		snprintf(req.message_body.login_req.pass, ROBOT_STR_LEN, "%d", self->id);

		if(!robot_send(self, &req))
		{
			goto error_ret;
		}
		send_time = get_current_ms();
		if(!robot_expect(self, &rsp))
		{
			goto error_ret;
		}
		current_time = get_current_ms();
		if(rsp.message_id != e_robot_login_rsp)
		{
			ERROR_PRINT("message_id mismatch.");
		}
		if(strcmp(rsp.message_body.login_rsp.name, req.message_body.login_req.name) != 0)
		{
			ERROR_PRINT("name mismatch.");
		}
		if(rsp.message_body.login_rsp.sid != self->id)
		{
			ERROR_PRINT("sid mismatch.");
		}
		delay_time = current_time - send_time;
		if(delay_time > self->max_delay)
		{
			self->max_delay = delay_time;
		}
		sleep(1);
	}
error_ret:
	robot_close_connection(self);
}


static void *robot_work(void *arg)
{
	robot_t *self = (robot_t*)arg;
	robot_test_login(self);
	self->working = false;
	return NULL;
}

static void init()
{
	int i;
	for(i = 0; i < ROBOT_NUM; ++i)
	{
		robot_init(&g_robot[i], i);
	}
}

static TERROR_CODE process()
{
	TERROR_CODE ret = E_TS_WOULD_BLOCK;
	size_t i;
	for(i = 0; i < ROBOT_NUM;++i)
		if(!g_robot[i].working)
		{
			if(pthread_create(&g_robot[i].thread_id, NULL, robot_work, &g_robot[i]) == 0)
			{
				g_robot[i].working = true;
			}
		}
	return ret;
}

static void fini()
{
	int i;
	uint64_t max_delay = 0;
	void *res;

	for(i = 0; i < ROBOT_NUM; ++i)
	{
		robot_fini(&g_robot[i]);

		if(g_robot[i].max_delay > max_delay)
		{
			max_delay = g_robot[i].max_delay;
		}
	}

	for(i = 0; i < ROBOT_NUM; ++i)
	{
		pthread_join(g_robot[i].thread_id, &res);
	}
	INFO_PRINT("max_delay = %llums", max_delay);
	
}

int main(int argc, char *argv[])
{
	int ret;
	init();

	if(tapp_loop(process, TAPP_IDLE_USEC, TAPP_IDLE_LIMIT, NULL, NULL) == E_TS_NOERROR)
	{
		ret = 0;
	}
	else
	{
		ret = 1;
	}
	fini();
	return 0;
}


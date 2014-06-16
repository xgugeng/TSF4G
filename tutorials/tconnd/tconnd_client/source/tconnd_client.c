#include "tapp.h"
#include "tconnd_proto.h"
#include "core/tlibc_error_code.h"
#include "tconnd_robot_config_types.h"
#include "tconnd_robot_config_reader.h"

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
#include <math.h>
#include "bscp_types.h"

#include <stdbool.h>



#include "tlog_log.h"
#ifdef MAKE_RELEASE
#define DEBUG_PRINT_OFF
#endif
#include "tlog_print.h"

#define IDLE_TIME_US 1000
#define ROBOT_PROTO_LEN 124

#define BLOCK_SIZE 65536
#define MAX_ROBOT_NUM 1024 

static int sndbuf = 10000000;
static int rcvbuf = 10000000;
static tconnd_robot_config_t g_config;

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

volatile bool g_working;

typedef struct robot_s
{
    int id;
    
    int socketfd;
	packet_buff_t packet_buff;

	char recvbuf[ROBOT_RECVBUF];
	size_t recvbuf_len;
	uint32_t rtt_min, rtt_max, rtt_count;
	uint64_t rtt_total;
	pthread_t thread_id;
	uint64_t total_send;
	uint64_t start_time;
	uint64_t start_size;
	
	uint64_t total_recv;

    uint32_t lost_connection;
}robot_t;

static robot_t g_robot[MAX_ROBOT_NUM];

uint64_t g_prog_starting_ms;

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
	self->rtt_min = 0xffffffff;
	self->rtt_max = 0;
	self->rtt_total = 0;
	self->rtt_count = 0;
	self->total_send = 0;
	self->start_time = get_current_ms();
	self->start_size = 0;
    self->total_recv = 0;
	self->lost_connection = 0;
}

static void robot_open_connection(robot_t *self)
{
    int nb;
	int r;
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port  = htons(g_config.port);
	address.sin_addr.s_addr = inet_addr(g_config.ip);

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
	
    nb = 1;
	if(ioctl(self->socketfd, FIONBIO, &nb) == -1)
	{
        ERROR_PRINT("robot [%d] ioctl errno[%d], %s.", self->id, errno, strerror(errno));
		exit(1);
	}

	for(;;)
	{
	    r = connect(self->socketfd, (struct sockaddr *)&address, sizeof(address));
		if(r == 0)
		{
			break;
		}
		if(errno == EINPROGRESS)
		{
		    break;
		}
		if((errno != EINTR) && (errno != EAGAIN) &&(errno != EWOULDBLOCK))
		{
			ERROR_PRINT("robot [%d] connect errno [%d], %s", self->id, errno, strerror(errno));
			exit(1);
		}
		usleep(IDLE_TIME_US);
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
	memcpy(self->packet_buff.packet + PACKET_HEAD_LENGTH, msg, sizeof(robot_proto_t));
	total_size = PACKET_HEAD_LENGTH + sizeof(robot_proto_t);
	send_size = 0;
	while(send_size < total_size)
	{
    	ssize_t r = send(self->socketfd, self->packet_buff.packet + send_size, total_size - send_size, 0);
		if(r >= 0)
		{
			send_size += (size_t)r;
		}
		else
		{
			if((errno != EINTR) && (errno != EWOULDBLOCK) && (errno != EAGAIN))
			{
    			++self->lost_connection;
				return false;
			}
			usleep(IDLE_TIME_US);
		}

		if(!g_working)
		{
		    return false;
		}
	}
    self->total_send += total_size;
	return true;
}

static bool robot_expect(robot_t *self, robot_proto_t *msg)
{
	for(;;)
	{
		ssize_t r = recv(self->socketfd, self->recvbuf + self->recvbuf_len, ROBOT_RECVBUF - self->recvbuf_len, 0);
		if(r > 0)
		{
			self->recvbuf_len += (size_t)r;
            self->total_recv += (uint64_t)r;
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
                ++self->lost_connection;
				return false;
			}
            usleep(IDLE_TIME_US);
		}

		if(!g_working)
		{
		    return false;
		}
	}
}

static void robot_test_login(robot_t *self)
{
	uint64_t start_time;
	uint64_t start_size;
	uint64_t send_time;
	uint64_t recv_time;
	uint64_t rtt;


	robot_open_connection(self);	
	start_time = get_current_ms();
	start_size = self->total_send;

	while(g_working)
	{
		robot_proto_t req;
		robot_proto_t rsp;
		int rand_num = rand();
		uint64_t diff_ms = get_current_ms() - start_time;
		if((diff_ms > 0) && (g_config.speed))
		{
			if(((self->total_send - start_size) / diff_ms) * 1000 > g_config.speed)
			{
				goto idle;
			}
		}
		
		
		req.message_id = e_robot_login_req;
		snprintf(req.message_body.login_req.name, ROBOT_STR_LEN, "robot_%d", self->id);
		snprintf(req.message_body.login_req.pass, ROBOT_STR_LEN, "%d", rand_num);

		send_time = get_current_ms();
		if(!robot_send(self, &req))
		{
			goto error_ret;
		}

		memset(&rsp, 0, sizeof(robot_proto_t));

		if(!robot_expect(self, &rsp))
		{
			goto error_ret;
		}
		recv_time = get_current_ms();
		
		if(rsp.message_id != e_robot_login_rsp)
		{
			ERROR_PRINT("message_id mismatch.");
			exit(1);
		}
		if(strcmp(rsp.message_body.login_rsp.name, req.message_body.login_req.name) != 0)
		{
			ERROR_PRINT("name mismatch.");
			exit(1);
		}
		if(rsp.message_body.login_rsp.sid != rand_num)
		{
			ERROR_PRINT("sid mismatch.");
			exit(1);
		}
		
		rtt = recv_time - send_time;


		self->rtt_total += rtt;
		++self->rtt_count;
		if(rtt > self->rtt_max)
		{
			self->rtt_max = (uint32_t)rtt;
		}
		if(rtt < self->rtt_min)
		{
			self->rtt_min = (uint32_t)rtt;
		}
idle:
		usleep(IDLE_TIME_US);
	}
error_ret:
	robot_close_connection(self);
}


static void *robot_work(void *arg)
{
	robot_t *self = (robot_t*)arg;
	robot_test_login(self);
	return NULL;
}

static void init()
{
	int i;
	g_prog_starting_ms = get_current_ms();
	g_working = true;
	for(i = 0; i < g_config.robot_num; ++i)
	{
		robot_init(&g_robot[i], i);
		if(pthread_create(&g_robot[i].thread_id, NULL, robot_work, &g_robot[i]) != 0)
		{
			ERROR_PRINT("robot[%d] pthread_create errno %d, %s", i, errno, strerror(errno));
			exit(1);
		}
	}
	srand((unsigned int)time(NULL));
}

#define PRINT_INTERVAL_MS 1000
static uint64_t last_print_time = 0;

static TERROR_CODE process(void *arg)
{
	uint32_t rtt_max, rtt_min;
	TERROR_CODE ret = E_TS_WOULD_BLOCK;
	size_t i;
	uint64_t current_time = get_current_ms();
	double total_speed;


	total_speed = 0;
	rtt_max = 0;
	rtt_min = 0xffffffff;

	for(i = 0; i < g_config.robot_num;++i)
	{
		double diff_byte = (double)g_robot[i].total_send - (double)g_robot[i].start_size;
		uint64_t diff_ms = current_time - g_robot[i].start_time;
		double speed;
		if(diff_ms < 1)
		{
			continue;
		}
		speed = ((diff_byte / 1024) / (double)diff_ms) * 1000;

		total_speed += speed;
		g_robot[i].start_time = current_time;
		g_robot[i].start_size = g_robot[i].total_send;

		if(g_robot[i].rtt_max > rtt_max)
		{
			rtt_max = g_robot[i].rtt_max;
		}
		if(g_robot[i].rtt_min < rtt_min)
		{
			rtt_min = g_robot[i].rtt_min;
		}
	}

	if(current_time - last_print_time >= PRINT_INTERVAL_MS)
	{
		last_print_time = current_time;
		
	    INFO_PRINT("rtt(min, max) : %ums, %ums, speed(total, total / %u) : %.2lfkb/s %.2lfkb/s", rtt_min, rtt_max
		    , g_config.robot_num , total_speed, total_speed / g_config.robot_num);
	}
	return ret;
}

/*
2014-04-05 09:49:25 [info] source/tconnd_client.c:433 :     testing_time : 17s
2014-04-05 09:49:25 [info] source/tconnd_client.c:434 :     packet_len : 256byte
2014-04-05 09:49:25 [info] source/tconnd_client.c:435 :     robot_num : 100
2014-04-05 09:49:25 [info] source/tconnd_client.c:436 :     lose_connection : 0
2014-04-05 09:49:25 [info] source/tconnd_client.c:437 :     totl_send : 109.98mb
2014-04-05 09:49:25 [info] source/tconnd_client.c:438 :     total_recv : 109.12mb
2014-04-05 09:49:25 [info] source/tconnd_client.c:439 :     rtt(min, max, avg) : 1ms 9ms 3ms
*/

static void fini()
{
	int i;
	void *res;
	uint64_t rtt_total, rtt_count;
	uint32_t rtt_min, rtt_max;
	uint64_t total_send, total_recv;
    uint32_t lose_connection;
	uint64_t testing_time;

	g_working = false;
    

	for(i = 0; i < g_config.robot_num; ++i)
	{
		pthread_join(g_robot[i].thread_id, &res);
	}

	rtt_total = 0;
	rtt_count = 0;
	rtt_min = 0xffffffff;
	rtt_max = 0;
	total_send = 0;
	total_recv = 0;
	lose_connection = 0;
	for(i = 0; i < g_config.robot_num; ++i)
	{
		rtt_total += g_robot[i].rtt_total;
		rtt_count += g_robot[i].rtt_count;		
		if(g_robot[i].rtt_min < rtt_min)
		{
		    rtt_min = g_robot[i].rtt_min;
		}
		if(g_robot[i].rtt_max > rtt_max)
		{
		    rtt_max = g_robot[i].rtt_max;
		}
		total_send += g_robot[i].total_send;
        total_recv += g_robot[i].total_recv;
        lose_connection += g_robot[i].lost_connection;
	}
	testing_time = get_current_ms() - g_prog_starting_ms;
	if(testing_time < 1000)
	{
	    testing_time = 1000;
	}
	WARN_PRINT("Summary:");
	INFO_PRINT("    robot_num : %u", g_config.robot_num);
    INFO_PRINT("    lose_connection : %u", lose_connection);
	INFO_PRINT("    packet_len : %ubyte", sizeof(robot_proto_t));
	INFO_PRINT("    testing_time : %llus", testing_time / 1000);
    INFO_PRINT("    totl_send : %.2lfmb", (double)total_send / (double)(1024 * 1024));
	INFO_PRINT("    total speed : %llukb/s", (total_send / 1024) / (testing_time / 1000));
	INFO_PRINT("    speed : %llukb/s", (total_send / 1024) / (testing_time / 1000) / g_config.robot_num );
    INFO_PRINT("    total_recv : %.2lfmb", (double)total_recv / (double)(1024 * 1024));
	if(rtt_count == 0)
	{
		rtt_total = 0;
		rtt_count = 1;
	}
    INFO_PRINT("    rtt(min, max, avg) : %ums %ums %llums", rtt_min, rtt_max, rtt_total / rtt_count);
}


int main(int argc, char *argv[])
{
	int ret;
	tapp_load_config(&g_config, argc, argv, (tapp_xml_reader_t)tlibc_read_tconnd_robot_config);

	init();

	if(tapp_loop(TAPP_IDLE_USEC, TAPP_IDLE_LIMIT, NULL, NULL, NULL, NULL
	            , process, NULL
	            , NULL, NULL) == E_TS_NOERROR)
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


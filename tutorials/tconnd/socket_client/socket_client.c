/*
 * 网络上不断发送数据， 直到出错为止。
*/
#ifdef TLOG_PRINT_LEVEL
#undef TLOG_PRINT_LEVEL
#endif//
#define TLOG_PRINT_LEVEL e_tlog_debug

#include <stdio.h>
#include <string.h>
#include "tlibc/platform/tlibc_platform.h"

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
#include <signal.h>
#include <assert.h>
#include <stdint.h>
#include "tlog/tlog_print.h"

const char* g_host = NULL;
int g_port = 0;
size_t g_block_size = 0;

static size_t g_sndbuf = 10 * 1000000;
static int g_socketfd;
static char *g_buff;
static uint64_t g_start_ms;
static uint64_t g_end_ms;

static uint64_t get_current_ms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}



static void init()
{
	struct sockaddr_in address;
	int r;
	int nb = 1;
	struct sigaction  sa;
	
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = SIG_IGN;
	if(sigaction(SIGPIPE, &sa, NULL) != 0)
	{
		ERROR_PRINT("sigaction error[%d], %s.", errno, strerror(errno));
		exit(1);
	}

    g_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(g_socketfd == -1)
    {
        ERROR_PRINT("socket errno [%d], %s.", errno, strerror(errno));
        exit(1);
    }
    
  	if(ioctl(g_socketfd, FIONBIO, &nb) == -1)
	{
        ERROR_PRINT("ioctl errno [%d], %s.", errno, strerror(errno));
		exit(1);
	}

	
    if(setsockopt(g_socketfd, SOL_SOCKET, SO_SNDBUF, &g_sndbuf, sizeof(g_sndbuf)) == -1)
	{	
        ERROR_PRINT("setsockopt errno[%d], %s.", errno, strerror(errno));
        exit(1);    
	}

	memset(&address, 0, sizeof(address));
	address.sin_family 	    = AF_INET;
	address.sin_port 	    = htons((uint16_t)g_port);
	address.sin_addr.s_addr = inet_addr(g_host); 	
	r = connect(g_socketfd, (struct sockaddr *)&address, sizeof(address));
	if(r != -1)
	{
		assert(0);
		ERROR_PRINT("what's happen??");
		exit(1);
	}
	if(errno != EINPROGRESS)
	{
		ERROR_PRINT("connect reutrn errno [%d], %s", errno, strerror(errno));
		exit(1);
	}
}
static void work()
{
	size_t idle_times = 0;	
	ssize_t send_size;
	size_t total_size = 0;
	int busy = FALSE;
	size_t lmb, mb;

	lmb = 0;
	for(;;)
	{
		busy = FALSE;
		send_size = send(g_socketfd, g_buff, g_block_size, 0);
		if(send_size < 0)
		{
			switch(errno)
			{
			case EINTR:
				busy = TRUE;
				break;
			case EAGAIN:
				break;
			default:
				ERROR_PRINT("send errno [%d], %s", errno, strerror(errno));
				exit(1);
			}	
		}
		else
		{
			busy = TRUE;
			total_size += (size_t)send_size;

			mb = total_size / (1024 * 1024);
			if((mb != lmb) && (mb % 100 == 0))
			{
				DEBUG_PRINT("%zubm send.", mb);
				lmb = mb;
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
}

static void fini()
{
	close(g_socketfd);
}

static void help()
{
	fprintf(stderr, "Usage: socket_client [options]\n");
	fprintf(stderr, "Options:\n");	
	fprintf(stderr, "  -b						Set the block size.\n");
	fprintf(stderr, "  -h						Set the ip to connect.\n");
	fprintf(stderr, "  -p						Set the port to connect.\n");	
	fprintf(stderr, "  -snd						Set the size of send buff, default value is 10m.\n");	
}

int main(int argc, char* argv[])
{
	int i;

	for (i = 1; i < argc; ++i)
	{
		char* arg;

		arg = strtok(argv[i], " ");
		if (arg[0] == '-' && arg[1] == '-')
		{
			++arg;
		}
		if (strcmp(arg, "-b") == 0)
		{
			arg = argv[i + 1];
			if(i >= argc)
			{
				help();
				exit(1);
			}
			errno = 0;
			g_block_size = (size_t)strtoull(arg, NULL, 10);
			if(errno != 0)
			{
				ERROR_PRINT("strtoull return errno [%d], %s.", errno, strerror(errno));
				exit(1);
			}
			g_buff = (char*)malloc(g_block_size);
			if(g_buff == NULL)
			{
				ERROR_PRINT("malloc(%zu) return NULL.", g_block_size);
				exit(1);
			}
		}
		else if (strcmp(arg, "-h") == 0)
		{
			arg = argv[i + 1];
			if(i >= argc)
			{
				help();
				exit(1);
			}
			g_host = arg;
		}
		else if (strcmp(arg, "-p") == 0)
		{
			arg = argv[i + 1];
			if(i >= argc)
			{
				help();
				exit(1);
			}
			g_port = (int)strtol(arg, NULL, 0);
			if(errno != 0)
			{
				ERROR_PRINT("strtoull return errno [%d], %s.", errno, strerror(errno));
				exit(1);
			}
		}
		else if (strcmp(arg, "-snd") == 0)
		{
			arg = argv[i + 1];
			if(i >= argc)
			{
				help();
				exit(1);
			}
			errno = 0;
			g_sndbuf = (size_t)strtoull(arg, NULL, 10);
			if(errno != 0)
			{
				ERROR_PRINT("strtoull return errno [%d], %s.", errno, strerror(errno));
				exit(1);
			}
		}

		arg = strtok(NULL, " =");
	}
	if((g_host == NULL) || (g_port == 0) || (g_buff == NULL))
	{
		help();
		exit(1);
	}

	init();
	g_start_ms = get_current_ms();	
	work();
	g_end_ms = get_current_ms();
	DEBUG_PRINT("ms = %"PRIu64, g_end_ms - g_start_ms);
	fini();

	return 0;   
}


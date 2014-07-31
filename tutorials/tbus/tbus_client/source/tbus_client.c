#include "tbusapi.h"

#include <sys/ipc.h>
#include <sys/shm.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define SHM_KEY 123456
#define MAX_MESSAGE_LENGTH 1024

tbusapi_t g_tbusapi;
tbus_t    *g_otb;

static tbus_atomic_size_t encode(const char *data, char *start, char *limit)
{
	size_t len = strlen(data) + 1;
	if(limit - start < len)
	{
		return 0;
	}
	memcpy(start, data, len);
	return (tbus_atomic_size_t)len;
}

int main()
{
	uint32_t i;

	g_otb = tbus_at(SHM_KEY);
	if(g_otb == NULL)
	{
		fprintf(stderr, "tbusapi_init failed.\n");
		exit(1);
	}
	tbusapi_init(&g_tbusapi, 0, g_otb, (tlibc_encode_t)encode);
	
	for(i = 0;i < 10;++i)
	{
		char data[MAX_MESSAGE_LENGTH];
		
		snprintf(data, MAX_MESSAGE_LENGTH, "hello %d", i);
		data[MAX_MESSAGE_LENGTH - 1] = 0;

		tbusapi_send(&g_tbusapi, data);
		//任何时刻都不能让tbus堆满消息！
//		sleep(1);
	}
	tbus_dt(g_otb);
	return 0;
}


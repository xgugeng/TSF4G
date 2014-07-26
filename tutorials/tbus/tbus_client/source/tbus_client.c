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

int main()
{
	uint32_t i;

	g_otb = tbus_at(SHM_KEY);
	if(g_otb == NULL)
	{
		fprintf(stderr, "tbusapi_init failed.\n");
		exit(1);
	}
	tbusapi_init(&g_tbusapi, 0, g_otb);
	
	for(i = 0;i < 10;++i)
	{
		char data[MAX_MESSAGE_LENGTH];
		tbus_atomic_size_t data_size;
		
		snprintf(data, MAX_MESSAGE_LENGTH, "hello %d", i);
		data[MAX_MESSAGE_LENGTH - 1] = 0;

		data_size = (tbus_atomic_size_t)(strlen(data) + 1);

		tbusapi_send(&g_tbusapi, data, data_size);
		//任何时刻都不能让tbus堆满消息！
//		sleep(1);
	}
	tbus_dt(g_otb);
	return 0;
}


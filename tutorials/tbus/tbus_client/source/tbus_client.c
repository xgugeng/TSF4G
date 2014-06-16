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

int main()
{
	uint32_t i;

	if(tbusapi_init(&g_tbusapi, 0, 0, SHM_KEY) != E_TLIBC_NOERROR)
	{
		fprintf(stderr, "tbusapi_init failed.\n");
		exit(1);
	}
	
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
	return 0;
}


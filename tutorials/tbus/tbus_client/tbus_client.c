#include "tbus/tbus.h"

#include <sys/ipc.h>
#include <sys/shm.h>


#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define SHM_KEY 123456

#define MAX_MESSAGE_LENGTH 1024

int main()
{
	int shm_id = shmget(SHM_KEY, 0, 0666);
	tbus_t *tb = shmat(shm_id, NULL, 0);
	tuint32 i;
	
	for(i = 0;i < 10;)
	{
		TERROR_CODE ret;
		char data[MAX_MESSAGE_LENGTH];
		size_t data_size;
		
		char *message;
		size_t message_size;
		
		snprintf(data, MAX_MESSAGE_LENGTH, "hello %d", i);
		data[MAX_MESSAGE_LENGTH - 1] = 0;

		data_size = strlen(data) + 1;
		message_size = data_size;
		
		ret = tbus_send_begin(tb, &message, &message_size);
		if(ret == E_TS_NOERROR)
		{
			memcpy(message, data, data_size);
			tbus_send_end(tb, data_size);
			++i;
			continue;
		}
		if(ret == E_TS_TBUS_NOT_ENOUGH_SPACE)
		{
			usleep(1000);
		}
		else
		{
			printf("error.\n");
			exit(1);
		}
	}
	return 0;
}


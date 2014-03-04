#include "tbus.h"

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
	uint32_t i;
	
	for(i = 0;i < 10;)
	{
		char data[MAX_MESSAGE_LENGTH];
		tbus_atomic_size_t data_size;
		
		char *message;
		tbus_atomic_size_t message_size = 0;
		
		snprintf(data, MAX_MESSAGE_LENGTH, "hello %d", i);
		data[MAX_MESSAGE_LENGTH - 1] = 0;

		data_size = (tbus_atomic_size_t)(strlen(data) + 1);
		message_size = data_size;
		
		message_size = tbus_send_begin(tb, &message, data_size);
		if(data_size <=  message_size)
		{
			memcpy(message, data, (size_t)data_size);
			tbus_send_end(tb, (tbus_atomic_size_t)data_size);			
            printf("message_size: %u\n", message_size);
            printf("data_size: %u\n", data_size);
			++i;
			continue;
		}
		else
		{
			usleep(1000);
		}
	}
	return 0;
}


#include "tbus.h"


#include <sys/ipc.h>
#include <sys/shm.h>


#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define SHM_KEY 123456

char *message = NULL;

int main()
{
	int shm_id = shmget(SHM_KEY, 0, 0666);
	tbus_t *tb = shmat(shm_id, NULL, 0);
	tbus_atomic_size_t len = 0;
	uint32_t i;

	for(i = 0;; ++i)
	{
		len = tbus_read_begin(tb, &message);
		if(len == 0)
		{
    		usleep(1000);
			continue;
		}
		else
		{
            printf("recv %u bytes, message:%s\n", len, message);
			tbus_read_end(tb, len);	
		}
	}
	
	return 0;
}


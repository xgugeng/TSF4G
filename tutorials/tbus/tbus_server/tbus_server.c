#include "tbus/tbus.h"


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
	size_t len = 0;
	TERROR_CODE ret;
	uint32_t i;

	for(i = 0;; ++i)
	{
		ret = tbus_read_begin(tb, &message, (uint32_t*)&len);
		if(ret == E_TS_NOERROR)
		{
			printf("recv %zu bytes, message:%s\n", len, message);
			tbus_read_end(tb, (uint32_t)len);
			continue;
		}
		else if(ret == E_TS_WOULD_BLOCK)
		{
		//	printf("tbus empty.\n");
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


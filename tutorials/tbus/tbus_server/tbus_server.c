#include "tserver/tbus/tbus.h"


#include <sys/ipc.h>
#include <sys/shm.h>


#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define SHM_KEY 123456

const char *message = NULL;

int main()
{
	int shm_id = shmget(SHM_KEY, 0, 0666);
	tbus_t *tb = shmat(shm_id, NULL, 0);
	tuint16 len;
	TERROR_CODE ret;
	tuint32 i;

	for(i = 0;; ++i)
	{
		ret = tbus_peek(tb, &message, &len);
		if(ret == E_TS_NOERROR)
		{
			printf("recv %u bytes, message:%s\n", len, message);
			tbus_peek_over(tb, len);
			continue;
		}
		else if(ret == E_TS_AGAIN)
		{			
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


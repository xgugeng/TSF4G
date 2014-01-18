#include "tserver/tbus/tbus.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define SHM_KEY 123456

#define MAX_MESSAGE_LENGTH 1024

int main()
{
	tuint32 i;
	tbus_t *tb = NULL;
	tb = tbus_open(SHM_KEY);
	
	for(i = 0;i < 10;)
	{
		TERROR_CODE ret;
		char message[MAX_MESSAGE_LENGTH];		
		snprintf(message, MAX_MESSAGE_LENGTH, "hello %d", i);
		message[MAX_MESSAGE_LENGTH - 1] = 0;
		
		ret = tbus_send(tb, message, strlen(message) + 1);
		if(ret == E_TS_NOERROR)
		{
			++i;
			continue;
		}
		if(ret == E_TS_WOULD_BLOCK)
		{
			usleep(1000);
		}
		else if(ret == E_TS_AGAIN)
		{
			continue;
		}
		else
		{
			printf("error.\n");
			exit(1);
		}
	}
	return 0;
}


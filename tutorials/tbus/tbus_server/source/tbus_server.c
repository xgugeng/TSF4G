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
    struct iovec iov[1];
	size_t iov_num;
	uint32_t i;
	tbus_atomic_size_t tbus_head;

	for(i = 0;; ++i)
	{
		iov_num = 1;
		tbus_head = tbus_read_begin(tb, iov, &iov_num);
		if(iov_num == 0)
		{
		    //可能是ignore操作， 也可能是由于包错误导致的清空缓存。
		    if(tbus_head != tb->head_offset)
		    {
		        //busy!
		        goto read_end;
		    }
		    else
		    {
    		    usleep(1000);
    		    continue;
		    }
		}
		
        printf("recv %zu bytes, message:%s\n", iov[0].iov_len, (const char*)iov[0].iov_base);
read_end:
		tbus_read_end(tb, tbus_head);	
	}
	
	return 0;
}


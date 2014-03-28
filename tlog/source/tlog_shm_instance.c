#include "tlog_shm_instance.h"

#include <sys/shm.h>


TERROR_CODE tlog_shm_instance_init(tlog_shm_instance_t *self, const tlog_shm_t *config)
{
    TERROR_CODE ret = E_TS_NOERROR;
    int input_tbusid;

	input_tbusid = shmget(config->input_tbuskey, 0, 0666);
	if(input_tbusid == -1)
	{
	    ret = E_TS_ERRNO;
		goto done;
	}
	self->itb = shmat(input_tbusid, NULL, 0);
	if(self->itb == NULL)
	{
	    ret = E_TS_ERRNO;
		goto done;
	}
	
done:
    return ret;
}

void tlog_shm_instance_log(tlog_shm_instance_t *self, 
		const tlog_shm_t *config,
		const char *message, size_t message_size)
{
}

void tlog_shm_instance_fini(tlog_shm_instance_t *self)
{
}



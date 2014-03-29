#include "appender/tlog_appender_shm.h"

#include <sys/shm.h>


TERROR_CODE tlog_appender_shm_init(tlog_appender_shm_t *self, const tlog_config_appender_shm_t *config)
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

void tlog_appender_shm_log(tlog_appender_shm_t *self, const tlog_config_appender_shm_t *config, const tlog_message_t *message)
{
}

void tlog_appender_shm_fini(tlog_appender_shm_t *self)
{
}



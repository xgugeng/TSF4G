#include "appender/tlog_appender_shm.h"

#include "tlog_config_writer.h"
#include "protocol/tlibc_binary_writer.h"

#include <sys/shm.h>


TERROR_CODE tlog_appender_shm_init(tlog_appender_shm_t *self, const tlog_config_appender_shm_t *config)
{
    TERROR_CODE ret = E_TS_NOERROR;
    int input_tbusid;

	input_tbusid = shmget(config->output_tbuskey, 0, 0666);
	if(input_tbusid == -1)
	{
	    ret = E_TS_ERRNO;
		goto done;
	}
	self->otb = shmat(input_tbusid, NULL, 0);
	if(self->otb == NULL)
	{
	    ret = E_TS_ERRNO;
		goto done;
	}
	
done:
    return ret;
}

void tlog_appender_shm_log(tlog_appender_shm_t *self, const tlog_config_appender_shm_t *config, const tlog_message_t *message)
{
    tbus_atomic_size_t tbus_writer_size;
    char *ptr;
    TLIBC_BINARY_WRITER bwriter;

    tbus_writer_size = tbus_send_begin(self->otb, &ptr, sizeof(tlog_message_t));
    if(tbus_writer_size < sizeof(tlog_message_t))
    {
        goto done;
    }

    tlibc_binary_writer_init(&bwriter, ptr, tbus_writer_size);
    if(tlibc_write_tlog_message(&bwriter.super, message) != E_TLIBC_NOERROR)
    {
        goto done;
    }

    tbus_send_end(self->otb, bwriter.offset);
    
    return;
done:
    return;
}

void tlog_appender_shm_fini(tlog_appender_shm_t *self)
{
    shmdt(self->otb);
}



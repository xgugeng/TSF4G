#ifndef _H_TLOG_APPENDER_SHM_H
#define _H_TLOG_APPENDER_SHM_H

#include "tbus.h"
#include "terrno.h"
#include "tlog_config_types.h"

typedef struct tlog_shm_instance_s
{
    tbus_t *itb;
}tlog_shm_instance_t;

TERROR_CODE tlog_shm_instance_init(tlog_shm_instance_t *self, const tlog_config_appender_shm_t *config);

void tlog_shm_instance_log(tlog_shm_instance_t *self, const tlog_config_appender_shm_t*config, const tlog_message_t *message);

void tlog_shm_instance_fini(tlog_shm_instance_t *self);


#endif//_H_TLOG_APPENDER_SHM_H


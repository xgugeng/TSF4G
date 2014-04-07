#ifndef _H_TLOG_APPENDER_SHM_H
#define _H_TLOG_APPENDER_SHM_H

#include "tbus.h"
#include "terrno.h"
#include "tlog_config_types.h"

typedef struct tlog_appender_shm_s
{
    tbus_t *otb;
}tlog_appender_shm_t;

TERROR_CODE tlog_appender_shm_init(tlog_appender_shm_t *self, const tlog_config_appender_shm_t *config);

void tlog_appender_shm_log(tlog_appender_shm_t *self, const tlog_config_appender_shm_t*config, const tlog_message_t *message);

void tlog_appender_shm_fini(tlog_appender_shm_t *self);


#endif//_H_TLOG_APPENDER_SHM_H


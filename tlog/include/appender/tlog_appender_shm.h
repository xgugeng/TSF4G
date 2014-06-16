#ifndef _H_TLOG_APPENDER_SHM_H
#define _H_TLOG_APPENDER_SHM_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "tbus.h"
#include "tlibc_error_code.h"
#include "tlog_config_types.h"

typedef struct tlog_appender_shm_s
{
    tbus_t *otb;
}tlog_appender_shm_t;

TERROR_CODE tlog_appender_shm_init(tlog_appender_shm_t *self, const tlog_config_appender_shm_t *config);

void tlog_appender_shm_log(tlog_appender_shm_t *self, const tlog_config_appender_shm_t*config, const tlog_message_t *message);

void tlog_appender_shm_fini(tlog_appender_shm_t *self);


#ifdef  __cplusplus
}
#endif

#endif//_H_TLOG_APPENDER_SHM_H


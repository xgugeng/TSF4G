#ifndef _H_TLOG_SHM_INSTANCE_H
#define _H_TLOG_SHM_INSTANCE_H

#include "tbus.h"
#include "terrno.h"
#include "tlog_config_types.h"

typedef struct tlog_shm_instance_s
{
    tbus_t *itb;
}tlog_shm_instance_t;

TERROR_CODE tlog_shm_instance_init(tlog_shm_instance_t *self, const tlog_shm_t *config);

void tlog_shm_instance_log(tlog_shm_instance_t *self, 
		const tlog_shm_t *config,
		const char *message, size_t message_size);

void tlog_shm_instance_fini(tlog_shm_instance_t *self);


#endif//_H_TLOG_SHM_INSTANCE_H


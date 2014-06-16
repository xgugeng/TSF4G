#ifndef _H_TLOG
#define _H_TLOG

#ifdef  __cplusplus
extern "C" {
#endif

#include "tlog_config_types.h"
#include "appender/tlog_appender_rolling_file.h"
#include "appender/tlog_appender_shm.h"


#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "tlibc_error_code.h"

#define TLOG_VERSION "0.0.1"
typedef union tlog_appender_body_u
{
	tlog_appener_rolling_file_t rolling_file;
	tlog_appender_shm_t shm;
}tlog_appender_body_t;

typedef struct tlog_appender_s
{
	tlog_appender_type_t type;
	tlog_appender_body_t appender;
}tlog_appender_t;

typedef struct tlog_instance_s
{
	uint32_t appender_vec_num;
	tlog_appender_t appender_vec[TLOG_MAX_APPENDER_NUM];
}tlog_instance_t;

typedef struct tlog_s
{
	tlog_config_t config;
	tlog_instance_t instance;
}tlog_t;


tlibc_error_code_t tlog_init(tlog_t *self, const tlog_config_t *config);

//所有的write都不带缓存，每次写入都将被立即执行， 如果确有需要可以增加缓存提高效率。
void tlog_write(tlog_t *self, const tlog_message_t *message);

void tlog_fini(tlog_t *self);

#ifdef  __cplusplus
}
#endif

#endif//_H_TLOG


#ifndef _H_TLOG_APPENDER_ROLLING_FILE_H
#define _H_TLOG_APPENDER_ROLLING_FILE_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "tlog_config_reader.h"
#include "tlog_message_types.h"

#include <stdio.h>

#define TLOG_APPENDER_ROLLING_FILE_BUFF_SIZE 65535

typedef struct tlog_appender_rolling_file_s
{
	FILE *fout;
	uint32_t index;
	char buff[TLOG_APPENDER_ROLLING_FILE_BUFF_SIZE];
}tlog_appener_rolling_file_t;


void tlog_appender_rolling_file_init(tlog_appener_rolling_file_t *self, const tlog_config_appender_rolling_file_t *config);

void tlog_appender_rolling_file_log(tlog_appener_rolling_file_t *self, const tlog_config_appender_rolling_file_t *config, const tlog_message_t *message);

void tlog_appender_rolling_file_fini(tlog_appener_rolling_file_t *self);

#ifdef  __cplusplus
}
#endif

#endif//_H_TLOG_APPENDER_ROLLING_FILE_INSTANCE_H


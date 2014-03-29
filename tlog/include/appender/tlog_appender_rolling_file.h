#ifndef _H_TLOG_APPENDER_ROLLING_FILE_H
#define _H_TLOG_APPENDER_ROLLING_FILE_H

#include "tlog_config_reader.h"
#include <stdio.h>

typedef struct tlog_rolling_file_instance_s
{
	FILE *fout;
	uint32_t index;
}tlog_rolling_file_instance_t;


void tlog_rolling_file_instance_init(tlog_rolling_file_instance_t *self, const tlog_config_appender_rolling_file_t *config);

void tlog_rolling_file_instance_log(tlog_rolling_file_instance_t *self, const tlog_config_appender_rolling_file_t *config, const tlog_message_t *message);

void tlog_rolling_file_instance_fini(tlog_rolling_file_instance_t *self);

#endif//_H_TLOG_APPENDER_ROLLING_FILE_INSTANCE_H


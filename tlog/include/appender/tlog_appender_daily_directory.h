#ifndef _H_TLOG_APPENDER_DAILY_DIRECTORY_H
#define _H_TLOG_APPENDER_DAILY_DIRECTORY_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "tlog_config_reader.h"
#include "tlog_message_types.h"

#include <stdio.h>

typedef struct tlog_appender_daily_directory_s
{
	FILE *fout;
	uint32_t index;
	char file_name[TSERVER_FILE_NAME_LENGH];
}tlog_appener_daily_directory_t;


void tlog_appender_daily_directory_init(tlog_appener_daily_directory_t *self, const tlog_config_appender_daily_directory_t *config);

void tlog_appender_daily_directory_log(tlog_appener_daily_directory_t *self, const tlog_config_appender_daily_directory_t *config, const tlog_message_t *message);

void tlog_appender_daily_directory_fini(tlog_appener_daily_directory_t *self);

#ifdef  __cplusplus
}
#endif

#endif//_H_TLOG_APPENDER_DAILY_DIRECTORY_INSTANCE_H


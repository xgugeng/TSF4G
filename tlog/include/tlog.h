#ifndef _H_TLOG
#define _H_TLOG

#include "tlibc/platform/tlibc_platform.h"
#include "tlog_config_types.h"

#include <stdio.h>
#include <stdbool.h>
#include "terrno.h"

#define TLOG_VERSION "0.0.1"


typedef struct _tlog_rolling_file_instance_t
{
	FILE *fout;
	uint32_t index;
}tlog_rolling_file_instance_t;

typedef struct _tlog_appender_instance_t
{
	tlog_appender_type_t type;
	tlog_rolling_file_instance_t rolling_file;	
}tlog_appender_instance_t;

typedef struct _tlog_instance_t
{
	uint32_t appender_instance_num;
	tlog_appender_instance_t appender_instance[TLOG_MAX_APPENDER_NUM];
}tlog_instance_t;



typedef struct _tlog_t tlog_t;
struct _tlog_t
{
	tlog_config_t config;
	tlog_instance_t instance;
};


TERROR_CODE tlog_init(tlog_t *self, const char *config_file);

void tlog_write(tlog_t *self, const char *message, size_t message_size);

void tlog_fini(tlog_t *self);

#endif//_H_TLOG

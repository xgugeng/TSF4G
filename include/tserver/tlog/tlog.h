#ifndef _H_TLOG
#define _H_TLOG

#include "tlibc/platform/tlibc_platform.h"
#include "tserver/terrno.h"

#define TLOG_VERSION "0.0.1"

typedef struct _tlog_t tlog_t;
struct _tlog_t
{
	const char *config_file;
};


TERROR_CODE tlog_init(tlog_t *self, const char *config_file);

#endif//_H_TLOG

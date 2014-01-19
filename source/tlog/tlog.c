#include "tserver/tlog/tlog.h"
#include "tserver/terrno.h"

#include <stdio.h>

TERROR_CODE tlog_init(tlog_t *self, const char *config_file)
{
	self->config_file = config_file;
	return E_TS_NOERROR;
}

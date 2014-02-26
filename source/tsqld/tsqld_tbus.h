#ifndef _H_TSQLD_TBUS_H
#define _H_TSQLD_TBUS_H

#include "tcommon/terrno.h"
#include "tsqld/tsqld_protocol_types.h"
#include <unistd.h>

TERROR_CODE tsqld_tbus_init();

void tsqld_tbus_send(const tsqld_protocol_t *head, const char* data, size_t data_size);

void tsqld_tbus_flush();

void tsqld_tbus_fini();



TERROR_CODE tsqld_tbus_proc();

#endif//_H_TSQLD_TBUS_H


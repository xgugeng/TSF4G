#ifndef _H_TBUS
#define _H_TBUS

#include "tserver/terrno.h"

#define TBUS_VERSION "0.0.1"

typedef struct _tbus_t tbus_t;
struct _tbus_t
{
	int size;
	volatile int head_offset;
	volatile int tail_offset;
	char buff[0];
};

TERROR_CODE tbus_init(tbus_t *tb, int size);


#endif//_H_TBUS

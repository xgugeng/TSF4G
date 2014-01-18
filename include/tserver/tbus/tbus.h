#ifndef _H_TBUS
#define _H_TBUS

#include "tlibc/platform/tlibc_platform.h"

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

tbus_t *tbus_open(int shm_key);

TERROR_CODE tbus_send(tbus_t *tb, const char* buf, tuint16 len);

TERROR_CODE tbus_peek(tbus_t *tb, const char** buf, tuint16 *len);

void tbus_peek_over(tbus_t *tb, tuint16 len);


#endif//_H_TBUS

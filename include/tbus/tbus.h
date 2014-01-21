#ifndef _H_TBUS
#define _H_TBUS

#include "tlibc/platform/tlibc_platform.h"

#include "terrno.h"

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


//len的返回值一定大于等于输入值
TERROR_CODE tbus_send_begin(tbus_t *tb, TLIBC_OUT char** buf, TLIBC_INOUT tuint16 *len);

void tbus_send_end(tbus_t *tb, tuint16 len);


TERROR_CODE tbus_read_begin(tbus_t *tb, TLIBC_OUT const char** buf, TLIBC_OUT tuint16 *len);

void tbus_read_end(tbus_t *tb, tuint16 len);


#endif//_H_TBUS

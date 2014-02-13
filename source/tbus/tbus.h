#ifndef _H_TBUS
#define _H_TBUS

#include "tlibc/platform/tlibc_platform.h"

#include "tcommon/terrno.h"

#define TBUS_VERSION "0.0.1"

typedef enum _tbus_cmd_e
{
    e_tbus_cmd_package = 1,
    e_tbus_cmd_ignore = 2,
}tbus_cmd_e;

typedef struct _tbus_header_s
{
    tbus_cmd_e cmd;
    size_t size;
}tbus_header_s;

typedef struct _tbus_t tbus_t;
struct _tbus_t
{
	size_t size;
	volatile int head_offset;
	volatile int tail_offset;
	char buff[0];
};


TERROR_CODE tbus_init(tbus_t *tb, size_t size);


//len的返回值一定大于等于输入值
TERROR_CODE tbus_send_begin(tbus_t *tb, TLIBC_OUT char** buf, TLIBC_INOUT size_t *len);

void tbus_send_end(tbus_t *tb, size_t len);


TERROR_CODE tbus_read_begin(tbus_t *tb, TLIBC_OUT const char** buf, TLIBC_OUT size_t *len);

void tbus_read_end(tbus_t *tb, size_t len);


#endif//_H_TBUS

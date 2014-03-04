#ifndef _H_TBUS
#define _H_TBUS

#include "tlibc/platform/tlibc_platform.h"

#include "terrno.h"

#include <stdint.h>

#define TBUS_VERSION "0.0.1"

typedef uint32_t tbus_atomic_size_t;

typedef enum _tbus_cmd_e
{
    e_tbus_cmd_package = 1,
    e_tbus_cmd_ignore = 2,
}tbus_cmd_e;

typedef struct _tbus_header_s
{
    tbus_cmd_e cmd;
    tbus_atomic_size_t size;
}tbus_header_s;



typedef struct _tbus_t tbus_t;
struct _tbus_t
{
	tbus_atomic_size_t size;
	volatile tbus_atomic_size_t head_offset;
	volatile tbus_atomic_size_t tail_offset;
	char buff[1];
};


TERROR_CODE tbus_init(tbus_t *tb, tbus_atomic_size_t size);

//判断tbus中是否具有长度为len的空间
//如果有则返回一个大于等于len的数字表示最大可写空间
//如果没有则返回0
tbus_atomic_size_t tbus_send_begin(tbus_t *tb, TLIBC_OUT char** buf, tbus_atomic_size_t len);


void tbus_send_end(tbus_t *tb, tbus_atomic_size_t len);


tbus_atomic_size_t tbus_read_begin(tbus_t *tb, TLIBC_OUT char** buf);

void tbus_read_end(tbus_t *tb, tbus_atomic_size_t len);


#endif//_H_TBUS

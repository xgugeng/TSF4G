#ifndef _H_TBUS
#define _H_TBUS

#ifdef  __cplusplus
extern "C" {
#endif

#include "platform/tlibc_platform.h"

#include "terrno.h"

#include <stdint.h>
#include <sys/uio.h>


#define TBUS_VERSION "0.0.1"

typedef uint32_t tbus_atomic_size_t;

typedef enum tbus_cmd_e
{
    e_tbus_cmd_package = 1,
    e_tbus_cmd_ignore = 2,
}tbus_cmd_t;

typedef struct tbus_header_s
{
    tbus_cmd_t cmd;
    tbus_atomic_size_t size;
}tbus_header_t;


typedef struct tbus_s
{
	volatile tbus_atomic_size_t head_offset;
	volatile tbus_atomic_size_t tail_offset;
	tbus_atomic_size_t packet_size;
	tbus_atomic_size_t size;
	char buff[1];
}tbus_t;


void tbus_init(tbus_t *tb, size_t size, size_t number);

tbus_atomic_size_t tbus_send_begin(tbus_t *tb, char** buf);
void tbus_send_end(tbus_t *tb, tbus_atomic_size_t len);


tbus_atomic_size_t tbus_read_begin(tbus_t *tb, struct iovec *iov, size_t *iov_num);
void tbus_read_end(tbus_t *tb, tbus_atomic_size_t head);

#ifdef  __cplusplus
}
#endif

#endif//_H_TBUS


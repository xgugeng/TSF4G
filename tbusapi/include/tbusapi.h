#ifndef _H_TBUSAPI_H
#define _H_TBUSAPI_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "tlibc_error_code.h"
#include "tbus.h"

#include <sys/uio.h>
#include <stdbool.h>
#include <stdint.h>

#define TBUSAPI_IOV_NUM 65535
typedef struct tbusapi_s tbusapi_t;
typedef bool (*tbusapi_on_recv_func)(tbusapi_t *self, const char *buf, size_t buf_len);
typedef uint16_t (*tbusapi_on_recviov_func)(tbusapi_t *self, struct iovec *iov, uint16_t iov_num);
typedef tbus_atomic_size_t (*tbusapi_encode_func)(char *dst, size_t dst_len, const char *src, size_t src_len);
struct tbusapi_s
{
	tbus_t *itb;
	tbus_t *otb;

	tbusapi_on_recviov_func on_recviov;
	tbusapi_on_recv_func on_recv;
	tbusapi_encode_func encode;

	struct iovec iov[TBUSAPI_IOV_NUM];
	size_t iov_num;
};

void tbusapi_init(tbusapi_t *self, tbus_t *itb, tbus_t *otb);

void tbusapi_send(tbusapi_t *self, const char *packet, size_t packet_len);

tlibc_error_code_t tbusapi_process(tbusapi_t *self);

#ifdef  __cplusplus
}
#endif

#endif//_H_TBUSAPI_H


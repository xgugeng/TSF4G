#ifndef _H_TCONNAPI_H
#define _H_TCONNAPI_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "tbusapi.h"
#include "sip.h"
#include "tlibcdef.h"
#include "tutils_encode.h"

typedef struct tconnapi_s tconnapi_t;

typedef void (*tconnapi_on_connect_func)(tconnapi_t *self, const sip_cid_t *cid);
typedef void (*tconnapi_on_close_func)(tconnapi_t *self, const sip_cid_t *cid);
typedef void (*tconnapi_on_recv_func)(tconnapi_t *self, const sip_cid_t *cid, const char *packet, sip_size_t packet_size);

struct tconnapi_s
{
	tbus_t *itb;
	tbus_t *otb;
    tbusapi_t tbusapi;

	tlibc_encode_t encode;
	tconnapi_on_connect_func on_connect;
	tconnapi_on_close_func on_close;
	tconnapi_on_recv_func on_recv;
};




tlibc_error_code_t tconnapi_init(tconnapi_t *self, key_t ikey, key_t okey, tlibc_encode_t encode);

void tconnapi_accept(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num);

void tconnapi_send(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num, const void* data);

void tconnapi_close(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num);

tlibc_error_code_t tconnapi_process(tconnapi_t *self);

void tconnapi_fini(tconnapi_t *self);

#ifdef  __cplusplus
}
#endif

#endif//_H_TCONNAPI_H


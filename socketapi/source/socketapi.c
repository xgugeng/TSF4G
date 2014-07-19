#include "socketapi.h"

#include <sys/uio.h>
#include <errno.h>
#include <string.h>

tlibc_error_code_t socketapi_init(socketapi_t *self, const char *ip, uint16_t port)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;

	return ret;
}

tlibc_error_code_t socketapi_process(socketapi_t *self)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	/*
	struct iovec iov[1];
	size_t iov_num = 1; 
	*/
	return ret;
}

void socketapi_send(socketapi_t *self, const char *packet, size_t packet_len)
{
}

void socketapi_fini(socketapi_t *self)
{
}


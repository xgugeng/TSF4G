#include "tbusapi.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/uio.h>
#include <errno.h>
#include <string.h>

static tbus_atomic_size_t encode(char *dst, size_t dst_len, const char *src, size_t src_len)
{
	if(src_len > dst_len)
	{
		return 0;
	}
	memcpy(dst, src, src_len);
	return (tbus_atomic_size_t)src_len;
}

static uint16_t tbusapi_on_recviov(tbusapi_t *self, struct iovec *iov, uint16_t iov_num)
{
	uint16_t i;
	for(i = 0; i < iov_num; ++i)
	{
		if(self->on_recv)
		{
			if(!self->on_recv(self, self->iov[i].iov_base, self->iov[i].iov_len))
			{
				goto done;
			}
		}
	}
done:
	return i;
}

void tbusapi_init(tbusapi_t *self, tbus_t *itb, tbus_t *otb)
{
	self->itb = itb;
	self->otb = otb;
	self->encode = encode;
	self->on_recv = NULL;
	self->on_recviov = tbusapi_on_recviov;
	self->iov_num = TBUSAPI_IOV_NUM;
}

tlibc_error_code_t tbusapi_process(tbusapi_t *self)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	size_t iov_num = self->iov_num; 
	tbus_atomic_size_t tbus_head = tbus_read_begin(self->itb, self->iov, &iov_num);
	if(iov_num == 0)
	{
		if(tbus_head != self->itb->head_offset)
		{
			goto read_end;
		}
		else
		{
			ret = E_TLIBC_WOULD_BLOCK;
			goto done;
		}
	}

	if(self->on_recviov)
	{
		uint16_t pos = self->on_recviov(self, self->iov, (uint16_t)iov_num);
		if(pos < iov_num)
		{
			tbus_head = tbus_packet2offset(self->itb, self->iov[pos].iov_base);
		}
		if(pos == 0)
		{
			ret = E_TLIBC_WOULD_BLOCK;
		}
	}

read_end:
	tbus_read_end(self->itb, tbus_head);	
done:
	return ret;
}

void tbusapi_send(tbusapi_t *self, const char *packet, size_t packet_len)
{
	char *buf = NULL;
	tbus_atomic_size_t buf_size;
	tbus_atomic_size_t code_size;

	buf_size = tbus_send_begin(self->otb, &buf);
	code_size = self->encode(buf, buf_size, packet, packet_len);
	if(code_size > 0)
	{
		tbus_send_end(self->otb, code_size);
	}
}

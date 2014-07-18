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

tlibc_error_code_t tbusapi_init(tbusapi_t *self, key_t input_tbuskey, key_t output_tbuskey)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;

	if(input_tbuskey == 0)
	{
		self->itb_id = 0;
		self->itb = NULL;
	}
	else
	{
		self->itb_id = shmget(input_tbuskey, 0, 0666);
		if(self->itb_id == -1)
		{
    		ret = E_TLIBC_ERRNO;
		    goto done;
		}
		self->itb = shmat(self->itb_id, NULL, 0);
		if((ssize_t)self->itb == -1)
		{
            ret = E_TLIBC_ERRNO;
            goto done;
		}
	}

	if(output_tbuskey == 0)
	{
		self->otb_id = 0;
		self->otb = NULL;
	}
	else
	{
		self->otb_id = shmget(output_tbuskey, 0, 0666);
		if(self->otb_id == -1)
		{
    		ret = E_TLIBC_ERRNO;
		    goto shmdt_itb;
		}
		
		self->otb = shmat(self->otb_id, NULL, 0);
		if((ssize_t)self->otb == -1)
		{
            ret = E_TLIBC_ERRNO;
            goto shmdt_itb;
		}
	}

	self->encode = encode;
	self->on_recv = NULL;
done:
	return ret;
shmdt_itb:
    if(self->itb)
    {
        shmdt(self->itb);
    }
	return ret;
}

tlibc_error_code_t tbusapi_process(tbusapi_t *self)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;
	struct iovec iov[1];
	size_t iov_num = 1; 
	tbus_atomic_size_t tbus_head = tbus_read_begin(self->itb, iov, &iov_num);
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

	if(self->on_recv)
	{
		self->on_recv(self, iov[0].iov_base, iov[0].iov_len);
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

void tbusapi_fini(tbusapi_t *self)
{
    if(self->itb)
    {
        shmdt(self->itb);
    }
    if(self->otb)
    {
        shmdt(self->otb);
    }
}


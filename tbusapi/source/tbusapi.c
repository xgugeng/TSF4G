#include "tbusapi.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/uio.h>

static void on_recviov(tbusapi_t *self, struct iovec *iov, uint32_t iov_num)
{
	uint32_t i;
	for(i = 0;i < iov_num;++i)
	{
		self->on_recv(self, iov[i].iov_base, iov[i].iov_len);
	}
}

TERROR_CODE tbusapi_init(tbusapi_t *self, key_t input_tbuskey, uint32_t iov_num, key_t output_tbuskey)
{
	TERROR_CODE ret = E_TS_NOERROR;

	self->itb_id = shmget(input_tbuskey, 0, 0666);
	self->itb = shmat(self->itb_id, NULL, 0);

	self->otb_id = shmget(output_tbuskey, 0, 0666);
	self->otb = shmat(self->otb_id, NULL, 0);

	self->iov_num = iov_num;
	if(iov_num > 0)
	{
		if(self->itb == NULL)
		{
			ret = E_TS_ERROR;
			goto done;
		}
	}

	self->on_recviov = on_recviov;
	self->on_recv = NULL;

done:
	return ret;
}

TERROR_CODE tbusapi_process(tbusapi_t *self)
{
	TERROR_CODE ret = E_TS_NOERROR;
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
			ret = E_TS_WOULD_BLOCK;
			goto done;
		}
	}

	if(self->on_recviov)
	{
		self->on_recviov(self, self->iov, (uint32_t)iov_num);
	}

read_end:
	tbus_read_end(self->itb, tbus_head);	
done:
	return ret;
}

